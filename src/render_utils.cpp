#include "render_utils.h"
#include <cmath>
#include <algorithm>

namespace obby {

Color DarkenColor(Color c, float factor) {
    return Color{
        static_cast<unsigned char>(c.r * factor),
        static_cast<unsigned char>(c.g * factor),
        static_cast<unsigned char>(c.b * factor),
        c.a
    };
}

Color LightenColor(Color c, float factor) {
    return Color{
        static_cast<unsigned char>(std::min(255, static_cast<int>(c.r * factor + 18))),
        static_cast<unsigned char>(std::min(255, static_cast<int>(c.g * factor + 18))),
        static_cast<unsigned char>(std::min(255, static_cast<int>(c.b * factor + 18))),
        c.a
    };
}

float PositionHash(Vector3 pos) {
    int ix = static_cast<int>(std::floor(pos.x * 4.0f));
    int iy = static_cast<int>(std::floor(pos.y * 4.0f));
    int iz = static_cast<int>(std::floor(pos.z * 4.0f));
    unsigned int h = static_cast<unsigned int>(ix * 73856093 ^ iy * 19349663 ^ iz * 83492791);
    return static_cast<float>(h % 1000) / 1000.0f;
}

WorldPalette GetWorldPalette(int worldId) {
    WorldPalette pal;
    switch (worldId) {
        case 2: // Ocean
            pal.skyTop       = Color{ 45, 125, 215, 255 };
            pal.skyBottom    = Color{ 150, 225, 240, 255 };
            pal.fogColor     = Color{ 150, 225, 240, 255 };
            pal.lightColor   = Color{ 255, 250, 230, 255 };
            pal.ambientColor = Color{ 120, 160, 195, 255 };
            pal.fogDensity   = 0.0065f;
            pal.colors[0]    = Color{ 40, 160, 200, 255 };
            pal.colors[1]    = Color{ 235, 190, 120, 255 };
            pal.colors[2]    = Color{ 70, 120, 180, 255 };
            pal.colors[3]    = Color{ 250, 215, 90, 255 };
            pal.hazard       = Color{ 230, 45, 80, 255 };
            pal.accent       = Color{ 50, 230, 190, 255 };
            break;
        case 3: // Winter
            pal.skyTop       = Color{ 95, 130, 175, 255 };
            pal.skyBottom    = Color{ 220, 235, 250, 255 };
            pal.fogColor     = Color{ 220, 235, 250, 255 };
            pal.lightColor   = Color{ 240, 245, 255, 255 };
            pal.ambientColor = Color{ 150, 175, 210, 255 };
            pal.fogDensity   = 0.0075f;
            pal.colors[0]    = Color{ 140, 185, 225, 255 };
            pal.colors[1]    = Color{ 240, 248, 255, 255 };
            pal.colors[2]    = Color{ 90, 130, 170, 255 };
            pal.colors[3]    = Color{ 180, 220, 240, 255 };
            pal.hazard       = Color{ 235, 55, 75, 255 };
            pal.accent       = Color{ 80, 210, 235, 255 };
            break;
        case 4: // Factory
            pal.skyTop       = Color{ 50, 55, 70, 255 };
            pal.skyBottom    = Color{ 180, 140, 95, 255 };
            pal.fogColor     = Color{ 140, 115, 90, 255 };
            pal.lightColor   = Color{ 255, 240, 210, 255 };
            pal.ambientColor = Color{ 110, 95, 85, 255 };
            pal.fogDensity   = 0.0060f;
            pal.colors[0]    = Color{ 110, 115, 125, 255 };
            pal.colors[1]    = Color{ 215, 130, 45, 255 };
            pal.colors[2]    = Color{ 70, 75, 85, 255 };
            pal.colors[3]    = Color{ 175, 180, 190, 255 };
            pal.hazard       = Color{ 235, 40, 40, 255 };
            pal.accent       = Color{ 255, 200, 45, 255 };
            break;
        case 1: // Meadow (Default)
        default:
            pal.skyTop       = Color{ 60, 145, 230, 255 };
            pal.skyBottom    = Color{ 180, 220, 245, 255 };
            pal.fogColor     = Color{ 180, 220, 245, 255 };
            pal.lightColor   = Color{ 255, 250, 235, 255 };
            pal.ambientColor = Color{ 130, 155, 180, 255 };
            pal.fogDensity   = 0.0055f;
            pal.colors[0]    = Color{ 65, 160, 75, 255 };
            pal.colors[1]    = Color{ 215, 155, 75, 255 };
            pal.colors[2]    = Color{ 70, 135, 195, 255 };
            pal.colors[3]    = Color{ 190, 100, 180, 255 };
            pal.hazard       = Color{ 225, 50, 50, 255 };
            pal.accent       = Color{ 250, 215, 45, 255 };
            break;
    }
    return pal;
}

} // namespace obby
