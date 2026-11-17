#include "crumble_tile.h"
#include "player.h"
#include "constants.h"
#include "render_utils.h"
#include <cmath>
#include <algorithm>

namespace obby {

CrumbleTile::CrumbleTile(const Vec3& position, const Vec3& size, float delay,
                         float respawnTime, int worldId)
    : Entity(position, size, Color{ 175, 140, 100, 255 }),
      m_basePos(position),
      m_delay(delay),
      m_respawnTime(respawnTime),
      m_state(State::STABLE),
      m_timer(0.0f),
      m_worldId(worldId) {}

void CrumbleTile::update(float dt, Player& player) {
    AABB cAABB = getBounds();
    AABB playerAABB = player.GetAABB();

    bool playerStanding = player.isGrounded &&
                          (std::abs(player.position.y - cAABB.getMax().y) < 0.05f) &&
                          (playerAABB.getMin().x < cAABB.getMax().x && playerAABB.getMax().x > cAABB.getMin().x) &&
                          (playerAABB.getMin().z < cAABB.getMax().z && playerAABB.getMax().z > cAABB.getMin().z);

    switch (m_state) {
        case State::STABLE:
            if (playerStanding) {
                m_state = State::CRUMBLING;
                m_timer = m_delay;
            }
            break;
        case State::CRUMBLING:
            m_timer -= dt;
            if (m_timer <= 0.0f) {
                m_state = State::FALLEN;
                m_timer = m_respawnTime;
            }
            break;
        case State::FALLEN:
            m_timer -= dt;
            m_position.y -= 15.0f * dt;
            if (m_timer <= 0.0f) {
                m_state = State::STABLE;
                m_position = m_basePos;
                m_timer = 0.0f;
            }
            break;
    }
}

void CrumbleTile::draw() const {
    if (m_state == State::FALLEN) return;

    Color tileColor = m_color;
    Vector3 drawPos = m_position.toRaylib();
    Vector3 size = m_size.toRaylib();

    if (m_state == State::CRUMBLING) {
        float shake = sinf(m_timer * 35.0f) * 0.05f;
        drawPos.x += shake;
        tileColor = Color{ 230, 90, 60, 255 }; // Warning amber-red shift
    }

    Color wireCol = DarkenColor(tileColor, Constants::OUTLINE_DARKEN_FACTOR);
    DrawCube(drawPos, size.x, size.y, size.z, tileColor);
    DrawCubeWires(drawPos, size.x + Constants::OUTLINE_EXPANSION, size.y + Constants::OUTLINE_EXPANSION, size.z + Constants::OUTLINE_EXPANSION, wireCol);

    // Structural fracture cracks on surface
    float crackY = drawPos.y + size.y * 0.5f + 0.005f;
    Color crackCol = Color{ 40, 30, 25, 230 };
    DrawCube(Vector3{ drawPos.x - size.x * 0.2f, crackY, drawPos.z }, size.x * 0.35f, 0.01f, 0.06f, crackCol);
    DrawCube(Vector3{ drawPos.x + size.x * 0.15f, crackY, drawPos.z - size.z * 0.15f }, 0.06f, 0.01f, size.z * 0.30f, crackCol);
    DrawCube(Vector3{ drawPos.x + size.x * 0.2f, crackY, drawPos.z + size.z * 0.18f }, size.x * 0.25f, 0.01f, 0.06f, crackCol);
    DrawCube(Vector3{ drawPos.x - size.x * 0.1f, crackY, drawPos.z + size.z * 0.2f }, 0.05f, 0.01f, size.z * 0.22f, crackCol);
}

} // namespace obby
