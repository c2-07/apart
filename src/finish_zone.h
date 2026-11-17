#pragma once
#include "entity.h"

namespace obby {

/**
 * @brief Course finish trigger zone crowned by a golden victory arch.
 * Demonstrates non-solid win trigger, particle burst triggering,
 * and animated trophy rotation.
 */
class FinishZone : public Entity {
private:
    bool m_completed;
    float m_animTimer;

public:
    FinishZone(const Vec3& position, const Vec3& size);
    virtual ~FinishZone() override = default;

    bool isCompleted() const { return m_completed; }
    void setCompleted(bool comp) { m_completed = comp; }

    virtual void update(float dt, Player& player) override;
    virtual void onPlayerTouch(Player& player) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "FinishZone"; }
    virtual bool isSolid() const override { return false; }
};

} // namespace obby

using FinishZone = obby::FinishZone;
