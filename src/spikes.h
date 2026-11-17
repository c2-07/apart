#pragma once
#include "entity.h"

namespace obby {

/**
 * @brief Hazard entity that resets the player to checkpoint on touch.
 * Demonstrates non-solid hazard collision, onPlayerTouch polymorphism,
 * and procedural hazard mesh drawing.
 */
class Spikes : public Entity {
public:
    Spikes(const Vec3& position, const Vec3& size);
    virtual ~Spikes() override = default;

    virtual void update(float dt, Player& player) override;
    virtual void onPlayerTouch(Player& player) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "Spikes"; }
    virtual bool isSolid() const override { return false; }
};

} // namespace obby

using Spikes = obby::Spikes;
