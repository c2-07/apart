#pragma once
#include "raylib.h"
#include "collision.h"
#include "particle.h"
#include <vector>

struct PlayerInput {
    float moveForward = 0.0f;
    float moveRight = 0.0f;
    bool jumpPressed = false;
    bool jumpHeld = false;
    bool respawnPressed = false;
};

struct Player {
    Vector3 position;
    Vector3 velocity;
    float modelYaw;
    float targetYaw;

    bool isGrounded;
    bool wasGrounded;
    float coyoteTimer;
    float jumpBufferTimer;

    int deathCount;
    bool isAlive;
    Vector3 spawnPoint;

    // Animation & Visual State
    float walkCycle;
    int colorIndex; // 0: Classic Blue, 1: Crimson Red, 2: Emerald Green, 3: Royal Purple, 4: Sunset Orange, 5: Sunshine Yellow

    void Init(Vector3 spawn);
    void Respawn();
    void UpdatePhysics(float dt, const PlayerInput& input, float cameraYaw,
                       const std::vector<AABB>& colliders, ParticleSystem* particles = nullptr);
    void Draw() const;
    void DrawShadow(const std::vector<AABB>& colliders) const;
    AABB GetAABB() const;
    Color GetShirtColor() const;
};
