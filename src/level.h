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

struct Mover {
    Vector3 basePos;
    Vector3 size;
    Vector3 delta;
    float period;
    int colorIndex;

    Vector3 currentPos;
    Vector3 frameDisplacement;
    AABB GetAABB() const;
};

struct Spinner {
    Vector3 center;
    float length;
    float height;
    float speed;
    float currentAngle;
    bool CheckOverlapWithPlayer(const AABB& playerAABB) const;
};

enum class CrumbleState {
    STABLE,
    CRUMBLING,
    FALLEN
};

struct CrumbleTile {
    Vector3 basePos;
    Vector3 size;
    float delay;
    float respawnTime;

    CrumbleState state;
    float timer;
    Vector3 currentPos;
    AABB GetAABB() const;
    bool IsSolid() const;
};

struct Spikes {
    Vector3 center;
    Vector3 size;
    AABB aabb;
};

struct BouncePad {
    Vector3 center;
    Vector3 size;
    float strength;
    AABB aabb;
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

    // Milestone 4: Moving entities and hazards
    std::vector<Mover> movers;
    std::vector<Spinner> spinners;
    std::vector<CrumbleTile> crumbleTiles;
    std::vector<Spikes> spikesList;
    std::vector<BouncePad> bouncePads;

    // Runtime state
    float elapsedTime;
    int checkpointsActivated;
    int deaths;
    bool isCompleted;
    int starRating;

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
