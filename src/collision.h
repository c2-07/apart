#pragma once
#include "aabb.h"
#include "vec3.h"
#include <vector>

// Check if two AABBs intersect (delegates to AABB::intersects)
inline bool CheckAABBOverlap(const AABB& a, const AABB& b) {
    return a.intersects(b);
}

// Ray-AABB intersection using slab method (delegates to AABB::raycast)
bool RaycastAABB(Vector3 origin, Vector3 direction, float minDist, float maxDist, const AABB& box, float* outDist = nullptr);

// Query all obstacles and find the closest hit distance
bool RaycastScene(Vector3 origin, Vector3 direction, float minDist, float maxDist, const std::vector<AABB>& boxes, float* outDist = nullptr);
