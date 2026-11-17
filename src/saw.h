#pragma once
#include "entity.h"
#include "player.h"

namespace obby {

/**
 * @brief Circular hazard saw blade moving back and forth along a linear track.
 * Rotates continuously and inflicts damage on contact with the player via circle-vs-box overlap.
 */
class Saw : public Entity {
private:
    Vec3 m_pointA;
    Vec3 m_pointB;
    float m_speed;
    float m_radius;
    float m_rotationAngle;
    float m_progress;
    int m_direction; // +1 = toward pointB, -1 = toward pointA

public:
    Saw(const Vec3& p1, const Vec3& p2, float speed = 2.5f, float radius = 0.8f);
    virtual ~Saw() override = default;

    virtual void update(float dt, Player& player) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "saw"; }

    virtual bool isSolid() const override { return false; }
    virtual AABB getBounds() const override { return AABB::fromCenterSize(m_position, m_size); }

    const Vec3& getPointA() const { return m_pointA; }
    const Vec3& getPointB() const { return m_pointB; }
    float getSpeed() const { return m_speed; }
    float getRadius() const { return m_radius; }
    float getRotationAngle() const { return m_rotationAngle; }
    float getProgress() const { return m_progress; }
    int getDirection() const { return m_direction; }

    bool checkCircleOverlap(const AABB& box) const;
};

} // namespace obby

using Saw = obby::Saw;
