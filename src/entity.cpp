#include "entity.h"

namespace obby {

// Initialize static member variable
int Entity::s_entityCount = 0;

Entity::Entity(const Vec3& position, const Vec3& size, Color color)
    : m_position(position), m_size(size), m_color(color) {
    ++s_entityCount;
}

Entity::~Entity() {
    --s_entityCount;
}

void Entity::onPlayerTouch(Player& player) {
    // Default implementation does nothing
    (void)player;
}

AABB Entity::getBounds() const {
    return AABB::fromCenterSize(m_position, m_size);
}

bool Entity::isSolid() const {
    return true;
}

} // namespace obby
