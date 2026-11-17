#include "level.h"
#include "constants.h"
#include "raymath.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

static Color DarkenColor(Color c, float factor) {
    return Color{
        static_cast<unsigned char>(c.r * factor),
        static_cast<unsigned char>(c.g * factor),
        static_cast<unsigned char>(c.b * factor),
        c.a
    };
}

WorldPalette GetWorldPalette(int worldId) {
    WorldPalette pal;
    switch (worldId) {
        case 2: // Ocean
            pal.skyTop    = Color{ 60, 140, 220, 255 };
            pal.skyBottom = Color{ 160, 220, 245, 255 };
            pal.colors[0] = Color{ 40, 160, 200, 255 };  // Turquoise water/sand
            pal.colors[1] = Color{ 235, 190, 120, 255 }; // Beach sand
            pal.colors[2] = Color{ 70, 120, 180, 255 };  // Deep sea blue
            pal.colors[3] = Color{ 250, 215, 90, 255 };  // Coral yellow
            pal.hazard    = Color{ 230, 45, 80, 255 };   // Sea urchin pink/red
            pal.accent    = Color{ 50, 230, 190, 255 };  // Seafoam cyan
            break;
        case 3: // Winter
            pal.skyTop    = Color{ 110, 145, 185, 255 };
            pal.skyBottom = Color{ 215, 230, 245, 255 };
            pal.colors[0] = Color{ 235, 245, 255, 255 }; // Pure snow white
            pal.colors[1] = Color{ 145, 195, 225, 255 }; // Ice blue
            pal.colors[2] = Color{ 110, 130, 155, 255 }; // Frost rock
            pal.colors[3] = Color{ 190, 220, 235, 255 }; // Pale glacier
            pal.hazard    = Color{ 220, 60, 95, 255 };   // Cryo spike
            pal.accent    = Color{ 90, 220, 255, 255 };  // Aurora cyan
            break;
        case 4: // Factory
            pal.skyTop    = Color{ 70, 75, 85, 255 };
            pal.skyBottom = Color{ 140, 135, 130, 255 };
            pal.colors[0] = Color{ 180, 100, 40, 255 };  // Industrial rust
            pal.colors[1] = Color{ 90, 95, 105, 255 };   // Steel gray
            pal.colors[2] = Color{ 230, 160, 30, 255 };  // Caution yellow
            pal.colors[3] = Color{ 60, 65, 75, 255 };    // Dark iron
            pal.hazard    = Color{ 245, 50, 40, 255 };   // Molten hazard red
            pal.accent    = Color{ 70, 230, 120, 255 };  // Neon green
            break;
        case 1: // Meadow (Default)
        default:
            pal.skyTop    = Color{ 120, 190, 245, 255 };
            pal.skyBottom = Color{ 200, 235, 255, 255 };
            pal.colors[0] = Color{ 90, 185, 75, 255 };   // Vibrant grass green
            pal.colors[1] = Color{ 185, 130, 80, 255 };  // Wood brown
            pal.colors[2] = Color{ 160, 175, 180, 255 }; // Clean stone gray
            pal.colors[3] = Color{ 245, 205, 60, 255 };  // Meadow sunflower
            pal.hazard    = Color{ 235, 65, 50, 255 };   // Danger red
            pal.accent    = Color{ 50, 220, 140, 255 };  // Emerald green
            break;
    }
    return pal;
}

Level::Level()
    : name("Untitled Level"),
      worldId(1),
      killY(-20.0f),
      parTime(60.0f),
      spawnPos{ 0.0f, 1.0f, 0.0f },
      hasFinish(false),
      elapsedTime(0.0f),
      checkpointsActivated(0),
      deaths(0),
      isCompleted(false),
      starRating(0) {}

bool Level::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Level Parser] Could not open level file: " << filepath << std::endl;
        return false;
    }

    boxes.clear();
    checkpoints.clear();
    hasFinish = false;
    spawnPos = Vector3{ 0.0f, 1.0f, 0.0f };
    name = "Untitled";
    worldId = 1;
    killY = -20.0f;
    parTime = 60.0f;

    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;
        // Trim leading whitespace
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue; // Empty line
        if (line[first] == '#') continue;        // Comment line

        std::stringstream ss(line.substr(first));
        std::string token;
        ss >> token;

        if (token == "name") {
            // Remainder of line is level name
            std::string rem;
            std::getline(ss, rem);
            size_t remFirst = rem.find_first_not_of(" \t\r\n");
            if (remFirst != std::string::npos) {
                name = rem.substr(remFirst);
            }
        } else if (token == "world") {
            if (!(ss >> worldId)) {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid world id\n";
            }
        } else if (token == "killY") {
            if (!(ss >> killY)) {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid killY\n";
            }
        } else if (token == "par") {
            if (!(ss >> parTime)) {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid par time\n";
            }
        } else if (token == "spawn") {
            float x, y, z;
            if (ss >> x >> y >> z) {
                spawnPos = Vector3{ x, y, z };
            } else {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid spawn\n";
            }
        } else if (token == "box") {
            float x, y, z, sx, sy, sz;
            int cIndex = 0;
            if (ss >> x >> y >> z >> sx >> sy >> sz) {
                if (!(ss >> cIndex)) cIndex = 0;
                LevelBox b;
                b.aabb = AABB::FromCenterSize(Vector3{ x, y, z }, Vector3{ sx, sy, sz });
                b.colorIndex = std::clamp(cIndex, 0, 3);
                boxes.push_back(b);
            } else {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid box\n";
            }
        } else if (token == "checkpoint") {
            float x, y, z;
            if (ss >> x >> y >> z) {
                Checkpoint cp;
                cp.position = Vector3{ x, y, z };
                // Trigger region around checkpoint platform
                cp.triggerAABB = AABB::FromCenterSize(Vector3{ x, y + 1.2f, z }, Vector3{ 3.0f, 2.6f, 3.0f });
                cp.activated = false;
                checkpoints.push_back(cp);
            } else {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid checkpoint\n";
            }
        } else if (token == "finish") {
            float x, y, z, sx, sy, sz;
            if (ss >> x >> y >> z >> sx >> sy >> sz) {
                finish.triggerAABB = AABB::FromCenterSize(Vector3{ x, y, z }, Vector3{ sx, sy, sz });
                hasFinish = true;
            } else {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid finish\n";
            }
        } else {
            // Forward-compatibility placeholder for mover, spinner, crumble, spikes, button, door, bounce
            // Simply skip unsupported tokens in M3 gracefully
        }
    }

    std::cout << "[Level Parser] Loaded '" << name << "' (World " << worldId
              << "): " << boxes.size() << " boxes, "
              << checkpoints.size() << " checkpoints, finish: "
              << (hasFinish ? "YES" : "NO") << std::endl;
    return true;
}

void Level::Reset(Player& player) {
    elapsedTime = 0.0f;
    checkpointsActivated = 0;
    deaths = 0;
    isCompleted = false;
    starRating = 0;

    for (auto& cp : checkpoints) {
        cp.activated = false;
    }

    player.Init(spawnPos);
}

void Level::Update(float dt, Player& player) {
    if (!isCompleted) {
        elapsedTime += dt;
    }

    // Check kill plane
    if (player.position.y < killY) {
        deaths++;
        player.deathCount = deaths;
        player.Respawn();
    }

    // Check checkpoints
    AABB playerBox = player.GetAABB();
    for (auto& cp : checkpoints) {
        if (!cp.activated && CheckAABBOverlap(playerBox, cp.triggerAABB)) {
            cp.activated = true;
            player.spawnPoint = cp.position;
            checkpointsActivated++;
        }
    }

    // Check finish trigger
    if (hasFinish && !isCompleted) {
        if (CheckAABBOverlap(playerBox, finish.triggerAABB)) {
            isCompleted = true;
            if (elapsedTime <= parTime && deaths == 0) {
                starRating = 3;
            } else if (elapsedTime <= parTime * 1.4f) {
                starRating = 2;
            } else {
                starRating = 1;
            }
        }
    }
}

std::vector<AABB> Level::GetSolidColliders() const {
    std::vector<AABB> solids;
    solids.reserve(boxes.size() + checkpoints.size());
    for (const auto& b : boxes) {
        solids.push_back(b.aabb);
    }
    // Also add checkpoint base pads as solid stepping platforms
    for (const auto& cp : checkpoints) {
        solids.push_back(AABB::FromCenterSize(Vector3{ cp.position.x, cp.position.y - 0.25f, cp.position.z }, Vector3{ 3.0f, 0.5f, 3.0f }));
    }
    return solids;
}

void Level::DrawSkybox() const {
    WorldPalette pal = GetWorldPalette(worldId);
    ClearBackground(pal.skyTop);

    // Drifting cube clouds in the sky
    float time = static_cast<float>(GetTime());
    for (int i = 0; i < 8; ++i) {
        float cx = fmodf(i * 35.0f + time * 1.5f + 100.0f, 240.0f) - 120.0f;
        float cy = 35.0f + sinf(i * 1.3f) * 4.0f;
        float cz = -80.0f + i * 25.0f;
        Vector3 cPos = { cx, cy, cz };
        DrawCube(cPos, 18.0f, 4.0f, 10.0f, Color{ 255, 255, 255, 190 });
        DrawCubeWires(cPos, 18.05f, 4.05f, 10.05f, Color{ 220, 235, 245, 190 });
    }
}

void Level::Draw() const {
    WorldPalette pal = GetWorldPalette(worldId);

    // 1. Draw static level boxes
    for (const auto& b : boxes) {
        Vector3 center = b.aabb.GetCenter();
        Vector3 size = b.aabb.GetSize();
        Color faceColor = pal.colors[b.colorIndex];
        Color wireColor = DarkenColor(faceColor, 0.65f);

        DrawCube(center, size.x, size.y, size.z, faceColor);
        DrawCubeWires(center, size.x + 0.02f, size.y + 0.02f, size.z + 0.02f, wireColor);
    }

    // 2. Draw checkpoints
    for (const auto& cp : checkpoints) {
        // Base pad
        Vector3 padCenter = { cp.position.x, cp.position.y - 0.25f, cp.position.z };
        Color padColor = cp.activated ? Color{ 80, 220, 100, 255 } : Color{ 180, 180, 190, 255 };
        DrawCube(padCenter, 3.0f, 0.5f, 3.0f, padColor);
        DrawCubeWires(padCenter, 3.02f, 0.52f, 3.02f, DarkenColor(padColor, 0.65f));

        // Flagpole
        Vector3 poleCenter = { cp.position.x - 1.0f, cp.position.y + 1.25f, cp.position.z - 1.0f };
        DrawCube(poleCenter, 0.15f, 2.5f, 0.15f, Color{ 70, 70, 80, 255 });

        // Flag / Beacon
        Vector3 flagCenter = { cp.position.x - 0.6f, cp.position.y + 2.1f, cp.position.z - 1.0f };
        Color flagColor = cp.activated ? Color{ 255, 215, 0, 255 } : Color{ 140, 150, 160, 255 };
        DrawCube(flagCenter, 0.7f, 0.5f, 0.05f, flagColor);
        DrawCubeWires(flagCenter, 0.72f, 0.52f, 0.07f, DarkenColor(flagColor, 0.6f));
    }

    // 3. Draw Finish zone
    if (hasFinish) {
        Vector3 fCenter = finish.triggerAABB.GetCenter();
        Vector3 fSize = finish.triggerAABB.GetSize();

        // Finish platform / portal pad
        DrawCube(fCenter, fSize.x, fSize.y, fSize.z, Color{ 255, 215, 0, 160 }); // Translucent golden glow
        DrawCubeWires(fCenter, fSize.x + 0.04f, fSize.y + 0.04f, fSize.z + 0.04f, Color{ 220, 160, 0, 255 });

        // Floating spinning trophy cube at center
        float t = static_cast<float>(GetTime());
        Vector3 trophyPos = { fCenter.x, fCenter.y + 1.5f + sinf(t * 3.0f) * 0.2f, fCenter.z };
        DrawCube(trophyPos, 1.2f, 1.2f, 1.2f, Color{ 255, 230, 80, 255 });
        DrawCubeWires(trophyPos, 1.22f, 1.22f, 1.22f, Color{ 200, 150, 20, 255 });
    }
}

int Level::GetActivatedCheckpointCount() const {
    return checkpointsActivated;
}

int Level::GetTotalCheckpointCount() const {
    return static_cast<int>(checkpoints.size());
}
