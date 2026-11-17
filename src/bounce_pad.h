#pragma once
#include "entity.h"

namespace obby {

/**
 * @brief Launch pad entity that propels the player high into the air.
 * Demonstrates solid entity collision, onPlayerTouch impulse response,
 * and concentric target drawing.
 */
class BouncePad : public Entity {
private:
    float m_strength;

public:
    BouncePad(const Vec3& position, const Vec3& size, float strength = 22.0f);
    virtual ~BouncePad() override = default;

    float getStrength() const { return m_strength; }
    void setStrength(float s) { m_strength = s; }

    virtual void update(float dt, Player& player) override;
    virtual void onPlayerTouch(Player& player) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "BouncePad"; }
    virtual bool isSolid() const override { return true; }
};

} // namespace obby

using BouncePad = obby::BouncePad;
