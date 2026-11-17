#include "finish_zone.h"
#include "player.h"
#include "audio.h"
#include "constants.h"
#include "render_utils.h"
#include "rlgl.h"
#include <cmath>

namespace obby {

FinishZone::FinishZone(const Vec3& position, const Vec3& size)
    : Entity(position, size, Color{ 255, 215, 45, 255 }),
      m_completed(false),
      m_animTimer(0.0f) {}

void FinishZone::update(float dt, Player& player) {
    m_animTimer += dt;
    if (!m_completed) {
        if (getBounds().intersects(player.GetAABB())) {
            onPlayerTouch(player);
        }
    }
}

void FinishZone::onPlayerTouch(Player& player) {
    (void)player;
    if (!m_completed) {
        m_completed = true;
        AudioManager::Play(SoundId::FINISH);
    }
}

void FinishZone::draw() const {
    Vector3 fCenter = m_position.toRaylib();
    Vector3 fSize = m_size.toRaylib();

    // Base pad
    Vector3 basePadPos = { fCenter.x, fCenter.y - fSize.y * 0.5f + Constants::SURFACE_DECAL_OFFSET_Y + 0.02f, fCenter.z };
    DrawCube(basePadPos, fSize.x + 0.4f, 0.08f, fSize.z + 0.4f, Color{ 245, 195, 45, 255 });
    DrawCubeWires(basePadPos, fSize.x + 0.42f, 0.09f, fSize.z + 0.42f, Color{ 185, 140, 20, 255 });

    // Victory Arch: Left & Right Pillars
    float archHeight = fSize.y * 1.35f;
    float pillarDist = fSize.x * 0.48f;
    float pillarW = 0.55f;
    Vector3 leftPillar = { fCenter.x - pillarDist, fCenter.y - fSize.y * 0.5f + archHeight * 0.5f, fCenter.z };
    Vector3 rightPillar = { fCenter.x + pillarDist, fCenter.y - fSize.y * 0.5f + archHeight * 0.5f, fCenter.z };
    Color gold = Color{ 255, 215, 50, 255 };
    Color goldWire = Color{ 190, 150, 20, 255 };

    DrawCube(leftPillar, pillarW, archHeight, pillarW, gold);
    DrawCubeWires(leftPillar, pillarW + 0.02f, archHeight + 0.02f, pillarW + 0.02f, goldWire);
    DrawCube(rightPillar, pillarW, archHeight, pillarW, gold);
    DrawCubeWires(rightPillar, pillarW + 0.02f, archHeight + 0.02f, pillarW + 0.02f, goldWire);

    // Cross Lintel Header Beam
    Vector3 lintelPos = { fCenter.x, fCenter.y - fSize.y * 0.5f + archHeight + 0.3f, fCenter.z };
    float lintelW = fSize.x + 0.8f;
    DrawCube(lintelPos, lintelW, 0.6f, pillarW * 1.15f, gold);
    DrawCubeWires(lintelPos, lintelW + 0.02f, 0.62f, pillarW * 1.15f + 0.02f, goldWire);

    // Pulsing inner portal jewels along the inner jambs
    float portalPulse = (sinf(m_animTimer * 5.0f) + 1.0f) * 0.5f;
    Color gemColor = Color{ 60, static_cast<unsigned char>(200 + portalPulse * 55), 255, 240 };
    DrawCube(Vector3{ leftPillar.x + 0.28f, leftPillar.y, leftPillar.z }, 0.22f, 0.22f, 0.22f, gemColor);
    DrawCube(Vector3{ rightPillar.x - 0.28f, rightPillar.y, rightPillar.z }, 0.22f, 0.22f, 0.22f, gemColor);

    // Victory Star Trophy rotating above arch lintel
    rlPushMatrix();
    rlTranslatef(lintelPos.x, lintelPos.y + 0.75f, lintelPos.z);
    rlRotatef(m_animTimer * 90.0f, 0.0f, 1.0f, 0.0f);
    DrawCube(Vector3{ 0.0f, 0.0f, 0.0f }, 0.45f, 0.45f, 0.45f, Color{ 255, 235, 90, 255 });
    DrawCubeWires(Vector3{ 0.0f, 0.0f, 0.0f }, 0.47f, 0.47f, 0.47f, Color{ 200, 160, 25, 255 });
    // Star cross-spikes
    rlRotatef(45.0f, 0.0f, 0.0f, 1.0f);
    DrawCube(Vector3{ 0.0f, 0.0f, 0.0f }, 0.35f, 0.35f, 0.35f, Color{ 255, 245, 120, 255 });
    rlPopMatrix();
}

} // namespace obby
