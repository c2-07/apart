#include "level.h"
#include "constants.h"
#include "audio.h"
#include "entity_factory.h"
#include "level_error.h"
#include "player.h"
#include "render_utils.h"
#include "raymath.h"
#include "rlgl.h"
#include "walker.h"
#include "bump_block.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

namespace obby {

Level::Level()
    : m_name("Untitled Level"),
      m_worldId(1),
      m_killY(-20.0f),
      m_parTime(60.0f),
      m_spawnPos(0.0f, 1.0f, 0.0f),
      m_finishZone(nullptr),
      elapsedTime(0.0f),
      checkpointsActivated(0),
      deaths(0),
      isCompleted(false),
      starRating(0),
      name("Untitled Level"),
      worldId(1),
      killY(-20.0f),
      parTime(60.0f),
      spawnPos{ 0.0f, 1.0f, 0.0f },
      hasFinish(false) {}

Button* Level::getButton(size_t index) const {
    if (index < m_buttons.size()) return m_buttons[index];
    return nullptr;
}

Door* Level::getDoor(size_t index) const {
    if (index < m_doors.size()) return m_doors[index];
    return nullptr;
}

Checkpoint* Level::getCheckpoint(size_t index) const {
    if (index < m_checkpoints.size()) return m_checkpoints[index];
    return nullptr;
}

int Level::getActivatedCheckpointCount() const {
    int count = 0;
    for (const auto* cp : m_checkpoints) {
        if (cp->isActivated()) ++count;
    }
    return count;
}

int Level::getTotalCheckpointCount() const {
    return static_cast<int>(m_checkpoints.size());
}

bool Level::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Level Parser] Could not open level file: " << filepath << std::endl;
        return false;
    }

    m_entities.clear();
    m_buttons.clear();
    m_doors.clear();
    m_checkpoints.clear();
    m_finishZone = nullptr;
    ghost.Reset();

    m_name = "Untitled";
    m_worldId = 1;
    m_killY = -20.0f;
    m_parTime = 60.0f;
    m_spawnPos = Vec3{ 0.0f, 1.0f, 0.0f };
    m_is2D = (filepath.find("2d") != std::string::npos || filepath.find("level_2d") != std::string::npos);
    hasFinish = false;
    isCompleted = false;
    elapsedTime = 0.0f;
    deaths = 0;
    starRating = 0;

    std::string line;
    int lineNum = 0;

    try {
        while (std::getline(file, line)) {
            lineNum++;
            size_t first = line.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) continue;
            if (line[first] == '#') continue;

            std::istringstream ss(line.substr(first));
            std::string token;
            ss >> token;

            if (token == "name") {
                std::string rem;
                std::getline(ss, rem);
                size_t remFirst = rem.find_first_not_of(" \t\r\n");
                if (remFirst != std::string::npos) m_name = rem.substr(remFirst);
            } else if (token == "world") {
                if (!(ss >> m_worldId)) throw LevelParseError("Invalid world ID format");
            } else if (token == "killY") {
                if (!(ss >> m_killY)) throw LevelParseError("Invalid killY format");
            } else if (token == "par") {
                if (!(ss >> m_parTime)) throw LevelParseError("Invalid par format");
            } else if (token == "spawn") {
                float x, y, z;
                if (ss >> x >> y >> z) {
                    m_spawnPos = Vec3{ x, y, z };
                } else {
                    throw LevelParseError("Invalid spawn coordinates");
                }
            } else if (token == "mode") {
                std::string modeStr;
                if (ss >> modeStr) {
                    m_is2D = (modeStr == "2d" || modeStr == "2D");
                }
            } else {
                // Factory instantiation
                std::unique_ptr<Entity> ent = EntityFactory::createEntity(token, ss, m_worldId);
                if (ent) {
                    // Populate observer pointers for puzzle linking
                    if (auto* btn = dynamic_cast<Button*>(ent.get())) {
                        m_buttons.push_back(btn);
                    } else if (auto* dr = dynamic_cast<Door*>(ent.get())) {
                        m_doors.push_back(dr);
                    } else if (auto* cp = dynamic_cast<Checkpoint*>(ent.get())) {
                        m_checkpoints.push_back(cp);
                    } else if (auto* fz = dynamic_cast<FinishZone*>(ent.get())) {
                        m_finishZone = fz;
                        hasFinish = true;
                    }
                    m_entities.push_back(std::move(ent));
                }
            }
        }
    } catch (const LevelParseError& err) {
        std::cerr << "[Level Parser Exception] " << filepath << ":" << lineNum << ": " << err.what() << std::endl;
        return false;
    }

    // Sync compatibility fields
    name = m_name;
    worldId = m_worldId;
    killY = m_killY;
    parTime = m_parTime;
    spawnPos = m_spawnPos.toRaylib();

    std::cout << "[Level Parser] Loaded '" << m_name << "' with " << m_entities.size()
              << " entities (" << m_buttons.size() << " buttons, " << m_doors.size() << " doors)." << std::endl;
    return true;
}

void Level::Reset(Player& player) {
    elapsedTime = 0.0f;
    deaths = 0;
    isCompleted = false;
    starRating = 0;
    ghost.Reset();

    for (auto* cp : m_checkpoints) {
        cp->setActivated(false);
    }
    for (auto* btn : m_buttons) {
        btn->setPressed(false);
        btn->setActive(false);
    }
    for (auto* dr : m_doors) {
        dr->setOpen(false);
    }
    if (m_finishZone) {
        m_finishZone->setCompleted(false);
    }

    player.init(m_spawnPos);
    player.set2DMode(m_is2D, m_spawnPos.z);
}

void Level::Update(float dt, Player& player, ParticleSystem* particles) {
    if (!isCompleted) {
        elapsedTime += dt;
    }

    // Ghost recorder tick
    if (ghost.state == GhostRecorder::State::RECORDING) {
        ghost.RecordTick(player.getPosition().toRaylib(), player.getModelYaw(), player.isGrounded());
    } else if (ghost.state == GhostRecorder::State::PLAYING) {
        ghost.UpdatePlayback();
    }

    // Check ghost standing on buttons
    bool hasGhost = ghost.HasActiveGhost();
    AABB ghostAABB = hasGhost ? ghost.GetAABB() : AABB();
    for (auto* btn : m_buttons) {
        bool ghostTouching = hasGhost && btn->getTriggerBounds().intersects(ghostAABB);
        btn->setGhostTouching(ghostTouching);
    }

    // Expose solid colliders for autonomous enemy navigation
    std::vector<AABB> levelSolids = GetSolidColliders();
    Walker::setLevelColliders(&levelSolids);
    BumpBlock::setParticleSystem(particles);

    // Polymorphic update loop across all entities (no type switches!)
    for (const auto& entity : m_entities) {
        entity->update(dt, player);
    }

    // Safe deferred removal of inactive/defeated entities via std::remove_if
    m_entities.erase(
        std::remove_if(m_entities.begin(), m_entities.end(),
            [](const std::unique_ptr<Entity>& e) {
                return e->isExpired();
            }),
        m_entities.end());

    // Synchronize doors with active buttons
    for (auto* dr : m_doors) {
        bool shouldOpen = false;
        for (const auto* btn : m_buttons) {
            if (btn->getId() == dr->getId() && btn->isActive()) {
                shouldOpen = true;
                break;
            }
        }
        dr->setOpen(shouldOpen);
    }

    // Check finish zone completion
    if (m_finishZone && m_finishZone->isCompleted() && !isCompleted) {
        isCompleted = true;
        if (particles) {
            particles->SpawnConfettiBurst(m_finishZone->getPosition().toRaylib());
        }
        if (elapsedTime <= m_parTime && deaths == 0) {
            starRating = 3;
        } else if (elapsedTime <= m_parTime * 1.4f) {
            starRating = 2;
        } else {
            starRating = 1;
        }
    }

    // Check kill plane
    if (player.getPosition().y < m_killY) {
        deaths++;
        player.setDeathCount(deaths);
        player.respawn();
        AudioManager::Play(SoundId::DEATH);
    }
}

void Level::DrawSkybox() const {
    ClearBackground(BLACK);
    WorldPalette pal = GetWorldPalette(m_worldId);
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    DrawRectangleGradientV(0, 0, sw, sh, pal.skyTop, pal.skyBottom);
}

void Level::Draw() const {
    WorldPalette pal = GetWorldPalette(m_worldId);

    // 1. Opaque entities pass with normal depth testing and depth write
    for (const auto& entity : m_entities) {
        entity->draw();
    }

    // 2. Transparent elements with depth write disabled to prevent occluding geometry
    rlDisableDepthMask();

    // Atmospheric ground-far haze below course void
    float hazeY = m_killY - 2.0f;
    Color hazeColor = Color{ pal.fogColor.r, pal.fogColor.g, pal.fogColor.b, 95 };
    DrawCube(Vector3{ 0.0f, hazeY, 0.0f }, 360.0f, 3.0f, 360.0f, hazeColor);

    // Drifting cube clouds
    float time = static_cast<float>(GetTime());
    for (int i = 0; i < 9; ++i) {
        float cx = fmodf(i * 42.0f + time * 1.2f + 150.0f, 320.0f) - 160.0f;
        float cy = 48.0f + sinf(i * 1.5f) * 5.0f;
        float cz = -90.0f + i * 24.0f;
        Vector3 cPos = { cx, cy, cz };
        DrawCube(cPos, 22.0f, 4.5f, 12.0f, Color{ 255, 255, 255, 175 });
        DrawCubeWires(cPos, 22.04f, 4.54f, 12.04f, Color{ pal.fogColor.r, pal.fogColor.g, pal.fogColor.b, 160 });
    }

    // Draw ghost replay if active
    ghost.Draw();

    rlEnableDepthMask();
}

std::vector<AABB> Level::GetSolidColliders() const {
    std::vector<AABB> solids;
    solids.reserve(m_entities.size());

    // Polymorphic solid extraction
    for (const auto& entity : m_entities) {
        if (entity->isSolid()) {
            solids.push_back(entity->getBounds());
        }
    }
    return solids;
}

} // namespace obby
