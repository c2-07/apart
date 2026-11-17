#include "bump_block.h"
#include "constants.h"
#include "render_utils.h"
#include "audio.h"
#include <cmath>
#include <algorithm>

namespace obby {

ParticleSystem* BumpBlock::s_particles = nullptr;

BumpBlock::BumpBlock(const Vec3& position, int coins, int worldId, const Vec3& size)
    : Entity(position, size, WHITE),
      m_coinsRemaining(std::max(0, coins)),
      m_worldId(worldId),
      m_bounceTimer(0.0f),
      m_coinPopTimer(0.0f) {}

void BumpBlock::triggerBump(Player& player) {
    if (m_coinsRemaining <= 0) return;

    m_coinsRemaining--;
    m_bounceTimer = Constants::BUMP_BLOCK_BOUNCE_TIME;
    m_coinPopTimer = Constants::COIN_POP_DURATION;

    player.addCoins(1);
    AudioManager::Play(SoundId::CHECKPOINT);

    if (s_particles) {
        Vector3 coinPos = { m_position.x, m_position.y + m_size.y * 0.5f + 0.4f, m_position.z };
        s_particles->SpawnCheckpointBurst(coinPos);
    }
}

void BumpBlock::update(float dt, Player& player) {
    if (m_bounceTimer > 0.0f) {
        m_bounceTimer -= dt;
        if (m_bounceTimer < 0.0f) m_bounceTimer = 0.0f;
    }

    if (m_coinPopTimer > 0.0f) {
        m_coinPopTimer -= dt;
        if (m_coinPopTimer < 0.0f) m_coinPopTimer = 0.0f;
    }

    // Only non-empty blocks can be bumped
    if (m_coinsRemaining <= 0) return;

    // Detect impact from below while player is moving upward or just resolved at ceiling
    AABB blockBounds = getBounds();
    AABB playerBounds = player.getBounds();

    float blockBottomY = blockBounds.getMin().y;
    float playerHeadY = playerBounds.getMax().y;

    // Horizontal overlap check
    bool overlapX = (playerBounds.getMin().x < blockBounds.getMax().x - 0.05f) &&
                    (playerBounds.getMax().x > blockBounds.getMin().x + 0.05f);
    bool overlapZ = (playerBounds.getMin().z < blockBounds.getMax().z - 0.05f) &&
                    (playerBounds.getMax().z > blockBounds.getMin().z + 0.05f);

    // Player head touching bottom face and feet below block
    bool touchingBottom = (playerHeadY >= blockBottomY - 0.15f) &&
                          (playerHeadY <= blockBottomY + 0.35f) &&
                          (playerBounds.getMin().y < blockBottomY);

    if (overlapX && overlapZ && touchingBottom) {
        if (player.getVelocity().y >= -0.2f) {
            triggerBump(player);
        }
    }
}

void BumpBlock::draw() const {
    Vector3 drawPos = m_position.toRaylib();
    if (m_bounceTimer > 0.0f) {
        float p = 1.0f - (m_bounceTimer / Constants::BUMP_BLOCK_BOUNCE_TIME);
        drawPos.y += sinf(p * PI) * Constants::BUMP_BLOCK_BOUNCE_HEIGHT;
    }

    if (m_coinsRemaining > 0) {
        WorldPalette pal = GetWorldPalette(m_worldId);
        Color baseCol = pal.colors[2];
        Color wireCol = DarkenColor(baseCol, 0.70f);

        DrawCube(drawPos, m_size.x, m_size.y, m_size.z, baseCol);
        DrawCubeWires(drawPos, m_size.x + 0.02f, m_size.y + 0.02f, m_size.z + 0.02f, wireCol);

        // Draw "?" question mark on front (+Z) and back (-Z)
        Color qCol = WHITE;
        float halfZ = m_size.z * 0.5f + 0.01f;

        // Front face (+Z)
        DrawCube(Vector3{ drawPos.x, drawPos.y + 0.22f, drawPos.z + halfZ }, 0.36f, 0.08f, 0.02f, qCol);
        DrawCube(Vector3{ drawPos.x + 0.14f, drawPos.y + 0.14f, drawPos.z + halfZ }, 0.08f, 0.12f, 0.02f, qCol);
        DrawCube(Vector3{ drawPos.x, drawPos.y + 0.06f, drawPos.z + halfZ }, 0.22f, 0.08f, 0.02f, qCol);
        DrawCube(Vector3{ drawPos.x, drawPos.y - 0.02f, drawPos.z + halfZ }, 0.08f, 0.08f, 0.02f, qCol);
        DrawCube(Vector3{ drawPos.x, drawPos.y - 0.20f, drawPos.z + halfZ }, 0.08f, 0.08f, 0.02f, qCol);

        // Back face (-Z)
        DrawCube(Vector3{ drawPos.x, drawPos.y + 0.22f, drawPos.z - halfZ }, 0.36f, 0.08f, 0.02f, qCol);
        DrawCube(Vector3{ drawPos.x - 0.14f, drawPos.y + 0.14f, drawPos.z - halfZ }, 0.08f, 0.12f, 0.02f, qCol);
        DrawCube(Vector3{ drawPos.x, drawPos.y + 0.06f, drawPos.z - halfZ }, 0.22f, 0.08f, 0.02f, qCol);
        DrawCube(Vector3{ drawPos.x, drawPos.y - 0.02f, drawPos.z - halfZ }, 0.08f, 0.08f, 0.02f, qCol);
        DrawCube(Vector3{ drawPos.x, drawPos.y - 0.20f, drawPos.z - halfZ }, 0.08f, 0.08f, 0.02f, qCol);
    } else {
        // Empty block: plain neutral gray
        Color emptyCol = Color{ 130, 130, 135, 255 };
        Color emptyWire = Color{ 75, 75, 80, 255 };
        DrawCube(drawPos, m_size.x, m_size.y, m_size.z, emptyCol);
        DrawCubeWires(drawPos, m_size.x + 0.02f, m_size.y + 0.02f, m_size.z + 0.02f, emptyWire);
    }

    // Released coin popping animation above the block
    if (m_coinPopTimer > 0.0f) {
        float progress = 1.0f - (m_coinPopTimer / Constants::COIN_POP_DURATION);
        float coinY = drawPos.y + m_size.y * 0.5f + 0.2f + sinf(progress * PI) * Constants::COIN_POP_HEIGHT;
        float alpha = (progress > 0.7f) ? (1.0f - progress) / 0.3f : 1.0f;
        Color coinColor = Color{ 255, 215, 0, static_cast<unsigned char>(alpha * 255.0f) };
        Color coinWire = Color{ 190, 145, 0, static_cast<unsigned char>(alpha * 255.0f) };

        Vector3 coinCenter = { drawPos.x, coinY, drawPos.z };
        DrawCube(coinCenter, 0.35f, 0.45f, 0.12f, coinColor);
        DrawCubeWires(coinCenter, 0.37f, 0.47f, 0.14f, coinWire);
    }
}

} // namespace obby
