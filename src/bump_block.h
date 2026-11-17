#pragma once
#include "entity.h"
#include "player.h"
#include "particle.h"

namespace obby {

/**
 * @brief Interactive solid bump block.
 * When hit from below while the player is moving upward, bounces up ~0.2 m over 0.15 s,
 * releases a coin with a pop animation, and becomes an empty gray block.
 */
class BumpBlock : public Entity {
private:
    int m_coinsRemaining;
    int m_worldId;
    float m_bounceTimer;
    float m_coinPopTimer;

    static ParticleSystem* s_particles;

public:
    BumpBlock(const Vec3& position, int coins = 1, int worldId = 1, const Vec3& size = Vec3{ 1.0f, 1.0f, 1.0f });
    virtual ~BumpBlock() override = default;

    virtual void update(float dt, Player& player) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "bump"; }

    virtual bool isSolid() const override { return true; }
    virtual AABB getBounds() const override { return AABB::fromCenterSize(m_position, m_size); }

    int getCoinsRemaining() const { return m_coinsRemaining; }
    bool isEmpty() const { return m_coinsRemaining <= 0; }
    bool isBouncing() const { return m_bounceTimer > 0.0f; }
    int getWorldId() const { return m_worldId; }

    void triggerBump(Player& player);

    static void setParticleSystem(ParticleSystem* ps) {
        s_particles = ps;
    }
};

} // namespace obby

using BumpBlock = obby::BumpBlock;
