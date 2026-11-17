#include "aabb.h"
#include <cmath>
#include <algorithm>

namespace obby {

AABB::AABB()
    : m_min(0.0f, 0.0f, 0.0f), m_max(0.0f, 0.0f, 0.0f) {}

AABB::AABB(const Vec3& minPoint, const Vec3& maxPoint)
    : m_min(minPoint), m_max(maxPoint) {}

AABB::AABB(const Vector3& minPoint, const Vector3& maxPoint)
    : m_min(minPoint), m_max(maxPoint) {}

AABB AABB::fromCenterSize(const Vec3& center, const Vec3& size) {
    Vec3 half = size * 0.5f;
    return AABB{ center - half, center + half };
}

AABB AABB::fromMinSize(const Vec3& minPoint, const Vec3& size) {
    return AABB{ minPoint, minPoint + size };
}

AABB AABB::FromCenterSize(Vector3 center, Vector3 size) {
    return fromCenterSize(Vec3(center), Vec3(size));
}

AABB AABB::FromMinSize(Vector3 minPoint, Vector3 size) {
    return fromMinSize(Vec3(minPoint), Vec3(size));
}

Vec3 AABB::getCenter() const {
    return (m_min + m_max) * 0.5f;
}

Vec3 AABB::getExtents() const {
    return (m_max - m_min) * 0.5f;
}

Vec3 AABB::getSize() const {
    return m_max - m_min;
}

bool AABB::intersects(const AABB& other) const {
    return (m_min.x <= other.m_max.x && m_max.x >= other.m_min.x) &&
           (m_min.y <= other.m_max.y && m_max.y >= other.m_min.y) &&
           (m_min.z <= other.m_max.z && m_max.z >= other.m_min.z);
}

bool AABB::contains(const Vec3& point) const {
    return (point.x >= m_min.x && point.x <= m_max.x) &&
           (point.y >= m_min.y && point.y <= m_max.y) &&
           (point.z >= m_min.z && point.z <= m_max.z);
}

bool AABB::operator==(const AABB& other) const {
    return (m_min == other.m_min) && (m_max == other.m_max);
}

bool AABB::operator!=(const AABB& other) const {
    return !(*this == other);
}

bool AABB::raycast(const Vec3& origin, const Vec3& direction, float minDist, float maxDist, float* outDist) const {
    float tMin = minDist;
    float tMax = maxDist;

    // X axis
    if (std::abs(direction.x) < 1e-6f) {
        if (origin.x < m_min.x || origin.x > m_max.x) return false;
    } else {
        float invD = 1.0f / direction.x;
        float t1 = (m_min.x - origin.x) * invD;
        float t2 = (m_max.x - origin.x) * invD;
        if (t1 > t2) std::swap(t1, t2);
        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        if (tMin > tMax) return false;
    }

    // Y axis
    if (std::abs(direction.y) < 1e-6f) {
        if (origin.y < m_min.y || origin.y > m_max.y) return false;
    } else {
        float invD = 1.0f / direction.y;
        float t1 = (m_min.y - origin.y) * invD;
        float t2 = (m_max.y - origin.y) * invD;
        if (t1 > t2) std::swap(t1, t2);
        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        if (tMin > tMax) return false;
    }

    // Z axis
    if (std::abs(direction.z) < 1e-6f) {
        if (origin.z < m_min.z || origin.z > m_max.z) return false;
    } else {
        float invD = 1.0f / direction.z;
        float t1 = (m_min.z - origin.z) * invD;
        float t2 = (m_max.z - origin.z) * invD;
        if (t1 > t2) std::swap(t1, t2);
        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        if (tMin > tMax) return false;
    }

    if (tMin <= maxDist && tMin >= minDist) {
        if (outDist) *outDist = tMin;
        return true;
    }

    return false;
}

bool AABB::resolveAxis(Vec3& position, Vec3& velocity, float halfW, float halfD, float height, int axis, float skin) const {
    AABB playerBox{
        Vec3{ position.x - halfW, position.y, position.z - halfD },
        Vec3{ position.x + halfW, position.y + height, position.z + halfD }
    };

    if (axis == 0) { // X axis
        bool overlapY = (playerBox.m_min.y < m_max.y - skin) && (playerBox.m_max.y > m_min.y + skin);
        bool overlapZ = (playerBox.m_min.z < m_max.z - skin) && (playerBox.m_max.z > m_min.z + skin);
        if (overlapY && overlapZ) {
            if (playerBox.m_min.x < m_max.x && playerBox.m_max.x > m_min.x) {
                if (velocity.x > 0.0f) {
                    position.x = m_min.x - halfW;
                } else if (velocity.x < 0.0f) {
                    position.x = m_max.x + halfW;
                }
                velocity.x = 0.0f;
                return true;
            }
        }
    } else if (axis == 2) { // Z axis
        bool overlapX = (playerBox.m_min.x < m_max.x - skin) && (playerBox.m_max.x > m_min.x + skin);
        bool overlapY = (playerBox.m_min.y < m_max.y - skin) && (playerBox.m_max.y > m_min.y + skin);
        if (overlapX && overlapY) {
            if (playerBox.m_min.z < m_max.z && playerBox.m_max.z > m_min.z) {
                if (velocity.z > 0.0f) {
                    position.z = m_min.z - halfD;
                } else if (velocity.z < 0.0f) {
                    position.z = m_max.z + halfD;
                }
                velocity.z = 0.0f;
                return true;
            }
        }
    } else if (axis == 1) { // Y axis
        bool overlapX = (playerBox.m_min.x < m_max.x - skin) && (playerBox.m_max.x > m_min.x + skin);
        bool overlapZ = (playerBox.m_min.z < m_max.z - skin) && (playerBox.m_max.z > m_min.z + skin);
        if (overlapX && overlapZ) {
            if (playerBox.m_min.y <= m_max.y && playerBox.m_max.y > m_min.y) {
                if (velocity.y <= 0.0f) {
                    position.y = m_max.y;
                    velocity.y = 0.0f;
                    return true; // Landed
                } else if (playerBox.m_min.y < m_min.y && playerBox.m_max.y >= m_min.y) {
                    position.y = m_min.y - height;
                    velocity.y = 0.0f;
                    return true; // Hit ceiling
                }
            }
        }
    }
    return false;
}

} // namespace obby
