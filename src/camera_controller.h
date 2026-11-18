#pragma once
#include "raylib.h"
#include "vec3.h"
#include "aabb.h"
#include <vector>

namespace obby {

/**
 * @brief Camera perspective modes.
 */
enum class CameraMode {
    ORBIT_3D,
    SIDE_SCROLL_2D
};

/**
 * @brief Third-person orbiting and 2D side-scrolling camera controller with collision pull-in and dynamic FOV.
 * Encapsulates spherical coordinate orbit mathematics, target smoothing, lookahead offsets,
 * raycast obstacle avoidance, locked side-scrolling tracking, and dynamic speed-proportional field of view.
 */
class CameraController {
private:
    CameraMode m_mode{CameraMode::ORBIT_3D};
    float m_lockedZ{0.0f};     // Locked Z depth plane for 2D side-scroller mode
    Vec3 m_targetPos;          // Smoothed focus point in world space
    float m_yaw;               // Horizontal orbit angle in degrees
    float m_pitch;             // Vertical orbit angle in degrees [-10, 70]
    float m_targetDistance;    // Desired camera distance [4, 14]
    float m_currentDistance;   // Distance adjusted for collision occlusion
    Vec3 m_currentEye;         // World space eye position
    float m_currentFov;        // Dynamic field of view

public:
    CameraController();

    void init(const Vec3& playerBasePos);
    void update(const Vec3& playerBasePos, float dt, const std::vector<AABB>& obstacles,
                float mouseSensitivity = 0.18f, bool invertY = false,
                const Vec3& playerVelocity = Vec3());

    Camera3D getRaylibCamera() const;
    float getYawRadians() const;

    // Camera Mode Controls
    void setMode(CameraMode mode, float lockedZ = 0.0f);
    CameraMode getMode() const { return m_mode; }
    bool is2D() const { return m_mode == CameraMode::SIDE_SCROLL_2D; }
    void toggleMode() {
        setMode(is2D() ? CameraMode::ORBIT_3D : CameraMode::SIDE_SCROLL_2D, m_lockedZ);
    }

    // Encapsulated Getters
    const Vec3& getTargetPos() const { return m_targetPos; }
    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }
    float getTargetDistance() const { return m_targetDistance; }
    float getCurrentDistance() const { return m_currentDistance; }
    const Vec3& getCurrentEye() const { return m_currentEye; }
    float getCurrentFov() const { return m_currentFov; }

    // Encapsulated Mutators
    void setYaw(float yaw) { m_yaw = yaw; }
    void setPitch(float pitch) { m_pitch = pitch; }
    void setTargetDistance(float dist) { m_targetDistance = dist; }
    void setCurrentDistance(float dist) { m_currentDistance = dist; }

    // Legacy method aliases
    void Init(const Vec3& playerBasePos) { init(playerBasePos); }
    void Update(const Vec3& playerBasePos, float dt, const std::vector<AABB>& obstacles,
                float mouseSensitivity = 0.18f, bool invertY = false,
                const Vec3& playerVelocity = Vec3()) {
        update(playerBasePos, dt, obstacles, mouseSensitivity, invertY, playerVelocity);
    }
    Camera3D GetRaylibCamera() const { return getRaylibCamera(); }
    float GetYawRadians() const { return getYawRadians(); }
};

} // namespace obby

using CameraController = obby::CameraController;
using OrbitCamera = obby::CameraController;
