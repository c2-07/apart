#include "spinner.h"
#include "player.h"
#include "audio.h"
#include "constants.h"
#include "render_utils.h"
#include "rlgl.h"
#include <cmath>
#include <algorithm>

namespace obby {

Spinner::Spinner(const Vec3& center, float length, float height, float speed)
    : Entity(center, Vec3{ length, height, length }, Color{ 230, 45, 70, 255 }),
      m_length(length),
      m_height(height),
      m_speed(speed),
      m_angle(0.0f) {}

bool Spinner::checkOverlapWithPlayer(const AABB& playerAABB) const {
    float halfH = m_height * 0.5f;
    if (playerAABB.getMax().y < (m_position.y - halfH) || playerAABB.getMin().y > (m_position.y + halfH)) {
        return false;
    }

    float cosA = cosf(m_angle);
    float sinA = sinf(m_angle);
    float halfL = m_length * 0.5f;
    constexpr float barThickness = 0.45f;

    float pCenterX = (playerAABB.getMin().x + playerAABB.getMax().x) * 0.5f - m_position.x;
    float pCenterZ = (playerAABB.getMin().z + playerAABB.getMax().z) * 0.5f - m_position.z;
    float pExtX = (playerAABB.getMax().x - playerAABB.getMin().x) * 0.5f;
    float pExtZ = (playerAABB.getMax().z - playerAABB.getMin().z) * 0.5f;

    float localX = cosA * pCenterX + sinA * pCenterZ;
    float localZ = -sinA * pCenterX + cosA * pCenterZ;

    float effectiveHalfL = halfL + std::abs(cosA) * pExtX + std::abs(sinA) * pExtZ;
    float effectiveHalfT = barThickness * 0.5f + std::abs(sinA) * pExtX + std::abs(cosA) * pExtZ;

    return (std::abs(localX) <= effectiveHalfL) && (std::abs(localZ) <= effectiveHalfT);
}

void Spinner::update(float dt, Player& player) {
    m_angle += m_speed * dt;
    if (checkOverlapWithPlayer(player.GetAABB())) {
        onPlayerTouch(player);
    }
}

void Spinner::onPlayerTouch(Player& player) {
    player.deathCount++;
    player.Respawn();
    AudioManager::Play(SoundId::DEATH);
}

void Spinner::draw() const {
    Vector3 center = m_position.toRaylib();

    // Center mounting pillar
    DrawCube(center, 0.7f, m_height + 0.2f, 0.7f, Color{ 55, 60, 75, 255 });
    DrawCubeWires(center, 0.72f, m_height + 0.22f, 0.72f, Color{ 35, 40, 50, 255 });

    // Center warning flashing beacon
    float beaconPulse = (sinf(static_cast<float>(GetTime()) * 8.0f) + 1.0f) * 0.5f;
    Color beaconCol = Color{ 255, static_cast<unsigned char>(50 + beaconPulse * 150), 40, 255 };
    DrawCube(Vector3{ center.x, center.y + m_height * 0.5f + 0.25f, center.z }, 0.45f, 0.35f, 0.45f, beaconCol);

    // Rotating arm with alternating warning segments
    rlPushMatrix();
    rlTranslatef(center.x, center.y, center.z);
    rlRotatef(m_angle * RAD2DEG, 0.0f, 1.0f, 0.0f);

    int segments = std::max(2, static_cast<int>(m_length / 1.5f));
    float segLen = m_length / static_cast<float>(segments);
    for (int s = 0; s < segments; ++s) {
        float segX = -m_length * 0.5f + (s + 0.5f) * segLen;
        Color segCol = (s % 2 == 0) ? Color{ 235, 45, 65, 255 } : Color{ 255, 215, 45, 255 };
        DrawCube(Vector3{ segX, 0.0f, 0.0f }, segLen * 0.96f, m_height * 0.75f, 0.45f, segCol);
        DrawCubeWires(Vector3{ segX, 0.0f, 0.0f }, segLen * 0.98f, m_height * 0.77f, 0.47f, DarkenColor(segCol, 0.7f));
    }

    rlPopMatrix();
}

} // namespace obby
