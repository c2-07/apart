#include "moving_platform.h"
#include "player.h"
#include "constants.h"
#include "render_utils.h"
#include <cmath>
#include <algorithm>

namespace obby {

MovingPlatform::MovingPlatform(const Vec3& basePos, const Vec3& size, const Vec3& delta,
                               float period, int colorIndex, int worldId)
    : Platform(basePos, size, colorIndex, worldId),
      m_basePos(basePos),
      m_delta(delta),
      m_period(period),
      m_elapsedTime(0.0f),
      m_frameDisplacement(0.0f, 0.0f, 0.0f) {}

void MovingPlatform::update(float dt, Player& player) {
    float prevTime = m_elapsedTime;
    m_elapsedTime += dt;

    float prevProg = sinf(m_period > 0.0f ? prevTime * (2.0f * PI / m_period) : 0.0f);
    float nextProg = sinf(m_period > 0.0f ? m_elapsedTime * (2.0f * PI / m_period) : 0.0f);
    float t0 = (prevProg + 1.0f) * 0.5f;
    float t1 = (nextProg + 1.0f) * 0.5f;

    Vec3 pos0 = m_basePos + m_delta * t0;
    Vec3 pos1 = m_basePos + m_delta * t1;
    m_position = pos1;
    m_frameDisplacement = pos1 - pos0;

    AABB moverAABB = getBounds();
    AABB playerAABB = player.GetAABB();

    bool standingOnTop = player.isGrounded &&
                         (std::abs(player.position.y - moverAABB.getMax().y) < 0.05f) &&
                         (playerAABB.getMin().x < moverAABB.getMax().x && playerAABB.getMax().x > moverAABB.getMin().x) &&
                         (playerAABB.getMin().z < moverAABB.getMax().z && playerAABB.getMax().z > moverAABB.getMin().z);

    if (standingOnTop) {
        player.position.x += m_frameDisplacement.x;
        player.position.y += m_frameDisplacement.y;
        player.position.z += m_frameDisplacement.z;
    }
}

void MovingPlatform::draw() const {
    WorldPalette pal = GetWorldPalette(m_worldId);
    Color faceColor = pal.colors[m_colorIndex];
    Color wireColor = DarkenColor(faceColor, Constants::OUTLINE_DARKEN_FACTOR);
    Vector3 center = m_position.toRaylib();
    Vector3 size = m_size.toRaylib();

    DrawCube(center, size.x, size.y, size.z, faceColor);
    DrawCubeWires(center, size.x + Constants::OUTLINE_EXPANSION, size.y + Constants::OUTLINE_EXPANSION, size.z + Constants::OUTLINE_EXPANSION, wireColor);

    // Top cap with directional chevron stripes
    float capH = 0.08f;
    Vector3 capPos = { center.x, center.y + size.y * 0.5f - capH * 0.5f, center.z };
    Color capCol = LightenColor(faceColor, 1.2f);
    DrawCube(capPos, size.x + 0.04f, capH, size.z + 0.04f, capCol);

    // Chevron movement direction indicator on mover top
    float chevronW = std::min(size.x, size.z) * 0.35f;
    Vector3 chevPos = { center.x, capPos.y + capH * 0.5f + 0.015f, center.z };
    DrawCube(chevPos, chevronW, 0.03f, chevronW, Color{ 255, 230, 80, 255 });

    // Warning bumper trim along bottom edge
    float bumperH = 0.12f;
    Vector3 bumperPos = { center.x, center.y - size.y * 0.5f + bumperH * 0.5f, center.z };
    DrawCube(bumperPos, size.x + 0.03f, bumperH, size.z + 0.03f, Color{ 35, 40, 50, 255 });
}

} // namespace obby
