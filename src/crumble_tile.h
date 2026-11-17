#pragma once
#include "entity.h"

namespace obby {

/**
 * @brief Fragile tile that crumbles and falls when stepped on, respawning after a delay.
 * Demonstrates internal state machine, time-based updates, and polymorphic solidity.
 */
class CrumbleTile : public Entity {
public:
    enum class State {
        STABLE,
        CRUMBLING,
        FALLEN
    };

private:
    Vec3 m_basePos;
    float m_delay;
    float m_respawnTime;
    State m_state;
    float m_timer;
    int m_worldId;

public:
    CrumbleTile(const Vec3& position, const Vec3& size, float delay = 0.8f,
                float respawnTime = 3.0f, int worldId = 1);
    virtual ~CrumbleTile() override = default;

    State getState() const { return m_state; }
    float getDelay() const { return m_delay; }
    float getRespawnTime() const { return m_respawnTime; }
    int getWorldId() const { return m_worldId; }

    virtual void update(float dt, Player& player) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "CrumbleTile"; }
    virtual bool isSolid() const override { return m_state == State::STABLE; }
};

} // namespace obby

using CrumbleTile = obby::CrumbleTile;
