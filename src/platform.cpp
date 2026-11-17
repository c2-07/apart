#include "platform.h"
#include "constants.h"
#include "render_utils.h"
#include <algorithm>

namespace obby {

Platform::Platform(const Vec3& position, const Vec3& size, int colorIndex, int worldId)
    : Entity(position, size, WHITE),
      m_colorIndex(std::clamp(colorIndex, 0, 3)),
      m_worldId(worldId) {
    WorldPalette pal = GetWorldPalette(m_worldId);
    m_color = pal.colors[m_colorIndex];
}

void Platform::update(float dt, Player& player) {
    (void)dt;
    (void)player;
    // Static platform has no per-tick logic
}

void Platform::draw() const {
    WorldPalette pal = GetWorldPalette(m_worldId);
    Color faceColor = pal.colors[m_colorIndex];
    Vector3 center = m_position.toRaylib();
    Vector3 size = m_size.toRaylib();

    float h = PositionHash(center);
    float var = (1.0f - Constants::COLOR_VARIATION_INTENSITY) + h * (Constants::COLOR_VARIATION_INTENSITY * 2.0f);
    Color variedColor = Color{
        static_cast<unsigned char>(std::clamp(static_cast<int>(faceColor.r * var), 0, 255)),
        static_cast<unsigned char>(std::clamp(static_cast<int>(faceColor.g * var), 0, 255)),
        static_cast<unsigned char>(std::clamp(static_cast<int>(faceColor.b * var), 0, 255)),
        faceColor.a
    };
    Color bodyWire = DarkenColor(variedColor, Constants::OUTLINE_DARKEN_FACTOR);

    DrawCube(center, size.x, size.y, size.z, variedColor);
    DrawCubeWires(center, size.x + Constants::OUTLINE_EXPANSION, size.y + Constants::OUTLINE_EXPANSION, size.z + Constants::OUTLINE_EXPANSION, bodyWire);

    // Slim top cap with slight overhang in lighter shade for readable jumping ledges
    if (size.y > Constants::PLATFORM_CAP_HEIGHT * 1.5f) {
        float capH = Constants::PLATFORM_CAP_HEIGHT;
        float capOverhang = Constants::PLATFORM_CAP_OVERHANG;
        Vector3 capCenter = { center.x, center.y + (size.y * 0.5f) - (capH * 0.5f), center.z };
        Vector3 capSize = { size.x + capOverhang * 2.0f, capH, size.z + capOverhang * 2.0f };
        Color capColor = LightenColor(variedColor, 1.25f);
        Color capWire = DarkenColor(capColor, Constants::OUTLINE_DARKEN_FACTOR);
        DrawCube(capCenter, capSize.x, capSize.y, capSize.z, capColor);
        DrawCubeWires(capCenter, capSize.x + Constants::OUTLINE_EXPANSION, capSize.y + Constants::OUTLINE_EXPANSION, capSize.z + Constants::OUTLINE_EXPANSION, capWire);
    }
}

} // namespace obby
