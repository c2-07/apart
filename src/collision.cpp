#include "collision.h"
#include <cmath>
#include <algorithm>

Vector3 AABB::GetCenter() const {
    return Vector3{
        (min.x + max.x) * 0.5f,
        (min.y + max.y) * 0.5f,
        (min.z + max.z) * 0.5f
    };
}

Vector3 AABB::GetExtents() const {
    return Vector3{
        (max.x - min.x) * 0.5f,
        (max.y - min.y) * 0.5f,
        (max.z - min.z) * 0.5f
    };
}

Vector3 AABB::GetSize() const {
    return Vector3{
        max.x - min.x,
        max.y - min.y,
        max.z - min.z
    };
}

AABB AABB::FromCenterSize(Vector3 center, Vector3 size) {
    Vector3 half{ size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
    return AABB{
        Vector3{ center.x - half.x, center.y - half.y, center.z - half.z },
        Vector3{ center.x + half.x, center.y + half.y, center.z + half.z }
    };
}

AABB AABB::FromMinSize(Vector3 min, Vector3 size) {
    return AABB{
        min,
        Vector3{ min.x + size.x, min.y + size.y, min.z + size.z }
    };
}

bool CheckAABBOverlap(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

bool RaycastAABB(Vector3 origin, Vector3 direction, float minDist, float maxDist, const AABB& box, float* outDist) {
    float tMin = minDist;
    float tMax = maxDist;

    // X axis
    if (std::abs(direction.x) < 1e-6f) {
        if (origin.x < box.min.x || origin.x > box.max.x) return false;
    } else {
        float invD = 1.0f / direction.x;
        float t1 = (box.min.x - origin.x) * invD;
        float t2 = (box.max.x - origin.x) * invD;
        if (t1 > t2) std::swap(t1, t2);
        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        if (tMin > tMax) return false;
    }

    // Y axis
    if (std::abs(direction.y) < 1e-6f) {
        if (origin.y < box.min.y || origin.y > box.max.y) return false;
    } else {
        float invD = 1.0f / direction.y;
        float t1 = (box.min.y - origin.y) * invD;
        float t2 = (box.max.y - origin.y) * invD;
        if (t1 > t2) std::swap(t1, t2);
        tMin = std::max(tMin, t1);
        tMax = std::min(tMax, t2);
        if (tMin > tMax) return false;
    }

    // Z axis
    if (std::abs(direction.z) < 1e-6f) {
        if (origin.z < box.min.z || origin.z > box.max.z) return false;
    } else {
        float invD = 1.0f / direction.z;
        float t1 = (box.min.z - origin.z) * invD;
        float t2 = (box.max.z - origin.z) * invD;
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

bool RaycastScene(Vector3 origin, Vector3 direction, float minDist, float maxDist, const std::vector<AABB>& boxes, float* outDist) {
    float closest = maxDist;
    bool hit = false;
    for (const auto& box : boxes) {
        float dist = 0.0f;
        if (RaycastAABB(origin, direction, minDist, closest, box, &dist)) {
            if (dist < closest) {
                closest = dist;
                hit = true;
            }
        }
    }
    if (hit && outDist) {
        *outDist = closest;
    }
    return hit;
}
