#pragma once
#include "raylib.h"
#include "collision.h"
#include <vector>

struct PlayerInput {
    float moveForward = 0.0f;   // +1 forward (W), -1 backward (S)
    float moveRight = 0.0f;     // +1 right (D), -1 left (A)
    bool jumpPressed = false;
    bool jumpHeld = false;
    bool respawnPressed = false;
};

struct Player {
    Vector3 position;           // Feet center
    Vector3 velocity;
    float modelYaw;             // Current facing yaw (radians)
    float targetYaw;            // Target facing yaw (radians)

    bool isGrounded;
    float coyoteTimer;          // Seconds remaining for coyote jump
    float jumpBufferTimer;      // Seconds remaining for buffered jump

    int deathCount;
    bool isAlive;
    Vector3 spawnPoint;

    void Init(Vector3 spawn);
    void Respawn();
    void UpdatePhysics(float dt, const PlayerInput& input, float cameraYaw, const std::vector<AABB>& colliders);
    void Draw() const;
    AABB GetAABB() const;
};
