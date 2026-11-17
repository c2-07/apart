#include "button.h"
#include "player.h"
#include "audio.h"
#include "constants.h"
#include "render_utils.h"
#include <cmath>

namespace obby {

Button::Button(int id, const Vec3& position, float duration)
    : Entity(position, Vec3{ 1.8f, 0.2f, 1.8f }, Color{ 220, 80, 50, 255 }),
      m_id(id),
      m_duration(duration),
      m_currentTimer(0.0f),
      m_isPressed(false),
      m_isActive(false) {}

AABB Button::getBounds() const {
    // Solid base plate
    return AABB::fromCenterSize(m_position, m_size);
}

AABB Button::getTriggerBounds() const {
    // Top trigger volume
    return AABB::fromCenterSize(
        Vec3{ m_position.x, m_position.y + 0.25f, m_position.z },
        Vec3{ 1.4f, 0.3f, 1.4f }
    );
}

void Button::setPressed(bool pressed) {
    if (pressed) {
        m_isPressed = true;
        m_isActive = true;
        m_currentTimer = m_duration;
    } else {
        m_isPressed = false;
    }
}

void Button::onPlayerTouch(Player& player) {
    (void)player;
    setPressed(true);
}

void Button::update(float dt, Player& player) {
    bool touching = getTriggerBounds().intersects(player.GetAABB());
    if (touching) {
        onPlayerTouch(player);
    } else {
        m_isPressed = false;
    }

    if (m_isActive) {
        m_currentTimer -= dt;
        if (m_currentTimer <= 0.0f) {
            m_currentTimer = 0.0f;
            m_isActive = false;
        }
    }
}

void Button::draw() const {
    Vector3 bPos = m_position.toRaylib();

    // Base plate
    DrawCube(bPos, m_size.x, m_size.y, m_size.z, Color{ 60, 65, 80, 255 });
    DrawCubeWires(bPos, m_size.x + 0.02f, m_size.y + 0.02f, m_size.z + 0.02f, Color{ 40, 45, 55, 255 });

    // Moving button head
    float headH = m_isPressed ? 0.05f : 0.16f;
    Vector3 headPos = { bPos.x, bPos.y + 0.1f + headH * 0.5f, bPos.z };
    Color headColor = m_isActive ? Color{ 50, 220, 95, 255 } : Color{ 235, 75, 60, 255 };
    DrawCube(headPos, 1.35f, headH, 1.35f, headColor);
    DrawCubeWires(headPos, 1.37f, headH + 0.02f, 1.37f, DarkenColor(headColor, 0.7f));

    // Countdown indicator ring dots around base
    if (m_isActive && m_duration > 0.0f) {
        float frac = m_currentTimer / m_duration;
        int dots = std::max(1, static_cast<int>(frac * 8.0f));
        float ringRadius = 0.95f;
        for (int i = 0; i < dots; ++i) {
            float ang = (i / 8.0f) * 2.0f * PI;
            Vector3 dotPos = {
                bPos.x + cosf(ang) * ringRadius,
                bPos.y + 0.11f + Constants::SURFACE_DECAL_OFFSET_Y,
                bPos.z + sinf(ang) * ringRadius
            };
            DrawCube(dotPos, 0.12f, 0.04f, 0.12f, Color{ 255, 230, 80, 255 });
        }
    }
}

} // namespace obby
