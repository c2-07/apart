#include "door.h"
#include "constants.h"
#include "render_utils.h"

namespace obby {

Door::Door(int id, const Vec3& center, const Vec3& size)
    : Entity(center, size, Color{ 60, 130, 210, 255 }),
      m_id(id),
      m_isOpen(false) {}

void Door::update(float dt, Player& player) {
    (void)dt;
    (void)player;
    // Door state is controlled by linked buttons in the level
}

void Door::draw() const {
    Vector3 center = m_position.toRaylib();
    Vector3 size = m_size.toRaylib();

    if (!m_isOpen) {
        // Solid closed door
        Color doorCol = m_color;
        Color wireCol = DarkenColor(doorCol, Constants::OUTLINE_DARKEN_FACTOR);
        DrawCube(center, size.x, size.y, size.z, doorCol);
        DrawCubeWires(center, size.x + Constants::OUTLINE_EXPANSION, size.y + Constants::OUTLINE_EXPANSION, size.z + Constants::OUTLINE_EXPANSION, wireCol);

        // Security reinforcement bands
        float bandH = size.y * 0.12f;
        Vector3 band1 = { center.x, center.y + size.y * 0.25f, center.z };
        Vector3 band2 = { center.x, center.y - size.y * 0.25f, center.z };
        DrawCube(band1, size.x + 0.02f, bandH, size.z + 0.02f, Color{ 35, 45, 60, 255 });
        DrawCube(band2, size.x + 0.02f, bandH, size.z + 0.02f, Color{ 35, 45, 60, 255 });
    } else {
        // Open door frame with semi-transparent energy barrier
        DrawCubeWires(center, size.x + 0.04f, size.y + 0.04f, size.z + 0.04f, Color{ 80, 220, 140, 200 });
        DrawCube(center, size.x * 0.95f, size.y * 0.95f, size.z * 0.15f, Color{ 60, 230, 120, 75 });
    }
}

} // namespace obby
