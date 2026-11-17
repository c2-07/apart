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

// Milestone 6: Solo Cooperative Puzzles
struct ButtonEntity {
    int id;
    Vector3 position;
    float duration;
    float currentTimer;
    bool isPressed;
    bool isActive;
    AABB triggerAABB;
    AABB baseAABB;
};

struct DoorEntity {
    int id;
    Vector3 center;
    Vector3 size;
    AABB aabb;
    bool isOpen;
};

struct GhostFrame {
    Vector3 position;
    float yaw;
    bool isGrounded;
};

class GhostRecorder {
public:
    enum class State {
        IDLE,
        RECORDING,
        PLAYING
    };

    State state;
    std::vector<GhostFrame> frames;
    size_t playbackIndex;
    float recordTimer;
    static constexpr float MAX_RECORD_TIME = 20.0f;

    GhostRecorder();
    void StartRecording();
    void StopRecording();
    void StartPlayback();
    void Reset();

    void RecordTick(Vector3 pos, float yaw, bool grounded);
    void UpdatePlayback();

    Vector3 GetCurrentPos() const;
    float GetCurrentYaw() const;
    bool HasActiveGhost() const;
    AABB GetAABB() const;
    void Draw() const;
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

    std::vector<Mover> movers;
    std::vector<Spinner> spinners;
    std::vector<CrumbleTile> crumbleTiles;
    std::vector<Spikes> spikesList;
    std::vector<BouncePad> bouncePads;

    // Milestone 6: Buttons and Doors
    std::vector<ButtonEntity> buttons;
    std::vector<DoorEntity> doors;
    GhostRecorder ghost;

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
