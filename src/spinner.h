#pragma once
#include "entity.h"

namespace obby {

/**
 * @brief Rotating hazard bar entity with warning beacon.
 * Demonstrates continuous rotational physics, rotated bounding collision,
 * and onPlayerTouch death response.
 */
class Spinner : public Entity {
private:
    float m_length;
    float m_height;
    float m_speed;
    float m_angle;

public:
    Spinner(const Vec3& center, float length, float height, float speed);
    virtual ~Spinner() override = default;

    float getLength() const { return m_length; }
    float getHeight() const { return m_height; }
    float getSpeed() const { return m_speed; }
    float getAngle() const { return m_angle; }

    bool checkOverlapWithPlayer(const AABB& playerAABB) const;

    virtual void update(float dt, Player& player) override;
    virtual void onPlayerTouch(Player& player) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "Spinner"; }
    virtual bool isSolid() const override { return false; }
};

} // namespace obby

using Spinner = obby::Spinner;
