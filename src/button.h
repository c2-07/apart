#pragma once
#include "entity.h"

namespace obby {

/**
 * @brief Timed pressure plate button entity that triggers puzzle mechanisms.
 * Demonstrates timed state management, composite collision bounds (solid base + trigger top),
 * and interactive puzzle activation.
 */
class Button : public Entity {
private:
    int m_id;
    float m_duration;
    float m_currentTimer;
    bool m_isPressed;
    bool m_isActive;

public:
    Button(int id, const Vec3& position, float duration);
    virtual ~Button() override = default;

    int getId() const { return m_id; }
    float getDuration() const { return m_duration; }
    float getCurrentTimer() const { return m_currentTimer; }
    bool isPressed() const { return m_isPressed; }
    bool isActive() const { return m_isActive; }

    void setPressed(bool pressed);
    void setActive(bool active) { m_isActive = active; }
    AABB getTriggerBounds() const;

    virtual void update(float dt, Player& player) override;
    virtual void onPlayerTouch(Player& player) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "Button"; }
    virtual bool isSolid() const override { return true; }
    virtual AABB getBounds() const override;
};

} // namespace obby

using Button = obby::Button;
