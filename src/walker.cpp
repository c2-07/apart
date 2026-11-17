#include "walker.h"
#include "constants.h"
#include "render_utils.h"
#include <cmath>
#include <algorithm>

namespace obby {

const std::vector<AABB>* Walker::s_levelColliders = nullptr;

Walker::Walker(const Vec3& position, float range, float speed)
    : Enemy(position, Vec3{ Constants::WALKER_WIDTH, Constants::WALKER_HEIGHT, Constants::WALKER_WIDTH }, Color{ 235, 95, 45, 255 }),
      m_range(std::max(0.0f, range)),
      m_walkSpeed(speed > 0.0f ? speed : Constants::WALKER_SPEED) {}

void Walker::think(float dt) {
    float dx = m_facingDirection * m_walkSpeed * dt;

    // 1. Range limit boundary turnaround
    if (m_range > 0.05f) {
        float nextDist = (m_position.x + dx) - m_startPos.x;
        if (std::abs(nextDist) > m_range) {
            m_facingDirection = (nextDist > 0.0f) ? -1 : 1;
            return;
        }
    }

    if (s_levelColliders) {
        // 2. Wall detection: check small bounding box in front of walker
        float probeX = m_position.x + dx + m_facingDirection * (m_size.x * 0.5f + 0.08f);
        AABB wallProbe = AABB::fromCenterSize(
            Vec3{ probeX, m_position.y, m_position.z },
            Vec3{ 0.12f, m_size.y * 0.75f, m_size.z * 0.8f }
        );

        for (const auto& solid : *s_levelColliders) {
            if (solid.intersects(wallProbe)) {
                m_facingDirection = -m_facingDirection;
                return;
            }
        }

        // 3. Ledge detection: probe point ahead of and below feet
        float feetY = m_position.y - m_size.y * 0.5f;
        float probeAheadX = m_position.x + m_facingDirection * (m_size.x * 0.5f + 0.15f);
        Vec3 ledgeProbe = Vec3{ probeAheadX, feetY - 0.20f, m_position.z };

        bool solidGroundAhead = false;
        for (const auto& solid : *s_levelColliders) {
            if (solid.contains(ledgeProbe) ||
                (probeAheadX >= solid.getMin().x && probeAheadX <= solid.getMax().x &&
                 m_position.z >= solid.getMin().z - 0.2f && m_position.z <= solid.getMax().z + 0.2f &&
                 std::abs(solid.getMax().y - feetY) < 0.25f)) {
                solidGroundAhead = true;
                break;
            }
        }

        if (!solidGroundAhead) {
            m_facingDirection = -m_facingDirection;
            return;
        }
    }

    // Apply valid horizontal movement
    m_position.x += dx;
}

void Walker::draw() const {
    if (!isActive()) return;

    Vector3 center = m_position.toRaylib();
    float feetY = m_position.y - m_size.y * 0.5f;
    float curH = m_size.y;
    unsigned char alpha = 255;

    // Flattening and fading animation upon defeat
    if (m_isFlattened) {
        float progress = std::clamp(m_flattenTimer / Constants::ENEMY_FLATTEN_TIME, 0.0f, 1.0f);
        curH = m_size.y * (0.15f + 0.85f * progress);
        alpha = static_cast<unsigned char>(progress * 255.0f);
    }

    center.y = feetY + curH * 0.5f;

    Color bodyColor = Color{ 235, 95, 45, alpha };
    Color bodyWire = DarkenColor(bodyColor, 0.75f);
    Color footColor = Color{ 55, 40, 35, alpha };
    Color eyeWhite = Color{ 255, 255, 255, alpha };
    Color eyePupil = Color{ 25, 25, 30, alpha };

    // 1. Main body
    DrawCube(center, m_size.x, curH, m_size.z, bodyColor);
    DrawCubeWires(center, m_size.x + 0.02f, curH + 0.02f, m_size.z + 0.02f, bodyWire);

    // 2. Small stubby feet
    if (!m_isFlattened) {
        float footW = 0.22f;
        float footH = 0.12f;
        float footZ = m_size.z * 0.28f;
        Vector3 leftFoot = { center.x - m_size.x * 0.28f, feetY + footH * 0.5f, center.z + footZ };
        Vector3 rightFoot = { center.x + m_size.x * 0.28f, feetY + footH * 0.5f, center.z + footZ };
        DrawCube(leftFoot, footW, footH, footW, footColor);
        DrawCube(rightFoot, footW, footH, footW, footColor);
    }

    // 3. Two expressive block eyes pointing in facing direction
    float eyeOffX = m_facingDirection * (m_size.x * 0.5f + 0.02f);
    float eyeY = center.y + curH * 0.15f;
    Vector3 leftEye = { center.x + eyeOffX, eyeY, center.z - 0.18f };
    Vector3 rightEye = { center.x + eyeOffX, eyeY, center.z + 0.18f };
    DrawCube(leftEye, 0.04f, 0.16f, 0.14f, eyeWhite);
    DrawCube(rightEye, 0.04f, 0.16f, 0.14f, eyeWhite);
    DrawCube(Vector3{ leftEye.x + m_facingDirection * 0.02f, eyeY, leftEye.z }, 0.04f, 0.08f, 0.08f, eyePupil);
    DrawCube(Vector3{ rightEye.x + m_facingDirection * 0.02f, eyeY, rightEye.z }, 0.04f, 0.08f, 0.08f, eyePupil);
}

} // namespace obby
