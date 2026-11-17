#include "player.h"
#include "constants.h"
#include "audio.h"
#include "raymath.h"
#include "rlgl.h"

#include <cmath>
#include <algorithm>

static Color DarkenColor(Color c, float factor) {
    return Color{
        static_cast<unsigned char>(c.r * factor),
        static_cast<unsigned char>(c.g * factor),
        static_cast<unsigned char>(c.b * factor),
        c.a
    };
}

void Player::Init(Vector3 spawn) {
    spawnPoint = spawn;
    deathCount = 0;
    colorIndex = 0;
    walkCycle = 0.0f;
    wasGrounded = false;
    Respawn();
}

void Player::Respawn() {
    position = spawnPoint;
    velocity = Vector3{ 0.0f, 0.0f, 0.0f };
    modelYaw = 0.0f;
    targetYaw = 0.0f;
    isGrounded = false;
    wasGrounded = false;
    coyoteTimer = 0.0f;
    jumpBufferTimer = 0.0f;
    isAlive = true;
}

Color Player::GetShirtColor() const {
    switch (colorIndex) {
        case 1: return Color{ 225, 45, 55, 255 };   // Crimson Red
        case 2: return Color{ 45, 185, 95, 255 };   // Emerald Green
        case 3: return Color{ 145, 65, 215, 255 };  // Royal Purple
        case 4: return Color{ 245, 125, 35, 255 };  // Sunset Orange
        case 5: return Color{ 245, 215, 45, 255 };  // Sunshine Yellow
        case 0:
        default: return Color{ 55, 130, 235, 255 }; // Classic Blue
    }
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

void Player::UpdatePhysics(float dt, const PlayerInput& input, float cameraYaw,
                           const std::vector<AABB>& colliders, ParticleSystem* particles) {
    if (input.respawnPressed) {
        deathCount++;
        Respawn();
        return;
    }

    wasGrounded = isGrounded;
    float previousFallSpeed = velocity.y;

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

    // Jump Execution
    if (jumpBufferTimer > 0.0f && coyoteTimer > 0.0f) {
        velocity.y = Constants::JUMP_VELOCITY;
        jumpBufferTimer = 0.0f;
        coyoteTimer = 0.0f;
        isGrounded = false;
        AudioManager::Play(SoundId::JUMP);
        if (particles) {
            particles->SpawnJumpPuff(position);
        }
    }

    // Gravity
    velocity.y += Constants::GRAVITY * dt;
    if (velocity.y < Constants::TERMINAL_VELOCITY) {
        velocity.y = Constants::TERMINAL_VELOCITY;
    }

    // Substepping
    float maxMovement = std::max({ std::abs(velocity.x * dt), std::abs(velocity.y * dt), std::abs(velocity.z * dt) });
    int substeps = std::max(1, static_cast<int>(std::ceil(maxMovement / 0.04f)));
    float subDt = dt / static_cast<float>(substeps);

    constexpr float SKIN = 0.01f;
    bool landedThisTick = false;

    for (int step = 0; step < substeps; ++step) {
        // 1. Resolve X
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

        // 2. Resolve Z
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

        // 3. Resolve Y
        position.y += velocity.y * subDt;
        AABB boxY = GetAABB();
        for (const auto& c : colliders) {
            bool overlapX = (boxY.min.x < c.max.x - SKIN) && (boxY.max.x > c.min.x + SKIN);
            bool overlapZ = (boxY.min.z < c.max.z - SKIN) && (boxY.max.z > c.min.z + SKIN);
            if (overlapX && overlapZ) {
                if (boxY.min.y <= c.max.y && boxY.max.y > c.min.y) {
                    if (velocity.y <= 0.0f) {
                        position.y = c.max.y;
                        velocity.y = 0.0f;
                        landedThisTick = true;
                    } else if (boxY.min.y < c.min.y && boxY.max.y >= c.min.y) {
                        position.y = c.min.y - Constants::PLAYER_HEIGHT;
                        velocity.y = 0.0f;
                    }
                    boxY = GetAABB();
                }
            }
        }
    }
    isGrounded = landedThisTick;

    // Landing event (transition from airborne to grounded with downward speed)
    if (!wasGrounded && isGrounded && previousFallSpeed < -3.5f) {
        AudioManager::Play(SoundId::LAND);
        if (particles) {
            particles->SpawnLandPuff(position, std::abs(previousFallSpeed));
        }
    }

    // Animation progress
    float hSpeed = Vector3Length(Vector3{ velocity.x, 0.0f, velocity.z });
    if (isGrounded && hSpeed > 0.5f) {
        walkCycle += hSpeed * dt * 2.2f;
    } else if (isGrounded) {
        walkCycle = 0.0f;
    }
}

void Player::DrawShadow(const std::vector<AABB>& colliders) const {
    float hitDist = 0.0f;
    Vector3 rayOrigin = { position.x, position.y + 0.1f, position.z };
    Vector3 rayDir = { 0.0f, -1.0f, 0.0f };

    if (RaycastScene(rayOrigin, rayDir, 0.01f, Constants::SHADOW_MAX_DISTANCE, colliders, &hitDist)) {
        float shadowY = rayOrigin.y - hitDist + Constants::SHADOW_OFFSET_Y;
        float progress = std::clamp(hitDist / Constants::SHADOW_MAX_DISTANCE, 0.0f, 1.0f);

        // Radius scales smoothly with elevation
        float outerRadius = Constants::SHADOW_BASE_RADIUS - progress * (Constants::SHADOW_BASE_RADIUS - Constants::SHADOW_MIN_RADIUS);
        float innerRadius = outerRadius * 0.55f;

        // Quadratic falloff for natural contact shadow look
        float fade = (1.0f - progress);
        fade = fade * fade;
        unsigned char outerAlpha = static_cast<unsigned char>(fade * (Constants::SHADOW_MAX_ALPHA * 0.5f));
        unsigned char innerAlpha = static_cast<unsigned char>(fade * Constants::SHADOW_MAX_ALPHA);

        // Outer soft fringe
        DrawCube(Vector3{ position.x, shadowY, position.z }, outerRadius, Constants::SHADOW_HEIGHT, outerRadius, Color{ 15, 20, 30, outerAlpha });
        // Inner dense core
        DrawCube(Vector3{ position.x, shadowY + 0.002f, position.z }, innerRadius, Constants::SHADOW_HEIGHT, innerRadius, Color{ 10, 15, 25, innerAlpha });
    }
}

void Player::Draw() const {
    rlPushMatrix();
    rlTranslatef(position.x, position.y, position.z);
    rlRotatef(modelYaw * RAD2DEG, 0.0f, 1.0f, 0.0f);

    float bobY = 0.0f;
    float legSwing = 0.0f;
    float armSwing = 0.0f;

    if (isGrounded) {
        float speed = Vector3Length(Vector3{ velocity.x, 0.0f, velocity.z });
        if (speed > 0.5f) {
            bobY = fabsf(sinf(walkCycle)) * 0.05f;
            legSwing = sinf(walkCycle) * 32.0f;
            armSwing = sinf(walkCycle) * 30.0f;
        }
    } else {
        // Airborne poses
        if (velocity.y > 0.0f) { // Jumping
            legSwing = -15.0f;
            armSwing = -40.0f;
        } else { // Falling
            legSwing = -5.0f;
            armSwing = 25.0f;
        }
    }

    Color shirt = GetShirtColor();
    Color shirtWire = DarkenColor(shirt, 0.65f);
    Color skin = Color{ 255, 225, 90, 255 }; // Classic blocky yellow tone
    Color skinWire = DarkenColor(skin, 0.7f);
    Color pants = Color{ 45, 60, 95, 255 };
    Color pantsWire = DarkenColor(pants, 0.65f);

    // 1. Torso
    Vector3 torsoPos = { 0.0f, 1.20f + bobY, 0.0f };
    Vector3 torsoSize = { 0.8f, 0.9f, 0.45f };
    DrawCube(torsoPos, torsoSize.x, torsoSize.y, torsoSize.z, shirt);
    DrawCubeWires(torsoPos, torsoSize.x + 0.02f, torsoSize.y + 0.02f, torsoSize.z + 0.02f, shirtWire);

    // 2. Head
    Vector3 headPos = { 0.0f, 1.95f + bobY, 0.0f };
    Vector3 headSize = { 0.55f, 0.55f, 0.55f };
    DrawCube(headPos, headSize.x, headSize.y, headSize.z, skin);
    DrawCubeWires(headPos, headSize.x + 0.02f, headSize.y + 0.02f, headSize.z + 0.02f, skinWire);

    // Face Details on +Z front
    DrawCube(Vector3{ 0.12f, 2.00f + bobY, 0.28f }, 0.07f, 0.07f, 0.02f, Color{ 30, 30, 40, 255 }); // Left Eye
    DrawCube(Vector3{ -0.12f, 2.00f + bobY, 0.28f }, 0.07f, 0.07f, 0.02f, Color{ 30, 30, 40, 255 }); // Right Eye
    DrawCube(Vector3{ 0.0f, 1.86f + bobY, 0.28f }, 0.20f, 0.04f, 0.02f, Color{ 40, 40, 50, 255 }); // Smile

    // 3. Left Leg
    rlPushMatrix();
    rlTranslatef(0.22f, 0.75f, 0.0f);
    rlRotatef(legSwing, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{ 0.0f, -0.375f, 0.0f }, 0.34f, 0.75f, 0.34f, pants);
    DrawCubeWires(Vector3{ 0.0f, -0.375f, 0.0f }, 0.35f, 0.76f, 0.35f, pantsWire);
    rlPopMatrix();

    // 4. Right Leg
    rlPushMatrix();
    rlTranslatef(-0.22f, 0.75f, 0.0f);
    rlRotatef(-legSwing, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{ 0.0f, -0.375f, 0.0f }, 0.34f, 0.75f, 0.34f, pants);
    DrawCubeWires(Vector3{ 0.0f, -0.375f, 0.0f }, 0.35f, 0.76f, 0.35f, pantsWire);
    rlPopMatrix();

    // 5. Left Arm
    rlPushMatrix();
    rlTranslatef(0.55f, 1.55f + bobY, 0.0f);
    rlRotatef(-armSwing, 1.0f, 0.0f, 0.0f);
    // Upper arm (sleeved)
    DrawCube(Vector3{ 0.0f, -0.2f, 0.0f }, 0.28f, 0.4f, 0.28f, shirt);
    DrawCubeWires(Vector3{ 0.0f, -0.2f, 0.0f }, 0.29f, 0.41f, 0.29f, shirtWire);
    // Lower arm / hand (skin)
    DrawCube(Vector3{ 0.0f, -0.55f, 0.0f }, 0.28f, 0.35f, 0.28f, skin);
    DrawCubeWires(Vector3{ 0.0f, -0.55f, 0.0f }, 0.29f, 0.36f, 0.29f, skinWire);
    rlPopMatrix();

    // 6. Right Arm
    rlPushMatrix();
    rlTranslatef(-0.55f, 1.55f + bobY, 0.0f);
    rlRotatef(armSwing, 1.0f, 0.0f, 0.0f);
    // Upper arm (sleeved)
    DrawCube(Vector3{ 0.0f, -0.2f, 0.0f }, 0.28f, 0.4f, 0.28f, shirt);
    DrawCubeWires(Vector3{ 0.0f, -0.2f, 0.0f }, 0.29f, 0.41f, 0.29f, shirtWire);
    // Lower arm / hand (skin)
    DrawCube(Vector3{ 0.0f, -0.55f, 0.0f }, 0.28f, 0.35f, 0.28f, skin);
    DrawCubeWires(Vector3{ 0.0f, -0.55f, 0.0f }, 0.29f, 0.36f, 0.29f, skinWire);
    rlPopMatrix();

    rlPopMatrix();
}
