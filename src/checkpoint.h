#pragma once
#include "entity.h"

namespace obby {

/**
 * @brief Respawn checkpoint with 3-segment waving flag cloth and pulsing ring.
 * Demonstrates onPlayerTouch state transition, sinusoidal vertex/cube animation,
 * and level spawn relocation.
 */
class Checkpoint : public Entity {
private:
    bool m_activated;
    float m_waveTimer;

public:
    Checkpoint(const Vec3& position);
    virtual ~Checkpoint() override = default;

    bool isActivated() const { return m_activated; }
    void setActivated(bool act) { m_activated = act; }

    virtual void update(float dt, Player& player) override;
    virtual void onPlayerTouch(Player& player) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "Checkpoint"; }
    virtual bool isSolid() const override { return true; }
};

} // namespace obby

using Checkpoint = obby::Checkpoint;
