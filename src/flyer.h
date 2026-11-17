#pragma once
#include "enemy.h"

namespace obby {

/**
 * @brief Airborne enemy that moves horizontally in a sinusoidal vertical wave pattern.
 * Unaffected by gravity and solids; turns around upon hitting range limits and can be stomped.
 */
class Flyer : public Enemy {
private:
    float m_amplitude;
    float m_waveSpeed;
    float m_flySpeed;
    float m_range;
    float m_elapsedTime;

public:
    Flyer(const Vec3& position, float amplitude = 1.0f, float waveSpeed = 2.0f,
          float flySpeed = 2.5f, float range = 5.0f);
    virtual ~Flyer() override = default;

    virtual void think(float dt) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "flyer"; }

    float getAmplitude() const { return m_amplitude; }
    float getWaveSpeed() const { return m_waveSpeed; }
    float getFlySpeed() const { return m_flySpeed; }
    float getRange() const { return m_range; }
};

} // namespace obby

using Flyer = obby::Flyer;
