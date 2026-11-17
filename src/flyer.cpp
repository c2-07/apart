#include "flyer.h"
#include "constants.h"
#include "render_utils.h"
#include <cmath>
#include <algorithm>

namespace obby {

Flyer::Flyer(const Vec3& position, float amplitude, float waveSpeed, float flySpeed, float range)
    : Enemy(position, Vec3{ Constants::FLYER_WIDTH, Constants::FLYER_HEIGHT, Constants::FLYER_WIDTH }, Color{ 145, 65, 215, 255 }),
      m_amplitude(std::max(0.1f, amplitude)),
      m_waveSpeed(std::max(0.1f, waveSpeed)),
      m_flySpeed(flySpeed > 0.0f ? flySpeed : Constants::FLYER_SPEED),
      m_range(range > 0.0f ? range : Constants::FLYER_DEFAULT_RANGE),
      m_elapsedTime(0.0f) {}

void Flyer::think(float dt) {
    m_elapsedTime += dt;

    // Horizontal linear movement with boundary reversal
    float dx = m_facingDirection * m_flySpeed * dt;
    float nextX = m_position.x + dx;

    if (std::abs(nextX - m_startPos.x) > m_range) {
        m_facingDirection = (nextX > m_startPos.x) ? -1 : 1;
        dx = m_facingDirection * m_flySpeed * dt;
    }
    m_position.x += dx;

    // Vertical sine wave oscillation around base spawn height
    m_position.y = m_startPos.y + sinf(m_elapsedTime * m_waveSpeed) * m_amplitude;
}

void Flyer::draw() const {
    if (!isActive()) return;

    Vector3 center = m_position.toRaylib();
    float feetY = m_position.y - m_size.y * 0.5f;
    float curH = m_size.y;
    unsigned char alpha = 255;

    // Flattening upon stomp
    if (m_isFlattened) {
        float progress = std::clamp(m_flattenTimer / Constants::ENEMY_FLATTEN_TIME, 0.0f, 1.0f);
        curH = m_size.y * (0.15f + 0.85f * progress);
        alpha = static_cast<unsigned char>(progress * 255.0f);
    }

    center.y = feetY + curH * 0.5f;

    Color bodyColor = Color{ 145, 65, 215, alpha };
    Color bodyWire = DarkenColor(bodyColor, 0.75f);
    Color wingColor = Color{ 230, 200, 255, alpha };
    Color eyeWhite = Color{ 255, 255, 255, alpha };
    Color eyePupil = Color{ 25, 25, 30, alpha };

    // 1. Aerodynamic blocky body
    DrawCube(center, m_size.x, curH, m_size.z, bodyColor);
    DrawCubeWires(center, m_size.x + 0.02f, curH + 0.02f, m_size.z + 0.02f, bodyWire);

    // 2. Animated flapping wings on Z-sides
    if (!m_isFlattened) {
        float flapAngle = sinf(m_elapsedTime * 12.0f) * 0.18f;
        float wingW = 0.45f;
        float wingH = 0.08f;
        Vector3 leftWing = { center.x, center.y + flapAngle, center.z - m_size.z * 0.5f - wingW * 0.4f };
        Vector3 rightWing = { center.x, center.y + flapAngle, center.z + m_size.z * 0.5f + wingW * 0.4f };
        DrawCube(leftWing, m_size.x * 0.7f, wingH, wingW, wingColor);
        DrawCube(rightWing, m_size.x * 0.7f, wingH, wingW, wingColor);
    }

    // 3. Forward-facing eyes
    float eyeOffX = m_facingDirection * (m_size.x * 0.5f + 0.02f);
    float eyeY = center.y + curH * 0.12f;
    Vector3 leftEye = { center.x + eyeOffX, eyeY, center.z - 0.16f };
    Vector3 rightEye = { center.x + eyeOffX, eyeY, center.z + 0.16f };
    DrawCube(leftEye, 0.04f, 0.14f, 0.12f, eyeWhite);
    DrawCube(rightEye, 0.04f, 0.14f, 0.12f, eyeWhite);
    DrawCube(Vector3{ leftEye.x + m_facingDirection * 0.02f, eyeY, leftEye.z }, 0.04f, 0.08f, 0.06f, eyePupil);
    DrawCube(Vector3{ rightEye.x + m_facingDirection * 0.02f, eyeY, rightEye.z }, 0.04f, 0.08f, 0.06f, eyePupil);
}

} // namespace obby
