#pragma once
#include "entity.h"
#include "player.h"

namespace obby {

/**
 * @brief Abstract base class for all enemy variants (Walker, Flyer).
 * Encapsulates shared facing direction, alive status, defeat/flattening animation state,
 * and standard stomp vs body damage arbitration.
 */
class Enemy : public Entity {
protected:
    int m_facingDirection; // +1 = moving right, -1 = moving left
    bool m_isAlive;
    bool m_isFlattened;
    float m_flattenTimer;
    Vec3 m_startPos;

public:
    Enemy(const Vec3& position, const Vec3& size, Color color = WHITE);
    virtual ~Enemy() override = default;

    // Template method pattern: calls think(), then performs stomp / hit check
    virtual void update(float dt, Player& player) override;

    // Pure virtual and virtual extension points
    virtual void think(float dt) = 0;
    virtual void onStomped();
    virtual void onHitPlayer(Player& player);

    // Overridden entity properties
    virtual bool isSolid() const override { return false; }
    virtual bool isExpired() const override;
    virtual AABB getBounds() const override {
        if (!m_isAlive) return AABB{ Vec3{ 0.0f, 0.0f, 0.0f }, Vec3{ 0.0f, 0.0f, 0.0f } };
        return Entity::getBounds();
    }

    // Getters and inspection methods
    int getFacingDirection() const { return m_facingDirection; }
    bool isAlive() const { return m_isAlive; }
    bool isFlattened() const { return m_isFlattened; }
    bool isActive() const { return !isExpired(); }
    float getFlattenTimer() const { return m_flattenTimer; }
    const Vec3& getStartPos() const { return m_startPos; }

    void setFacingDirection(int dir) { m_facingDirection = dir; }
};

} // namespace obby

using Enemy = obby::Enemy;
