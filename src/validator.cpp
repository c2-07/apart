#include "validator.h"
#include "level.h"
#include "constants.h"
#include "raymath.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <queue>
#include <algorithm>

struct PlatformNode {
    AABB aabb;
    bool isBounce;
    float bounceStrength;
    Vector3 moverDelta;
    bool isTrigger;
};

static float ComputeMaxJumpDistance(float deltaY, float launchVelY) {
    constexpr float g = -Constants::GRAVITY; // 30 m/s^2
    float v0 = launchVelY;

    float discriminant = v0 * v0 - 2.0f * g * deltaY;
    if (discriminant < 0.0f) {
        return 0.0f; // Target ledge is higher than peak jump apex
    }

    float tAir = (v0 + sqrtf(discriminant)) / g;
    return Constants::RUN_SPEED * tAir;
}

ValidationResult LevelValidator::ValidateFile(const std::string& filepath) {
    ValidationResult res;
    res.filename = filepath;
    res.passed = false;
    res.maxObservedGap = 0.0f;
    res.maxAllowedGap = 6.13f;

    Level level;
    if (!level.LoadFromFile(filepath)) {
        std::cerr << "[Validator] Failed to load " << filepath << std::endl;
        return res;
    }

    res.levelName = level.name;
    res.worldId = level.worldId;
    res.boxCount = static_cast<int>(level.boxes.size());
    res.moverCount = static_cast<int>(level.movers.size());
    res.hazardCount = static_cast<int>(level.spinners.size() + level.spikesList.size());
    res.hasFinish = level.hasFinish;

    if (!level.hasFinish) {
        std::cerr << "[Validator] " << filepath << " has no finish zone!" << std::endl;
        return res;
    }

    std::vector<PlatformNode> nodes;

    // Node 0: Spawn platform
    nodes.push_back({
        AABB::FromCenterSize(Vector3{ level.spawnPos.x, level.spawnPos.y - 0.5f, level.spawnPos.z }, Vector3{ 6.0f, 1.0f, 6.0f }),
        false, 0.0f, Vector3{ 0.0f, 0.0f, 0.0f }, false
    });

    for (const auto& b : level.boxes) {
        nodes.push_back({ b.aabb, false, 0.0f, Vector3{ 0.0f, 0.0f, 0.0f }, false });
    }
    for (const auto& cp : level.checkpoints) {
        nodes.push_back({
            AABB::FromCenterSize(Vector3{ cp.position.x, cp.position.y - 0.25f, cp.position.z }, Vector3{ 3.0f, 0.5f, 3.0f }),
            false, 0.0f, Vector3{ 0.0f, 0.0f, 0.0f }, false
        });
    }
    for (const auto& m : level.movers) {
        nodes.push_back({ m.GetAABB(), false, 0.0f, m.delta, false });
    }
    for (const auto& c : level.crumbleTiles) {
        nodes.push_back({ c.GetAABB(), false, 0.0f, Vector3{ 0.0f, 0.0f, 0.0f }, false });
    }
    for (const auto& bp : level.bouncePads) {
        nodes.push_back({ bp.aabb, true, bp.strength, Vector3{ 0.0f, 0.0f, 0.0f }, false });
    }
    for (const auto& btn : level.buttons) {
        nodes.push_back({ btn.baseAABB, false, 0.0f, Vector3{ 0.0f, 0.0f, 0.0f }, false });
    }

    int finishNodeIndex = static_cast<int>(nodes.size());
    nodes.push_back({ level.finish.triggerAABB, false, 0.0f, Vector3{ 0.0f, 0.0f, 0.0f }, true });

    size_t numNodes = nodes.size();
    std::vector<bool> visited(numNodes, false);
    std::queue<int> queue;

    queue.push(0);
    visited[0] = true;

    float maxGapFound = 0.0f;

    while (!queue.empty()) {
        int u = queue.front();
        queue.pop();

        if (u == finishNodeIndex) {
            res.passed = true;
            break;
        }

        const auto& nodeA = nodes[u];

        for (size_t v = 0; v < numNodes; ++v) {
            if (visited[v]) continue;

            const auto& nodeB = nodes[v];

            // Target landing height
            float targetY = nodeB.isTrigger ? nodeB.aabb.min.y : nodeB.aabb.max.y;
            float deltaY = targetY - nodeA.aabb.max.y;

            // Launch velocity from node A
            float launchVel = nodeA.isBounce ? nodeA.bounceStrength : Constants::JUMP_VELOCITY;

            // Compute maximum horizontal jump distance possible
            float maxAllowedDist = 0.0f;
            if (nodeB.isTrigger && nodeB.aabb.min.y <= nodeA.aabb.max.y && nodeB.aabb.max.y >= nodeA.aabb.max.y) {
                // If trigger overlaps platform height directly
                maxAllowedDist = 8.0f;
            } else {
                maxAllowedDist = ComputeMaxJumpDistance(deltaY, launchVel);
            }

            // Expand reach by mover displacements if applicable
            float moverExpansion = Vector3Length(nodeA.moverDelta) + Vector3Length(nodeB.moverDelta);
            maxAllowedDist += moverExpansion;

            // Compute horizontal gap between AABBs in XZ
            float dx = std::max(0.0f, std::max(nodeA.aabb.min.x - nodeB.aabb.max.x, nodeB.aabb.min.x - nodeA.aabb.max.x));
            float dz = std::max(0.0f, std::max(nodeA.aabb.min.z - nodeB.aabb.max.z, nodeB.aabb.min.z - nodeA.aabb.max.z));
            float gapXZ = sqrtf(dx * dx + dz * dz);

            if (gapXZ <= maxAllowedDist && maxAllowedDist > 0.0f) {
                visited[v] = true;
                queue.push(static_cast<int>(v));
                if (gapXZ > maxGapFound && !nodeA.isBounce && Vector3Length(nodeA.moverDelta) < 0.1f) {
                    maxGapFound = gapXZ;
                }
            }
        }
    }

    res.maxObservedGap = maxGapFound;
    return res;
}

bool LevelValidator::ValidateAllLevels(const std::vector<std::string>& files) {
    std::cout << "\n================================================================================" << std::endl;
    std::cout << "                 LEVEL VALIDATOR: 20-LEVEL REACHABILITY SUITE                  " << std::endl;
    std::cout << "================================================================================" << std::endl;
    std::cout << std::left
              << std::setw(12) << "File"
              << std::setw(28) << "Level Name"
              << std::setw(8)  << "World"
              << std::setw(8)  << "Boxes"
              << std::setw(8)  << "Movers"
              << std::setw(10) << "Max Gap"
              << std::setw(8)  << "Status" << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;

    bool allPassed = true;
    for (const auto& file : files) {
        ValidationResult r = ValidateFile(file);
        std::cout << std::left
                  << std::setw(12) << r.filename.substr(r.filename.find_last_of("/\\") + 1)
                  << std::setw(28) << r.levelName
                  << std::setw(8)  << r.worldId
                  << std::setw(8)  << r.boxCount
                  << std::setw(8)  << r.moverCount
                  << std::setw(10) << (std::to_string(std::round(r.maxObservedGap * 10.0f) / 10.0f).substr(0, 4) + "m")
                  << std::setw(8)  << (r.passed ? "PASS" : "FAIL") << std::endl;

        if (!r.passed) allPassed = false;
    }
    std::cout << "================================================================================\n" << std::endl;
    return allPassed;
}
