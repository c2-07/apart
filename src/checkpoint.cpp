#include "checkpoint.h"
#include "player.h"
#include "audio.h"
#include "constants.h"
#include "render_utils.h"
#include <cmath>

namespace obby {

Checkpoint::Checkpoint(const Vec3& position)
    : Entity(position, Vec3{ 3.0f, 0.5f, 3.0f }, Color{ 220, 225, 235, 255 }),
      m_activated(false),
      m_waveTimer(0.0f) {}

void Checkpoint::update(float dt, Player& player) {
    m_waveTimer += dt;
    if (!m_activated) {
        AABB triggerBox = AABB::fromCenterSize(
            Vec3{ m_position.x, m_position.y + 0.6f, m_position.z },
            Vec3{ 2.4f, 1.4f, 2.4f }
        );
        if (triggerBox.intersects(player.getBounds())) {
            onPlayerTouch(player);
        }
    }
}

void Checkpoint::onPlayerTouch(Player& player) {
    if (!m_activated) {
        m_activated = true;
        player.setSpawnPoint(m_position);
        player.resetHearts();
        AudioManager::Play(SoundId::CHECKPOINT);
    }
}

void Checkpoint::draw() const {
    Vector3 cpPos = m_position.toRaylib();

    // 1. Base Pad: Thin 0.06m box elevated slightly above platform surface
    Vector3 padPos = { cpPos.x, cpPos.y + Constants::CHECKPOINT_PAD_OFFSET_Y, cpPos.z };
    Color padColor = m_activated ? Color{ 245, 220, 60, 255 } : Color{ 175, 180, 195, 255 };
    Color padWire = DarkenColor(padColor, Constants::OUTLINE_DARKEN_FACTOR);
    DrawCube(padPos, 2.6f, Constants::CHECKPOINT_PAD_HEIGHT, 2.6f, padColor);
    DrawCubeWires(padPos, 2.62f, Constants::CHECKPOINT_PAD_HEIGHT + 0.01f, 2.62f, padWire);

    // 2. Pulsing Activation Ring on pad surface
    if (m_activated) {
        float pulse = (sinf(m_waveTimer * Constants::CHECKPOINT_RING_PULSE_SPEED) + 1.0f) * 0.5f;
        float ringSize = 2.0f + pulse * 0.35f;
        unsigned char ringAlpha = static_cast<unsigned char>(160 + pulse * 80);
        DrawCube(Vector3{ padPos.x, padPos.y + 0.035f, padPos.z }, ringSize, 0.02f, ringSize, Color{ 60, 240, 140, ringAlpha });
    }

    // 3. Flagpole: Slender metallic post
    float poleH = 3.2f;
    Vector3 polePos = { cpPos.x - 0.95f, cpPos.y + (poleH * 0.5f), cpPos.z - 0.95f };
    DrawCube(polePos, 0.12f, poleH, 0.12f, Color{ 220, 225, 235, 255 });
    DrawCubeWires(polePos, 0.13f, poleH + 0.02f, 0.13f, Color{ 140, 145, 155, 255 });

    // Gold finial sphere/cube atop pole
    DrawCube(Vector3{ polePos.x, polePos.y + (poleH * 0.5f) + 0.08f, polePos.z }, 0.22f, 0.22f, 0.22f, Color{ 255, 215, 0, 255 });

    // 4. Waving Flag Cloth: 3 connected segments driven by sinusoidal wave
    Color flagCol = m_activated ? Color{ 50, 210, 95, 255 } : Color{ 150, 155, 165, 255 };
    Color flagWire = DarkenColor(flagCol, Constants::OUTLINE_DARKEN_FACTOR);
    float flagBaseY = polePos.y + (poleH * 0.5f) - 0.45f;
    float segW = 0.42f;

    for (int s = 0; s < 3; ++s) {
        float phase = m_waveTimer * Constants::CHECKPOINT_FLAG_WAVE_SPEED - s * 0.85f;
        float waveZ = sinf(phase) * (0.05f + s * 0.045f);
        Vector3 segPos = { polePos.x + 0.06f + (s + 0.5f) * segW, flagBaseY, polePos.z + waveZ };
        DrawCube(segPos, segW, 0.70f, 0.05f, flagCol);
        DrawCubeWires(segPos, segW + 0.01f, 0.71f, 0.06f, flagWire);
    }
}

} // namespace obby
