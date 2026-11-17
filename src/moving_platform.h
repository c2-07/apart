#pragma once
#include "platform.h"

namespace obby {

/**
 * @brief Moving platform ping-ponging along a displacement delta vector.
 * Demonstrates multi-level inheritance (Entity -> Platform -> MovingPlatform),
 * overriding virtual update to animate position, computing frame displacement,
 * and standing-on-platform player mechanics.
 */
class MovingPlatform : public Platform {
private:
    Vec3 m_basePos;
    Vec3 m_delta;
    float m_period;
    float m_elapsedTime;
    Vec3 m_frameDisplacement;

public:
    MovingPlatform(const Vec3& basePos, const Vec3& size, const Vec3& delta,
                   float period, int colorIndex = 0, int worldId = 1);
    virtual ~MovingPlatform() override = default;

    const Vec3& getBasePos() const { return m_basePos; }
    const Vec3& getDelta() const { return m_delta; }
    float getPeriod() const { return m_period; }
    const Vec3& getFrameDisplacement() const { return m_frameDisplacement; }

    virtual void update(float dt, Player& player) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "MovingPlatform"; }
};

} // namespace obby

using MovingPlatform = obby::MovingPlatform;
