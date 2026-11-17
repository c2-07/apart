#pragma once
#include "enemy.h"
#include <vector>

namespace obby {

/**
 * @brief Ground patrol enemy that walks horizontally at 2 m/s.
 * Reverses direction upon hitting walls, reaching platform ledges, or reaching its patrol range limit.
 */
class Walker : public Enemy {
private:
    float m_range;
    float m_walkSpeed;

    // Static pointer to level solid colliders for autonomous wall and ledge detection
    static const std::vector<AABB>* s_levelColliders;

public:
    Walker(const Vec3& position, float range = 0.0f, float speed = 2.0f);
    virtual ~Walker() override = default;

    virtual void think(float dt) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "walker"; }

    float getRange() const { return m_range; }
    float getWalkSpeed() const { return m_walkSpeed; }

    static void setLevelColliders(const std::vector<AABB>* colliders) {
        s_levelColliders = colliders;
    }
};

} // namespace obby

using Walker = obby::Walker;
