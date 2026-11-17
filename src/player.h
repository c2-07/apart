#pragma once
#include "raylib.h"
#include "vec3.h"
#include "aabb.h"
#include "particle.h"
#include "player_animator.h"
#include <vector>

namespace obby {

struct PlayerInput {
    float moveForward = 0.0f;
    float moveRight = 0.0f;
    bool jumpPressed = false;
    bool jumpHeld = false;
    bool respawnPressed = false;
};

/**
 * @brief Represents the player avatar, physics state, collision response, and lifecycle.
 * Demonstrates:
 * - Full encapsulation of physics state (position, velocity, timers)
 * - Delegation of animation and rendering to PlayerAnimator
 * - Substepped AABB collision response along separate axes (X, Z, Y)
 * - Coyote time, jump buffering, and responsive air-control mechanics
 */
class Player {
private:
    Vec3 m_position;
    Vec3 m_velocity;
    float m_modelYaw;
    float m_targetYaw;

    bool m_isGrounded;
    bool m_wasGrounded;
    float m_coyoteTimer;
    float m_jumpBufferTimer;

    int m_deathCount;
    bool m_isAlive;
    Vec3 m_spawnPoint;
    int m_colorIndex; // 0: Classic Blue, 1: Crimson Red, 2: Emerald Green, 3: Royal Purple, 4: Sunset Orange, 5: Sunshine Yellow

    // Health & Invincibility
    int m_maxHearts;
    int m_hearts;
    float m_invincibleTimer;

    PlayerAnimator m_animator;

public:
    Player();

    void init(const Vec3& spawn);
    void respawn();
    void jump();
    void applyImpulse(const Vec3& impulse);

    // Health & Damage interface
    void takeDamage(const Vector2& knockbackDirection);
    void heal(int amount);
    bool isInvincible() const { return m_invincibleTimer > 0.0f; }
    int getHearts() const { return m_hearts; }
    int getMaxHearts() const { return m_maxHearts; }
    void resetHearts();
    float getInvincibleTimer() const { return m_invincibleTimer; }

    void updatePhysics(float dt, const PlayerInput& input, float cameraYaw,
                       const std::vector<AABB>& colliders, ParticleSystem* particles = nullptr);
    void draw() const;
    void drawShadow(const std::vector<AABB>& colliders) const;

    // Encapsulated Getters
    const Vec3& getPosition() const { return m_position; }
    const Vec3& getVelocity() const { return m_velocity; }
    float getModelYaw() const { return m_modelYaw; }
    float getTargetYaw() const { return m_targetYaw; }
    bool isGrounded() const { return m_isGrounded; }
    bool wasGrounded() const { return m_wasGrounded; }
    float getCoyoteTimer() const { return m_coyoteTimer; }
    float getJumpBufferTimer() const { return m_jumpBufferTimer; }
    int getDeathCount() const { return m_deathCount; }
    bool isAlive() const { return m_isAlive; }
    const Vec3& getSpawnPoint() const { return m_spawnPoint; }
    int getColorIndex() const { return m_colorIndex; }
    AABB getBounds() const;
    Color getShirtColor() const;
    const PlayerAnimator& getAnimator() const { return m_animator; }

    // Encapsulated Mutators
    void setPosition(const Vec3& pos) { m_position = pos; }
    void setVelocity(const Vec3& vel) { m_velocity = vel; }
    void setModelYaw(float yaw) { m_modelYaw = yaw; }
    void setGrounded(bool grounded) { m_isGrounded = grounded; }
    void setDeathCount(int count) { m_deathCount = count; }
    void setSpawnPoint(const Vec3& spawn) { m_spawnPoint = spawn; }
    void setColorIndex(int idx) { m_colorIndex = idx; }
    void setAlive(bool alive) { m_isAlive = alive; }

    // Compatibility method aliases
    void Init(const Vec3& spawn) { init(spawn); }
    void Respawn() { respawn(); }
    void UpdatePhysics(float dt, const PlayerInput& input, float cameraYaw,
                       const std::vector<AABB>& colliders, ParticleSystem* particles = nullptr) {
        updatePhysics(dt, input, cameraYaw, colliders, particles);
    }
    void Draw() const { draw(); }
    void DrawShadow(const std::vector<AABB>& colliders) const { drawShadow(colliders); }
    AABB GetAABB() const { return getBounds(); }
    Color GetShirtColor() const { return getShirtColor(); }
};

} // namespace obby

using Player = obby::Player;
using PlayerInput = obby::PlayerInput;
