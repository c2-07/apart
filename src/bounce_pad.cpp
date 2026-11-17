#include "bounce_pad.h"
#include "player.h"
#include "audio.h"
#include "constants.h"
#include "render_utils.h"
#include <cmath>

namespace obby {

BouncePad::BouncePad(const Vec3& position, const Vec3& size, float strength)
    : Entity(position, size, Color{ 40, 200, 220, 255 }),
      m_strength(strength) {}

void BouncePad::update(float dt, Player& player) {
    (void)dt;
    AABB padAABB = getBounds();
    AABB playerAABB = player.GetAABB();

    bool standingOnPad = player.isGrounded &&
                         (std::abs(player.position.y - padAABB.getMax().y) < 0.08f) &&
                         (playerAABB.getMin().x < padAABB.getMax().x && playerAABB.getMax().x > padAABB.getMin().x) &&
                         (playerAABB.getMin().z < padAABB.getMax().z && playerAABB.getMax().z > padAABB.getMin().z);

    if (standingOnPad) {
        onPlayerTouch(player);
    }
}

void BouncePad::onPlayerTouch(Player& player) {
    player.velocity.y = m_strength;
    player.isGrounded = false;
    AudioManager::Play(SoundId::JUMP);
}

void BouncePad::draw() const {
    Vector3 center = m_position.toRaylib();
    Vector3 size = m_size.toRaylib();

    // Base body
    DrawCube(center, size.x, size.y, size.z, Color{ 35, 42, 55, 255 });
    DrawCubeWires(center, size.x + Constants::OUTLINE_EXPANSION, size.y + Constants::OUTLINE_EXPANSION, size.z + Constants::OUTLINE_EXPANSION, Color{ 20, 25, 35, 255 });

    // Concentric launch rings on top face
    float topY = center.y + size.y * 0.5f + Constants::SURFACE_DECAL_OFFSET_Y;

    // Outer cyan perimeter ring
    DrawCube(Vector3{ center.x, topY, center.z }, size.x * 0.85f, 0.04f, size.z * 0.85f, Color{ 40, 210, 240, 255 });

    // Inner bright launch bullseye
    DrawCube(Vector3{ center.x, topY + 0.01f, center.z }, size.x * 0.52f, 0.04f, size.z * 0.52f, Color{ 255, 60, 140, 255 });

    // Center launch chevron / spring dot
    DrawCube(Vector3{ center.x, topY + 0.02f, center.z }, size.x * 0.22f, 0.04f, size.z * 0.22f, Color{ 255, 240, 100, 255 });
}

} // namespace obby
