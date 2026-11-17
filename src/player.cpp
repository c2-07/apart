#include "player.h"
#include "constants.h"
#include "audio.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>

namespace obby {

Player::Player()
    : m_position(Vec3{ 0.0f, 0.0f, 0.0f }),
      m_velocity(Vec3{ 0.0f, 0.0f, 0.0f }),
      m_modelYaw(0.0f),
      m_targetYaw(0.0f),
      m_isGrounded(false),
      m_wasGrounded(false),
      m_coyoteTimer(0.0f),
      m_jumpBufferTimer(0.0f),
      m_deathCount(0),
      m_isAlive(true),
      m_spawnPoint(Vec3{ 0.0f, 0.0f, 0.0f }),
      m_colorIndex(0),
      m_animator() {}

void Player::init(const Vec3& spawn) {
    m_spawnPoint = spawn;
    m_deathCount = 0;
    m_colorIndex = 0;
    m_wasGrounded = false;
    m_animator.reset();
    respawn();
}

void Player::respawn() {
    m_position = m_spawnPoint;
    m_velocity = Vec3{ 0.0f, 0.0f, 0.0f };
    m_modelYaw = 0.0f;
    m_targetYaw = 0.0f;
    m_isGrounded = false;
    m_wasGrounded = false;
    m_coyoteTimer = 0.0f;
    m_jumpBufferTimer = 0.0f;
    m_isAlive = true;
    m_animator.reset();
}

void Player::jump() {
    m_velocity.y = Constants::JUMP_VELOCITY;
    m_jumpBufferTimer = 0.0f;
    m_coyoteTimer = 0.0f;
    m_isGrounded = false;
    AudioManager::Play(SoundId::JUMP);
}

void Player::applyImpulse(const Vec3& impulse) {
    m_velocity += impulse;
}

Color Player::getShirtColor() const {
    switch (m_colorIndex) {
        case 1: return Color{ 225, 45, 55, 255 };   // Crimson Red
        case 2: return Color{ 45, 185, 95, 255 };   // Emerald Green
        case 3: return Color{ 145, 65, 215, 255 };  // Royal Purple
        case 4: return Color{ 245, 125, 35, 255 };  // Sunset Orange
        case 5: return Color{ 245, 215, 45, 255 };  // Sunshine Yellow
        case 0:
        default: return Color{ 55, 130, 235, 255 }; // Classic Blue
    }
}

AABB Player::getBounds() const {
    return AABB{
        Vector3{ m_position.x - Constants::HALF_PLAYER_WIDTH, m_position.y, m_position.z - Constants::HALF_PLAYER_DEPTH },
        Vector3{ m_position.x + Constants::HALF_PLAYER_WIDTH, m_position.y + Constants::PLAYER_HEIGHT, m_position.z + Constants::HALF_PLAYER_DEPTH }
    };
}

static float NormalizeAngle(float angle) {
    while (angle > PI)  angle -= 2.0f * PI;
    while (angle < -PI) angle += 2.0f * PI;
    return angle;
}

void Player::updatePhysics(float dt, const PlayerInput& input, float cameraYaw,
                           const std::vector<AABB>& colliders, ParticleSystem* particles) {
    if (input.respawnPressed) {
        m_deathCount++;
        respawn();
        return;
    }

    m_wasGrounded = m_isGrounded;
    float previousFallSpeed = m_velocity.y;

    // Camera-relative input direction
    Vec3 forwardVec = { -sinf(cameraYaw), 0.0f, -cosf(cameraYaw) };
    Vec3 rightVec   = {  cosf(cameraYaw), 0.0f, -sinf(cameraYaw) };

    Vec3 wishDir = (forwardVec * input.moveForward) + (rightVec * input.moveRight);
    wishDir.y = 0.0f;

    float wishSpeed = wishDir.length();
    if (wishSpeed > 0.001f) {
        wishDir = wishDir.normalized();
        wishSpeed = std::min(wishSpeed, 1.0f);
        m_targetYaw = atan2f(wishDir.x, wishDir.z);
    } else {
        wishSpeed = 0.0f;
    }

    // Smooth model rotation
    float angleDiff = NormalizeAngle(m_targetYaw - m_modelYaw);
    float maxTurn = Constants::MODEL_TURN_SPEED * dt;
    if (std::abs(angleDiff) <= maxTurn) {
        m_modelYaw = m_targetYaw;
    } else {
        m_modelYaw += (angleDiff > 0.0f ? maxTurn : -maxTurn);
    }
    m_modelYaw = NormalizeAngle(m_modelYaw);

    // Horizontal acceleration & friction
    float currentAccel = m_isGrounded ? Constants::GROUND_ACCEL : Constants::AIR_ACCEL;
    float currentFriction = m_isGrounded ? Constants::GROUND_FRICTION : Constants::AIR_DRAG;

    Vec3 targetHorizVel = wishDir * (wishSpeed * Constants::RUN_SPEED);
    Vec3 currentHorizVel = { m_velocity.x, 0.0f, m_velocity.z };

    if (wishSpeed > 0.001f) {
        Vec3 velDiff = targetHorizVel - currentHorizVel;
        float diffLen = velDiff.length();
        if (diffLen > 0.001f) {
            float step = currentAccel * dt;
            if (step >= diffLen) {
                currentHorizVel = targetHorizVel;
            } else {
                currentHorizVel += velDiff.normalized() * step;
            }
        }
    } else {
        float speed = currentHorizVel.length();
        if (speed > 0.001f) {
            float drop = currentFriction * dt;
            float newSpeed = std::max(0.0f, speed - drop);
            currentHorizVel = currentHorizVel.normalized() * newSpeed;
        } else {
            currentHorizVel = Vec3{ 0.0f, 0.0f, 0.0f };
        }
    }

    m_velocity.x = currentHorizVel.x;
    m_velocity.z = currentHorizVel.z;

    // Jump Buffering & Coyote Time
    if (input.jumpPressed) {
        m_jumpBufferTimer = Constants::JUMP_BUFFER_TIME;
    } else if (m_jumpBufferTimer > 0.0f) {
        m_jumpBufferTimer -= dt;
    }

    if (m_isGrounded) {
        m_coyoteTimer = Constants::COYOTE_TIME;
    } else if (m_coyoteTimer > 0.0f) {
        m_coyoteTimer -= dt;
    }

    // Jump Execution
    if (m_jumpBufferTimer > 0.0f && m_coyoteTimer > 0.0f) {
        jump();
        if (particles) {
            particles->SpawnJumpPuff(m_position.toRaylib());
        }
    }

    // Gravity
    m_velocity.y += Constants::GRAVITY * dt;
    if (m_velocity.y < Constants::TERMINAL_VELOCITY) {
        m_velocity.y = Constants::TERMINAL_VELOCITY;
    }

    // Substepping
    float maxMovement = std::max({ std::abs(m_velocity.x * dt), std::abs(m_velocity.y * dt), std::abs(m_velocity.z * dt) });
    int substeps = std::max(1, static_cast<int>(std::ceil(maxMovement / 0.04f)));
    float subDt = dt / static_cast<float>(substeps);

    constexpr float SKIN = 0.01f;
    bool landedThisTick = false;

    for (int step = 0; step < substeps; ++step) {
        // 1. Resolve X
        m_position.x += m_velocity.x * subDt;
        AABB boxX = getBounds();
        for (const auto& c : colliders) {
            bool overlapY = (boxX.getMin().y < c.getMax().y - SKIN) && (boxX.getMax().y > c.getMin().y + SKIN);
            bool overlapZ = (boxX.getMin().z < c.getMax().z - SKIN) && (boxX.getMax().z > c.getMin().z + SKIN);
            if (overlapY && overlapZ) {
                if (boxX.getMin().x < c.getMax().x && boxX.getMax().x > c.getMin().x) {
                    if (m_velocity.x > 0.0f) {
                        m_position.x = c.getMin().x - Constants::HALF_PLAYER_WIDTH;
                    } else if (m_velocity.x < 0.0f) {
                        m_position.x = c.getMax().x + Constants::HALF_PLAYER_WIDTH;
                    }
                    m_velocity.x = 0.0f;
                    boxX = getBounds();
                }
            }
        }

        // 2. Resolve Z
        m_position.z += m_velocity.z * subDt;
        AABB boxZ = getBounds();
        for (const auto& c : colliders) {
            bool overlapX = (boxZ.getMin().x < c.getMax().x - SKIN) && (boxZ.getMax().x > c.getMin().x + SKIN);
            bool overlapY = (boxZ.getMin().y < c.getMax().y - SKIN) && (boxZ.getMax().y > c.getMin().y + SKIN);
            if (overlapX && overlapY) {
                if (boxZ.getMin().z < c.getMax().z && boxZ.getMax().z > c.getMin().z) {
                    if (m_velocity.z > 0.0f) {
                        m_position.z = c.getMin().z - Constants::HALF_PLAYER_DEPTH;
                    } else if (m_velocity.z < 0.0f) {
                        m_position.z = c.getMax().z + Constants::HALF_PLAYER_DEPTH;
                    }
                    m_velocity.z = 0.0f;
                    boxZ = getBounds();
                }
            }
        }

        // 3. Resolve Y
        m_position.y += m_velocity.y * subDt;
        AABB boxY = getBounds();
        for (const auto& c : colliders) {
            bool overlapX = (boxY.getMin().x < c.getMax().x - SKIN) && (boxY.getMax().x > c.getMin().x + SKIN);
            bool overlapZ = (boxY.getMin().z < c.getMax().z - SKIN) && (boxY.getMax().z > c.getMin().z + SKIN);
            if (overlapX && overlapZ) {
                if (boxY.getMin().y <= c.getMax().y && boxY.getMax().y > c.getMin().y) {
                    if (m_velocity.y <= 0.0f) {
                        m_position.y = c.getMax().y;
                        m_velocity.y = 0.0f;
                        landedThisTick = true;
                    } else if (boxY.getMin().y < c.getMin().y && boxY.getMax().y >= c.getMin().y) {
                        m_position.y = c.getMin().y - Constants::PLAYER_HEIGHT;
                        m_velocity.y = 0.0f;
                    }
                    boxY = getBounds();
                }
            }
        }
    }
    m_isGrounded = landedThisTick;

    // Update procedural animator
    float hSpeed = std::sqrt(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z);
    m_animator.update(dt, m_isGrounded, m_wasGrounded, hSpeed, previousFallSpeed,
                      wishSpeed, wishDir, m_velocity, m_position, particles);
}

void Player::drawShadow(const std::vector<AABB>& colliders) const {
    m_animator.drawShadow(m_position, colliders);
}

void Player::draw() const {
    m_animator.draw(m_position, m_modelYaw, m_velocity, m_isGrounded, getShirtColor());
}

} // namespace obby
