#pragma once
#include "raylib.h"
#include <vector>

struct AABB {
    Vector3 min;
    Vector3 max;

    Vector3 GetCenter() const;
    Vector3 GetExtents() const;
    Vector3 GetSize() const;
    static AABB FromCenterSize(Vector3 center, Vector3 size);
    static AABB FromMinSize(Vector3 min, Vector3 size);
};

// Check if two AABBs intersect
bool CheckAABBOverlap(const AABB& a, const AABB& b);

// Ray-AABB intersection using slab method.
// Returns true if ray intersects box within [minDist, maxDist], and sets outDist.
bool RaycastAABB(Vector3 origin, Vector3 direction, float minDist, float maxDist, const AABB& box, float* outDist = nullptr);

// Query all obstacles and find the closest hit distance
bool RaycastScene(Vector3 origin, Vector3 direction, float minDist, float maxDist, const std::vector<AABB>& boxes, float* outDist = nullptr);
