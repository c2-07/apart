#pragma once
#include "raylib.h"
#include "collision.h"
#include <vector>

struct OrbitCamera {
    Vector3 targetPos;       // Current focus position
    float yaw;               // Horizontal angle in degrees
    float pitch;             // Vertical angle in degrees [-10, 70]
    float targetDistance;    // Desired distance [4, 14]
    float currentDistance;   // Smoothed / obstacle-adjusted distance
    Vector3 currentEye;      // Camera position in world

    void Init(Vector3 playerBasePos);
    void Update(Vector3 playerBasePos, float dt, const std::vector<AABB>& obstacles,
                float mouseSensitivity = 0.18f, bool invertY = false);
    Camera3D GetRaylibCamera() const;
    float GetYawRadians() const;
};
