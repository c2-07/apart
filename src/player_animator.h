#pragma once
#include "raylib.h"
#include "vec3.h"
#include "aabb.h"
#include "particle.h"
#include <vector>

namespace obby {

/**
 * @brief Handles procedural character animations, mesh composition, and shadow projection.
 * Encapsulates walk cycle timing, landing squash/stretch, lean angles, limb rotations,
 * facial details, and soft contact blob shadows.
 */
class PlayerAnimator {
private:
    float m_walkCycle;
    float m_landingTimer;
    float m_skidTimer;

    static Color darkenColor(Color c, float factor);

public:
    PlayerAnimator();

    void reset();
    void update(float dt, bool isGrounded, bool wasGrounded, float horizontalSpeed,
                float previousFallSpeed, float wishSpeed, const Vec3& wishDir,
                const Vec3& velocity, const Vec3& position, ParticleSystem* particles);

    void draw(const Vec3& position, float modelYaw, const Vec3& velocity,
              bool isGrounded, Color shirtColor) const;

    void drawShadow(const Vec3& position, const std::vector<AABB>& colliders) const;

    // Encapsulated state accessors
    float getWalkCycle() const { return m_walkCycle; }
    void setWalkCycle(float cycle) { m_walkCycle = cycle; }
    float getLandingTimer() const { return m_landingTimer; }
    void setLandingTimer(float timer) { m_landingTimer = timer; }
    float getSkidTimer() const { return m_skidTimer; }
    void setSkidTimer(float timer) { m_skidTimer = timer; }
};

} // namespace obby

using PlayerAnimator = obby::PlayerAnimator;
