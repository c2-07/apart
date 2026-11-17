#include "player.h"
#include "constants.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>

void Player::Init(Vector3 spawn) {
    spawnPoint = spawn;
    deathCount = 0;
    Respawn();
}

void Player::Respawn() {
    position = spawnPoint;
    velocity = Vector3{ 0.0f, 0.0f, 0.0f };
    modelYaw = 0.0f;
    targetYaw = 0.0f;
    isGrounded = false;
    coyoteTimer = 0.0f;
    jumpBufferTimer = 0.0f;
    isAlive = true;
}

AABB Player::GetAABB() const {
    return AABB{
        Vector3{ position.x - Constants::HALF_PLAYER_WIDTH, position.y, position.z - Constants::HALF_PLAYER_DEPTH },
        Vector3{ position.x + Constants::HALF_PLAYER_WIDTH, position.y + Constants::PLAYER_HEIGHT, position.z + Constants::HALF_PLAYER_DEPTH }
    };
}

static float NormalizeAngle(float angle) {
    while (angle > PI)  angle -= 2.0f * PI;
    while (angle < -PI) angle += 2.0f * PI;
    return angle;
}

void Player::UpdatePhysics(float dt, const PlayerInput& input, float cameraYaw, const std::vector<AABB>& colliders) {
    if (input.respawnPressed) {
        deathCount++;
        Respawn();
        return;
    }

    // Camera-relative input direction
    Vector3 forwardVec = { -sinf(cameraYaw), 0.0f, -cosf(cameraYaw) };
    Vector3 rightVec   = {  cosf(cameraYaw), 0.0f, -sinf(cameraYaw) };

    Vector3 wishDir = Vector3Add(
        Vector3Scale(forwardVec, input.moveForward),
        Vector3Scale(rightVec, input.moveRight)
    );
    wishDir.y = 0.0f;

    float wishSpeed = Vector3Length(wishDir);
    if (wishSpeed > 0.001f) {
        wishDir = Vector3Normalize(wishDir);
        wishSpeed = std::min(wishSpeed, 1.0f);
        targetYaw = atan2f(wishDir.x, wishDir.z);
    } else {
        wishSpeed = 0.0f;
    }

    // Smooth model rotation
    float angleDiff = NormalizeAngle(targetYaw - modelYaw);
    float maxTurn = Constants::MODEL_TURN_SPEED * dt;
    if (std::abs(angleDiff) <= maxTurn) {
        modelYaw = targetYaw;
    } else {
        modelYaw += (angleDiff > 0.0f ? maxTurn : -maxTurn);
    }
    modelYaw = NormalizeAngle(modelYaw);

    // Horizontal acceleration & friction
    float currentAccel = isGrounded ? Constants::GROUND_ACCEL : Constants::AIR_ACCEL;
    float currentFriction = isGrounded ? Constants::GROUND_FRICTION : Constants::AIR_DRAG;

    Vector3 targetHorizVel = Vector3Scale(wishDir, wishSpeed * Constants::RUN_SPEED);
    Vector3 currentHorizVel = { velocity.x, 0.0f, velocity.z };

    if (wishSpeed > 0.001f) {
        Vector3 velDiff = Vector3Subtract(targetHorizVel, currentHorizVel);
        float diffLen = Vector3Length(velDiff);
        if (diffLen > 0.001f) {
            float step = currentAccel * dt;
            if (step >= diffLen) {
                currentHorizVel = targetHorizVel;
            } else {
                currentHorizVel = Vector3Add(currentHorizVel, Vector3Scale(Vector3Normalize(velDiff), step));
            }
        }
    } else {
        float speed = Vector3Length(currentHorizVel);
        if (speed > 0.001f) {
            float drop = currentFriction * dt;
            float newSpeed = std::max(0.0f, speed - drop);
            currentHorizVel = Vector3Scale(Vector3Normalize(currentHorizVel), newSpeed);
        } else {
            currentHorizVel = Vector3{ 0.0f, 0.0f, 0.0f };
        }
    }

    velocity.x = currentHorizVel.x;
    velocity.z = currentHorizVel.z;

    // Jump Buffering & Coyote Time
    if (input.jumpPressed) {
        jumpBufferTimer = Constants::JUMP_BUFFER_TIME;
    } else if (jumpBufferTimer > 0.0f) {
        jumpBufferTimer -= dt;
    }

    if (isGrounded) {
        coyoteTimer = Constants::COYOTE_TIME;
    } else if (coyoteTimer > 0.0f) {
        coyoteTimer -= dt;
    }

    if (jumpBufferTimer > 0.0f && coyoteTimer > 0.0f) {
        velocity.y = Constants::JUMP_VELOCITY;
        jumpBufferTimer = 0.0f;
        coyoteTimer = 0.0f;
        isGrounded = false;
    }

    // Apply gravity
    velocity.y += Constants::GRAVITY * dt;
    if (velocity.y < Constants::TERMINAL_VELOCITY) {
        velocity.y = Constants::TERMINAL_VELOCITY;
    }

    // Substepping for collision to prevent tunneling at high velocity
    // Max movement per substep: 0.15 meters
    float maxMovement = std::max({ std::abs(velocity.x * dt), std::abs(velocity.y * dt), std::abs(velocity.z * dt) });
    int substeps = std::max(1, static_cast<int>(std::ceil(maxMovement / 0.15f)));
    float subDt = dt / static_cast<float>(substeps);

    constexpr float SKIN = 0.01f; // 1 cm skin margin to prevent edge snagging

    for (int step = 0; step < substeps; ++step) {
        // --- 1. Resolve X ---
        position.x += velocity.x * subDt;
        AABB boxX = GetAABB();
        for (const auto& c : colliders) {
            bool overlapY = (boxX.min.y < c.max.y - SKIN) && (boxX.max.y > c.min.y + SKIN);
            bool overlapZ = (boxX.min.z < c.max.z - SKIN) && (boxX.max.z > c.min.z + SKIN);
            if (overlapY && overlapZ) {
                if (boxX.min.x < c.max.x && boxX.max.x > c.min.x) {
                    if (velocity.x > 0.0f) {
                        position.x = c.min.x - Constants::HALF_PLAYER_WIDTH;
                    } else if (velocity.x < 0.0f) {
                        position.x = c.max.x + Constants::HALF_PLAYER_WIDTH;
                    }
                    velocity.x = 0.0f;
                    boxX = GetAABB();
                }
            }
        }

        // --- 2. Resolve Z ---
        position.z += velocity.z * subDt;
        AABB boxZ = GetAABB();
        for (const auto& c : colliders) {
            bool overlapX = (boxZ.min.x < c.max.x - SKIN) && (boxZ.max.x > c.min.x + SKIN);
            bool overlapY = (boxZ.min.y < c.max.y - SKIN) && (boxZ.max.y > c.min.y + SKIN);
            if (overlapX && overlapY) {
                if (boxZ.min.z < c.max.z && boxZ.max.z > c.min.z) {
                    if (velocity.z > 0.0f) {
                        position.z = c.min.z - Constants::HALF_PLAYER_DEPTH;
                    } else if (velocity.z < 0.0f) {
                        position.z = c.max.z + Constants::HALF_PLAYER_DEPTH;
                    }
                    velocity.z = 0.0f;
                    boxZ = GetAABB();
                }
            }
        }

        // --- 3. Resolve Y ---
        position.y += velocity.y * subDt;
        bool landedThisSubstep = false;
        AABB boxY = GetAABB();
        for (const auto& c : colliders) {
            bool overlapX = (boxY.min.x < c.max.x - SKIN) && (boxY.max.x > c.min.x + SKIN);
            bool overlapZ = (boxY.min.z < c.max.z - SKIN) && (boxY.max.z > c.min.z + SKIN);
            if (overlapX && overlapZ) {
                if (boxY.min.y < c.max.y && boxY.max.y > c.min.y) {
                    if (velocity.y <= 0.0f) {
                        position.y = c.max.y;
                        velocity.y = 0.0f;
                        landedThisSubstep = true;
                    } else {
                        position.y = c.min.y - Constants::PLAYER_HEIGHT;
                        velocity.y = 0.0f;
                    }
                    boxY = GetAABB();
                }
            }
        }
        isGrounded = landedThisSubstep;
    }
}

void Player::Draw() const {
    Vector3 center = { position.x, position.y + Constants::PLAYER_HEIGHT * 0.5f, position.z };
    Vector3 size = { Constants::PLAYER_WIDTH, Constants::PLAYER_HEIGHT, Constants::PLAYER_DEPTH };

    // Roblox-like bright saturated color + darker wireframe outline
    DrawCube(center, size.x, size.y, size.z, Color{ 66, 135, 245, 255 });
    DrawCubeWires(center, size.x + 0.01f, size.y + 0.01f, size.z + 0.01f, Color{ 30, 80, 180, 255 });

    // Direction indicator
    float eyeX = center.x + sinf(modelYaw) * 0.51f;
    float eyeZ = center.z + cosf(modelYaw) * 0.51f;
    Vector3 eyeCenter = { eyeX, center.y + 0.5f, eyeZ };
    DrawCube(eyeCenter, 0.3f, 0.3f, 0.1f, Color{ 255, 220, 50, 255 });
}
