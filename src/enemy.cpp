#include "enemy.h"
#include "constants.h"
#include "audio.h"
#include <cmath>

namespace obby {

Enemy::Enemy(const Vec3& position, const Vec3& size, Color color)
    : Entity(position, size, color),
      m_facingDirection(1),
      m_isAlive(true),
      m_isFlattened(false),
      m_flattenTimer(0.0f),
      m_startPos(position) {}

void Enemy::onStomped() {
    m_isAlive = false;
    m_isFlattened = true;
    m_flattenTimer = Constants::ENEMY_FLATTEN_TIME;
    AudioManager::Play(SoundId::JUMP);
}

void Enemy::onHitPlayer(Player& player) {
    Vector2 knockDir = { (player.getPosition().x < m_position.x) ? -1.0f : 1.0f, 0.0f };
    player.takeDamage(knockDir);
}

bool Enemy::isExpired() const {
    return !m_isAlive && (!m_isFlattened || m_flattenTimer <= 0.0f);
}

void Enemy::update(float dt, Player& player) {
    if (m_isFlattened) {
        m_flattenTimer -= dt;
        return;
    }

    if (!m_isAlive) return;

    // Polymorphic autonomous behavior
    think(dt);

    // Overlap and stomp check against player
    AABB enemyBounds = getBounds();
    AABB playerBounds = player.getBounds();

    if (enemyBounds.intersects(playerBounds)) {
        float enemyMidY = enemyBounds.getCenter().y;
        float playerFeetY = playerBounds.getMin().y;

        // Stomp condition: Player feet above enemy midpoint and moving downward
        if (playerFeetY >= enemyMidY - 0.15f && player.getVelocity().y <= 0.0f) {
            onStomped();
            Vec3 vel = player.getVelocity();
            vel.y = Constants::STOMP_BOUNCE;
            player.setVelocity(vel);
            player.setGrounded(false);
        } else {
            onHitPlayer(player);
        }
    }
}

} // namespace obby
