#pragma once
#include "raylib.h"
#include "entity.h"
#include "ghost_recorder.h"
#include "render_utils.h"
#include "button.h"
#include "door.h"
#include "checkpoint.h"
#include "finish_zone.h"
#include "particle.h"

#include <string>
#include <vector>
#include <memory>

struct Player;

namespace obby {

/**
 * @brief Represents a platforming course containing polymorphically managed entities.
 * Demonstrates:
 * - Ownership via std::vector<std::unique_ptr<Entity>>
 * - Polymorphic update() and draw() dispatch without type switches
 * - Factory method instantiation (EntityFactory)
 * - Exception throwing on corrupted level data (LevelParseError)
 */
class Level {
private:
    std::string m_name;
    int m_worldId;
    float m_killY;
    float m_parTime;
    Vec3 m_spawnPos;

    // Polymorphic entity container (Level is the unique owner)
    std::vector<std::unique_ptr<Entity>> m_entities;

    // Non-owning observer pointers for fast puzzle synchronization
    std::vector<Button*> m_buttons;
    std::vector<Door*> m_doors;
    std::vector<Checkpoint*> m_checkpoints;
    FinishZone* m_finishZone;

public:
    // Puzzle Ghost Recorder
    GhostRecorder ghost;

    // Runtime metrics
    float elapsedTime;
    int checkpointsActivated;
    int deaths;
    bool isCompleted;
    int starRating;

    // Compatibility fields for legacy access
    std::string name;
    int worldId;
    float killY;
    float parTime;
    Vector3 spawnPos;
    bool hasFinish;

    Level();
    ~Level() = default;

    // Move-only semantics (owns unique_ptrs)
    Level(const Level&) = delete;
    Level& operator=(const Level&) = delete;
    Level(Level&&) noexcept = default;
    Level& operator=(Level&&) noexcept = default;

    // Encapsulated Getters & Setters
    const std::string& getName() const { return m_name; }
    int getWorldId() const { return m_worldId; }
    float getKillY() const { return m_killY; }
    float getParTime() const { return m_parTime; }
    const Vec3& getSpawnPos() const { return m_spawnPos; }
    const std::vector<std::unique_ptr<Entity>>& getEntities() const { return m_entities; }

    Button* getButton(size_t index) const;
    Door* getDoor(size_t index) const;
    Checkpoint* getCheckpoint(size_t index) const;
    FinishZone* getFinishZone() const { return m_finishZone; }
    const std::vector<Button*>& getButtons() const { return m_buttons; }
    const std::vector<Door*>& getDoors() const { return m_doors; }
    const std::vector<Checkpoint*>& getCheckpoints() const { return m_checkpoints; }

    int getActivatedCheckpointCount() const;
    int getTotalCheckpointCount() const;
    int GetActivatedCheckpointCount() const { return getActivatedCheckpointCount(); }
    int GetTotalCheckpointCount() const { return getTotalCheckpointCount(); }

    // Core Level lifecycle
    bool LoadFromFile(const std::string& filepath);
    void Reset(Player& player);
    void Update(float dt, Player& player, ParticleSystem* particles = nullptr);
    void Draw() const;
    void DrawSkybox() const;

    // Polymorphic solid collider extraction
    std::vector<AABB> GetSolidColliders() const;
};

} // namespace obby

using Level = obby::Level;
