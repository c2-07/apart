#pragma once
#include "raylib.h"
#include "collision.h"
#include "player.h"

#include <string>
#include <vector>

struct LevelBox {
    AABB aabb;
    int colorIndex;
};

struct Checkpoint {
    Vector3 position;
    AABB triggerAABB;
    bool activated;
};

struct FinishZone {
    AABB triggerAABB;
};

struct WorldPalette {
    Color skyTop;
    Color skyBottom;
    Color colors[4];
    Color hazard;
    Color accent;
};

WorldPalette GetWorldPalette(int worldId);

class Level {
public:
    std::string name;
    int worldId;
    float killY;
    float parTime;
    Vector3 spawnPos;

    std::vector<LevelBox> boxes;
    std::vector<Checkpoint> checkpoints;
    FinishZone finish;
    bool hasFinish;

    // Runtime state
    float elapsedTime;
    int checkpointsActivated;
    int deaths;
    bool isCompleted;
    int starRating; // 1, 2, or 3

    Level();
    bool LoadFromFile(const std::string& filepath);
    void Reset(Player& player);
    void Update(float dt, Player& player);
    void Draw() const;
    void DrawSkybox() const;

    std::vector<AABB> GetSolidColliders() const;
    int GetActivatedCheckpointCount() const;
    int GetTotalCheckpointCount() const;
};
