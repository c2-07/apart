#pragma once
#include "raylib.h"
#include "vec3.h"
#include "aabb.h"
#include <vector>

namespace obby {

/**
 * @brief Third-person orbiting camera controller with collision pull-in and dynamic FOV.
 * Encapsulates spherical coordinate orbit mathematics, target smoothing, lookahead offsets,
 * raycast obstacle avoidance, and dynamic speed-proportional field of view.
 */
class CameraController {
private:
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
