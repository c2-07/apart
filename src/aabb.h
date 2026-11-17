#pragma once
#include "vec3.h"
#include <vector>

namespace obby {

/**
 * @brief Axis-Aligned Bounding Box (AABB) value class.
 * Demonstrates encapsulation (private data members, public accessors),
 * constructors with member initializer lists, operator overloading (operator==, !=),
 * and spatial collision resolution.
 */
class AABB {
private:
    Vec3 m_min;
    Vec3 m_max;

public:
    // Constructors
    AABB();
    AABB(const Vec3& minPoint, const Vec3& maxPoint);
    AABB(const Vector3& minPoint, const Vector3& maxPoint);

    // Named factory constructors
    static AABB fromCenterSize(const Vec3& center, const Vec3& size);
    static AABB fromMinSize(const Vec3& minPoint, const Vec3& size);

    // Compatibility static factories
    static AABB FromCenterSize(Vector3 center, Vector3 size);
    static AABB FromMinSize(Vector3 minPoint, Vector3 size);

    // Encapsulated Getters & Setters
    const Vec3& getMin() const { return m_min; }
    const Vec3& getMax() const { return m_max; }
    void setMin(const Vec3& minPoint) { m_min = minPoint; }
    void setMax(const Vec3& maxPoint) { m_max = maxPoint; }

    // Derived spatial metrics
    Vec3 getCenter() const;
    Vec3 getExtents() const;
    Vec3 getSize() const;

    // PascalCase aliases for backward compatibility
    Vec3 GetCenter() const { return getCenter(); }
    Vec3 GetExtents() const { return getExtents(); }
    Vec3 GetSize() const { return getSize(); }

    // Collision queries & operator overloading
    bool intersects(const AABB& other) const;
    bool contains(const Vec3& point) const;

    bool operator==(const AABB& other) const;
    bool operator!=(const AABB& other) const;

    // Raycast intersection (Kay-Kajiya slab method)
    bool raycast(const Vec3& origin, const Vec3& direction, float minDist, float maxDist, float* outDist = nullptr) const;

    // Per-axis collision resolution method
    bool resolveAxis(Vec3& position, Vec3& velocity, float halfW, float halfD, float height, int axis, float skin = 0.01f) const;
};

} // namespace obby

using AABB = obby::AABB;
