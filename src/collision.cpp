#include "collision.h"

bool RaycastAABB(Vector3 origin, Vector3 direction, float minDist, float maxDist, const AABB& box, float* outDist) {
    return box.raycast(Vec3(origin), Vec3(direction), minDist, maxDist, outDist);
}

bool RaycastScene(Vector3 origin, Vector3 direction, float minDist, float maxDist, const std::vector<AABB>& boxes, float* outDist) {
    bool hit = false;
    float closestHit = maxDist;
    Vec3 orig(origin);
    Vec3 dir(direction);

    for (const auto& box : boxes) {
        float d = 0.0f;
        if (box.raycast(orig, dir, minDist, closestHit, &d)) {
            hit = true;
            closestHit = d;
        }
    }

    if (hit && outDist) {
        *outDist = closestHit;
    }
    return hit;
}
