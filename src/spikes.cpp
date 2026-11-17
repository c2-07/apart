#include "spikes.h"
#include "player.h"
#include "audio.h"
#include "constants.h"
#include "render_utils.h"
#include <algorithm>

namespace obby {

Spikes::Spikes(const Vec3& position, const Vec3& size)
    : Entity(position, size, Color{ 225, 45, 45, 255 }) {}

void Spikes::update(float dt, Player& player) {
    (void)dt;
    if (getBounds().intersects(player.getBounds())) {
        onPlayerTouch(player);
    }
}

void Spikes::onPlayerTouch(Player& player) {
    player.setDeathCount(player.getDeathCount() + 1);
    player.respawn();
    AudioManager::Play(SoundId::DEATH);
}

void Spikes::draw() const {
    Vector3 center = m_position.toRaylib();
    Vector3 size = m_size.toRaylib();

    // Hazard-trimmed base block
    float baseH = std::min(0.2f, size.y * 0.4f);
    Vector3 basePos = { center.x, center.y - size.y * 0.5f + baseH * 0.5f, center.z };
    DrawCube(basePos, size.x, baseH, size.z, Color{ 45, 40, 50, 255 });
    DrawCubeWires(basePos, size.x + Constants::OUTLINE_EXPANSION, baseH + Constants::OUTLINE_EXPANSION, size.z + Constants::OUTLINE_EXPANSION, Color{ 25, 20, 30, 255 });

    // Pyramid spikes with silver tips
    int countX = std::max(1, static_cast<int>(size.x / 1.0f));
    int countZ = std::max(1, static_cast<int>(size.z / 1.0f));
    float stepX = size.x / static_cast<float>(countX);
    float stepZ = size.z / static_cast<float>(countZ);
    float spikeH = size.y - baseH;

    for (int ix = 0; ix < countX; ++ix) {
        for (int iz = 0; iz < countZ; ++iz) {
            float px = center.x - size.x * 0.5f + (ix + 0.5f) * stepX;
            float pz = center.z - size.z * 0.5f + (iz + 0.5f) * stepZ;
            Vector3 sPos = { px, center.y - size.y * 0.5f + baseH + spikeH * 0.45f, pz };

            // Hazard body
            DrawCube(sPos, stepX * 0.65f, spikeH * 0.9f, stepZ * 0.65f, Color{ 225, 45, 45, 255 });
            DrawCubeWires(sPos, stepX * 0.66f, spikeH * 0.91f, stepZ * 0.66f, Color{ 140, 25, 25, 255 });

            // Metallic silver tip
            Vector3 tipPos = { px, sPos.y + spikeH * 0.45f + 0.05f, pz };
            DrawCube(tipPos, stepX * 0.35f, 0.10f, stepZ * 0.35f, Color{ 230, 235, 245, 255 });
        }
    }
}

} // namespace obby
