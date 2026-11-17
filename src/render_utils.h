#pragma once
#include "raylib.h"
#include "vec3.h"

namespace obby {

struct WorldPalette {
    Color skyTop;
    Color skyBottom;
    Color fogColor;
    Color lightColor;
    Color ambientColor;
    float fogDensity;
    Color colors[4];
    Color hazard;
    Color accent;
};

WorldPalette GetWorldPalette(int worldId);
Color DarkenColor(Color c, float factor);
Color LightenColor(Color c, float factor);
float PositionHash(Vector3 pos);

} // namespace obby

using WorldPalette = obby::WorldPalette;
using obby::GetWorldPalette;
using obby::DarkenColor;
using obby::LightenColor;
using obby::PositionHash;
