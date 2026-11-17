#include "saw.h"
#include "constants.h"
#include "render_utils.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>

namespace obby {

Saw::Saw(const Vec3& p1, const Vec3& p2, float speed, float radius)
    : Entity(p1, Vec3{ radius * 2.0f, radius * 2.0f, radius * 2.0f }, Color{ 190, 190, 195, 255 }),
      m_pointA(p1),
      m_pointB(p2),
      m_speed(speed > 0.0f ? speed : Constants::SAW_DEFAULT_SPEED),
      m_radius(radius > 0.1f ? radius : Constants::SAW_DEFAULT_RADIUS),
      m_rotationAngle(0.0f),
      m_progress(0.0f),
      m_direction(1) {}

bool Saw::checkCircleOverlap(const AABB& box) const {
    // Distance from saw center to closest point on player's bounding box
    float closestX = std::clamp(m_position.x, box.getMin().x, box.getMax().x);
    float closestY = std::clamp(m_position.y, box.getMin().y, box.getMax().y);
    float closestZ = std::clamp(m_position.z, box.getMin().z, box.getMax().z);

    float dx = m_position.x - closestX;
    float dy = m_position.y - closestY;
    float dz = m_position.z - closestZ;

    return (dx * dx + dy * dy + dz * dz) <= (m_radius * m_radius);
}

void Saw::update(float dt, Player& player) {
    // 1. Continuous rotation
    m_rotationAngle += dt * Constants::SAW_ROTATION_SPEED;
    if (m_rotationAngle > 2.0f * PI) {
        m_rotationAngle -= 2.0f * PI;
    }

    // 2. Track interpolation
    Vec3 track = m_pointB - m_pointA;
    float trackLen = track.length();

    if (trackLen > 0.001f) {
        float step = (m_speed * dt) / trackLen;
        m_progress += m_direction * step;

        if (m_progress >= 1.0f) {
            m_progress = 1.0f;
            m_direction = -1;
        } else if (m_progress <= 0.0f) {
            m_progress = 0.0f;
            m_direction = 1;
        }

        m_position = m_pointA + track * m_progress;
    } else {
        m_position = m_pointA;
    }

    // 3. Circle-vs-AABB collision check with player
    if (checkCircleOverlap(player.getBounds())) {
        Vector2 knockDir = { (player.getPosition().x < m_position.x) ? -1.0f : 1.0f, 0.0f };
        player.takeDamage(knockDir);
    }
}

void Saw::draw() const {
    // 1. Draw track line and endpoint mounting brackets
    Vector3 startPos = m_pointA.toRaylib();
    Vector3 endPos = m_pointB.toRaylib();
    Color railColor = Color{ 55, 55, 60, 255 };
    Color mountColor = Color{ 40, 40, 45, 255 };

    DrawLine3D(startPos, endPos, railColor);
    DrawCube(startPos, 0.22f, 0.22f, 0.22f, mountColor);
    DrawCube(endPos, 0.22f, 0.22f, 0.22f, mountColor);

    // 2. Draw saw disc
    Vector3 center = m_position.toRaylib();
    float thickness = 0.08f;
    Vector3 frontCenter = { center.x, center.y, center.z + thickness * 0.5f };
    Vector3 backCenter = { center.x, center.y, center.z - thickness * 0.5f };

    Color discColor = Color{ 175, 175, 182, 255 };
    Color toothColor = Color{ 220, 222, 228, 255 };
    Color hubColor = Color{ 45, 45, 50, 255 };

    float innerR = m_radius * 0.72f;
    float outerR = m_radius;

    // Central circular body
    DrawCylinderEx(backCenter, frontCenter, innerR, innerR, 16, discColor);

    // 3. Triangular teeth radiating from inner disc to outer radius
    constexpr int NUM_TEETH = 12;
    float toothSpan = (2.0f * PI) / NUM_TEETH;

    for (int i = 0; i < NUM_TEETH; ++i) {
        float baseAngle1 = m_rotationAngle + i * toothSpan;
        float baseAngle2 = baseAngle1 + toothSpan * 0.45f;
        float tipAngle = baseAngle1 + toothSpan * 0.85f;

        Vector3 v1 = { center.x + cosf(baseAngle1) * innerR, center.y + sinf(baseAngle1) * innerR, center.z };
        Vector3 v2 = { center.x + cosf(baseAngle2) * innerR, center.y + sinf(baseAngle2) * innerR, center.z };
        Vector3 v3 = { center.x + cosf(tipAngle) * outerR, center.y + sinf(tipAngle) * outerR, center.z };

        // Front face
        DrawTriangle3D(Vector3{ v1.x, v1.y, center.z + thickness * 0.5f },
                       Vector3{ v3.x, v3.y, center.z + thickness * 0.5f },
                       Vector3{ v2.x, v2.y, center.z + thickness * 0.5f },
                       toothColor);

        // Back face
        DrawTriangle3D(Vector3{ v1.x, v1.y, center.z - thickness * 0.5f },
                       Vector3{ v2.x, v2.y, center.z - thickness * 0.5f },
                       Vector3{ v3.x, v3.y, center.z - thickness * 0.5f },
                       toothColor);
    }

    // 4. Center hub bolt
    DrawCylinderEx(backCenter, frontCenter, m_radius * 0.18f, m_radius * 0.18f, 8, hubColor);
}

} // namespace obby
