#include "camera.h"
#include "constants.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>

void OrbitCamera::Init(Vector3 playerBasePos) {
    yaw = Constants::CAM_DEFAULT_YAW;
    pitch = Constants::CAM_DEFAULT_PITCH;
    targetDistance = Constants::CAM_DEFAULT_DISTANCE;
    currentDistance = targetDistance;
    targetPos = Vector3{ playerBasePos.x, playerBasePos.y + Constants::CAM_TARGET_HEIGHT, playerBasePos.z };

    float yawRad = yaw * DEG2RAD;
    float pitchRad = pitch * DEG2RAD;
    Vector3 offset = {
        sinf(yawRad) * cosf(pitchRad) * currentDistance,
        sinf(pitchRad) * currentDistance,
        cosf(yawRad) * cosf(pitchRad) * currentDistance
    };
    currentEye = Vector3Add(targetPos, offset);
}

void OrbitCamera::Update(Vector3 playerBasePos, float dt, const std::vector<AABB>& obstacles,
                         float mouseSensitivity, bool invertY) {
    // Mouse input for rotation (when right button is held or always if cursor is captured)
    // To allow natural obby control: right mouse drag or free mouse when locked
    Vector2 mouseDelta = GetMouseDelta();
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) || IsCursorHidden()) {
        yaw += mouseDelta.x * mouseSensitivity;
        float pitchDir = invertY ? 1.0f : -1.0f;
        pitch += mouseDelta.y * mouseSensitivity * pitchDir;
    }

    // Keyboard controls for camera rotation
    if (IsKeyDown(KEY_LEFT))  yaw -= Constants::CAM_KEY_SPEED * dt;
    if (IsKeyDown(KEY_RIGHT)) yaw += Constants::CAM_KEY_SPEED * dt;
    if (IsKeyDown(KEY_UP))    pitch += Constants::CAM_KEY_SPEED * dt;
    if (IsKeyDown(KEY_DOWN))  pitch -= Constants::CAM_KEY_SPEED * dt;

    // Clamp pitch
    if (pitch < Constants::CAM_MIN_PITCH) pitch = Constants::CAM_MIN_PITCH;
    if (pitch > Constants::CAM_MAX_PITCH) pitch = Constants::CAM_MAX_PITCH;

    // Mouse scroll zoom
    float wheel = GetMouseWheelMove();
    if (std::abs(wheel) > 0.01f) {
        targetDistance -= wheel * Constants::CAM_ZOOM_SPEED;
        targetDistance = std::clamp(targetDistance, Constants::CAM_MIN_DISTANCE, Constants::CAM_MAX_DISTANCE);
    }

    // Smooth target follow
    Vector3 desiredTarget = { playerBasePos.x, playerBasePos.y + Constants::CAM_TARGET_HEIGHT, playerBasePos.z };
    float targetLerp = std::min(1.0f, dt * Constants::CAM_SMOOTH_SPEED);
    targetPos = Vector3Lerp(targetPos, desiredTarget, targetLerp);

    // Compute ray direction from target to camera eye
    float yawRad = yaw * DEG2RAD;
    float pitchRad = pitch * DEG2RAD;
    Vector3 camDir = {
        sinf(yawRad) * cosf(pitchRad),
        sinf(pitchRad),
        cosf(yawRad) * cosf(pitchRad)
    };
    camDir = Vector3Normalize(camDir);

    // Raycast against obstacles to pull in camera
    float desiredDist = targetDistance;
    float hitDist = 0.0f;
    if (RaycastScene(targetPos, camDir, 0.5f, targetDistance, obstacles, &hitDist)) {
        float pullInDist = std::max(1.0f, hitDist - Constants::CAM_OBSTACLE_MARGIN);
        desiredDist = std::min(desiredDist, pullInDist);
    }

    // Smooth distance adjustment
    float distLerp = std::min(1.0f, dt * Constants::CAM_SMOOTH_SPEED * 1.5f);
    currentDistance = currentDistance + (desiredDist - currentDistance) * distLerp;

    currentEye = Vector3Add(targetPos, Vector3Scale(camDir, currentDistance));
}

Camera3D OrbitCamera::GetRaylibCamera() const {
    Camera3D cam;
    cam.position = currentEye;
    cam.target = targetPos;
    cam.up = Vector3{ 0.0f, 1.0f, 0.0f };
    cam.fovy = 65.0f;
    cam.projection = CAMERA_PERSPECTIVE;
    return cam;
}

float OrbitCamera::GetYawRadians() const {
    return yaw * DEG2RAD;
}
