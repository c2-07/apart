#include "camera_controller.h"
#include "constants.h"
#include "collision.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>

namespace obby {

CameraController::CameraController()
    : m_targetPos(Vec3{ 0.0f, 0.0f, 0.0f }),
      m_yaw(Constants::CAM_DEFAULT_YAW),
      m_pitch(Constants::CAM_DEFAULT_PITCH),
      m_targetDistance(Constants::CAM_DEFAULT_DISTANCE),
      m_currentDistance(Constants::CAM_DEFAULT_DISTANCE),
      m_currentEye(Vec3{ 0.0f, 0.0f, 0.0f }),
      m_currentFov(Constants::CAM_FOV_BASE) {}

void CameraController::init(const Vec3& playerBasePos) {
    if (m_mode == CameraMode::SIDE_SCROLL_2D) {
        m_yaw = 0.0f;
        m_pitch = 0.0f;
        m_targetDistance = 14.0f;
        m_currentDistance = 14.0f;
        m_currentFov = Constants::CAM_FOV_BASE;
        m_targetPos = Vec3{ playerBasePos.x, playerBasePos.y + Constants::CAM_TARGET_HEIGHT, m_lockedZ };
        m_currentEye = Vec3{ m_targetPos.x, m_targetPos.y, m_lockedZ + m_currentDistance };
        return;
    }

    m_yaw = Constants::CAM_DEFAULT_YAW;
    m_pitch = Constants::CAM_DEFAULT_PITCH;
    m_targetDistance = Constants::CAM_DEFAULT_DISTANCE;
    m_currentDistance = m_targetDistance;
    m_currentFov = Constants::CAM_FOV_BASE;
    m_targetPos = Vec3{ playerBasePos.x, playerBasePos.y + Constants::CAM_TARGET_HEIGHT, playerBasePos.z };

    float yawRad = m_yaw * DEG2RAD;
    float pitchRad = m_pitch * DEG2RAD;
    Vec3 offset = {
        sinf(yawRad) * cosf(pitchRad) * m_currentDistance,
        sinf(pitchRad) * m_currentDistance,
        cosf(yawRad) * cosf(pitchRad) * m_currentDistance
    };
    m_currentEye = m_targetPos + offset;
}

void CameraController::setMode(CameraMode mode, float lockedZ) {
    m_mode = mode;
    m_lockedZ = lockedZ;
    if (m_mode == CameraMode::SIDE_SCROLL_2D) {
        m_yaw = 0.0f;
        m_pitch = 0.0f;
        m_targetDistance = 14.0f;
        m_currentDistance = 14.0f;
        m_targetPos.z = m_lockedZ;
        m_currentEye = Vec3{ m_targetPos.x, m_targetPos.y, m_lockedZ + m_currentDistance };
    } else {
        m_yaw = Constants::CAM_DEFAULT_YAW;
        m_pitch = Constants::CAM_DEFAULT_PITCH;
        m_targetDistance = Constants::CAM_DEFAULT_DISTANCE;
    }
}

void CameraController::update(const Vec3& playerBasePos, float dt, const std::vector<AABB>& obstacles,
                             float mouseSensitivity, bool invertY, const Vec3& playerVelocity) {
    if (m_mode == CameraMode::SIDE_SCROLL_2D) {
        // In 2D mode, camera is locked straight-on to the X-Y plane
        m_yaw = 0.0f;
        m_pitch = 0.0f;

        // Smooth horizontal lookahead in movement direction
        float lookaheadX = std::clamp(playerVelocity.x * 0.25f, -2.5f, 2.5f);
        Vec3 desiredTarget = {
            playerBasePos.x + lookaheadX,
            playerBasePos.y + Constants::CAM_TARGET_HEIGHT,
            m_lockedZ
        };

        float targetLerp = std::min(1.0f, dt * Constants::CAM_SMOOTH_SPEED);
        m_targetPos = Vec3(Vector3Lerp(m_targetPos.toRaylib(), desiredTarget.toRaylib(), targetLerp));

        // Optional mouse scroll zoom
        float wheel = GetMouseWheelMove();
        if (std::abs(wheel) > 0.01f) {
            m_targetDistance -= wheel * Constants::CAM_ZOOM_SPEED;
            m_targetDistance = std::clamp(m_targetDistance, 8.0f, 22.0f);
        }
        float distLerp = std::min(1.0f, dt * Constants::CAM_SMOOTH_SPEED * 1.5f);
        m_currentDistance = m_currentDistance + (m_targetDistance - m_currentDistance) * distLerp;

        // Speed-based dynamic FOV
        float hSpeed = std::abs(playerVelocity.x);
        float speedRatio = std::clamp(hSpeed / Constants::RUN_SPEED, 0.0f, 1.0f);
        float desiredFov = Constants::CAM_FOV_BASE + speedRatio * (Constants::CAM_FOV_MAX - Constants::CAM_FOV_BASE);
        float fovLerp = std::min(1.0f, dt * 5.0f);
        m_currentFov = m_currentFov + (desiredFov - m_currentFov) * fovLerp;

        m_currentEye = Vec3{ m_targetPos.x, m_targetPos.y, m_lockedZ + m_currentDistance };
        return;
    }
    // Mouse input for rotation (when right button is held or always if cursor is captured)
    Vector2 mouseDelta = GetMouseDelta();
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsCursorHidden()) {
        m_yaw += mouseDelta.x * mouseSensitivity;
        float pitchDir = invertY ? 1.0f : -1.0f;
        m_pitch += mouseDelta.y * mouseSensitivity * pitchDir;
    }

    // Keyboard controls for camera rotation
    if (IsKeyDown(KEY_LEFT))  m_yaw -= Constants::CAM_KEY_SPEED * dt;
    if (IsKeyDown(KEY_RIGHT)) m_yaw += Constants::CAM_KEY_SPEED * dt;
    if (IsKeyDown(KEY_UP))    m_pitch += Constants::CAM_KEY_SPEED * dt;
    if (IsKeyDown(KEY_DOWN))  m_pitch -= Constants::CAM_KEY_SPEED * dt;

    // Clamp pitch
    if (m_pitch < Constants::CAM_MIN_PITCH) m_pitch = Constants::CAM_MIN_PITCH;
    if (m_pitch > Constants::CAM_MAX_PITCH) m_pitch = Constants::CAM_MAX_PITCH;

    // Mouse scroll zoom
    float wheel = GetMouseWheelMove();
    if (std::abs(wheel) > 0.01f) {
        m_targetDistance -= wheel * Constants::CAM_ZOOM_SPEED;
        m_targetDistance = std::clamp(m_targetDistance, Constants::CAM_MIN_DISTANCE, Constants::CAM_MAX_DISTANCE);
    }

    // Lookahead calculation in movement direction
    float hSpeed = std::sqrt(playerVelocity.x * playerVelocity.x + playerVelocity.z * playerVelocity.z);
    Vec3 lookahead = { 0.0f, 0.0f, 0.0f };
    if (hSpeed > 0.1f) {
        float speedRatio = std::clamp(hSpeed / Constants::RUN_SPEED, 0.0f, 1.0f);
        Vec3 horizVel = { playerVelocity.x, 0.0f, playerVelocity.z };
        lookahead = horizVel.normalized() * (speedRatio * Constants::CAM_LOOKAHEAD_DIST);
    }

    // Dynamic FOV widening with speed
    float speedRatio = std::clamp(hSpeed / Constants::RUN_SPEED, 0.0f, 1.0f);
    float desiredFov = Constants::CAM_FOV_BASE + speedRatio * (Constants::CAM_FOV_MAX - Constants::CAM_FOV_BASE);
    float fovLerp = std::min(1.0f, dt * 5.0f);
    m_currentFov = m_currentFov + (desiredFov - m_currentFov) * fovLerp;

    // Smooth target follow with lookahead
    Vec3 desiredTarget = {
        playerBasePos.x + lookahead.x,
        playerBasePos.y + Constants::CAM_TARGET_HEIGHT,
        playerBasePos.z + lookahead.z
    };
    float targetLerp = std::min(1.0f, dt * Constants::CAM_SMOOTH_SPEED);
    m_targetPos = Vec3(Vector3Lerp(m_targetPos.toRaylib(), desiredTarget.toRaylib(), targetLerp));

    // Compute ray direction from target to camera eye
    float yawRad = m_yaw * DEG2RAD;
    float pitchRad = m_pitch * DEG2RAD;
    Vec3 camDir = {
        sinf(yawRad) * cosf(pitchRad),
        sinf(pitchRad),
        cosf(yawRad) * cosf(pitchRad)
    };
    camDir = camDir.normalized();

    // Raycast against obstacles to pull in camera
    float desiredDist = m_targetDistance;
    float hitDist = 0.0f;
    if (RaycastScene(m_targetPos.toRaylib(), camDir.toRaylib(), 0.5f, m_targetDistance, obstacles, &hitDist)) {
        float pullInDist = std::max(1.0f, hitDist - Constants::CAM_OBSTACLE_MARGIN);
        desiredDist = std::min(desiredDist, pullInDist);
    }

    // Smooth distance adjustment
    float distLerp = std::min(1.0f, dt * Constants::CAM_SMOOTH_SPEED * 1.5f);
    m_currentDistance = m_currentDistance + (desiredDist - m_currentDistance) * distLerp;

    // Compute final eye position
    m_currentEye = m_targetPos + (camDir * m_currentDistance);
}

Camera3D CameraController::getRaylibCamera() const {
    Camera3D cam;
    cam.position = m_currentEye.toRaylib();
    cam.target = m_targetPos.toRaylib();
    cam.up = Vector3{ 0.0f, 1.0f, 0.0f };
    cam.fovy = m_currentFov;
    cam.projection = CAMERA_PERSPECTIVE;
    return cam;
}

float CameraController::getYawRadians() const {
    return m_yaw * DEG2RAD;
}

} // namespace obby
