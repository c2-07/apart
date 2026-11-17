#include "level.h"
#include "constants.h"
#include "audio.h"
#include "raymath.h"
#include "rlgl.h"

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
            pal.colors[0] = Color{ 40, 160, 200, 255 };
            pal.colors[1] = Color{ 235, 190, 120, 255 };
            pal.colors[2] = Color{ 70, 120, 180, 255 };
            pal.colors[3] = Color{ 250, 215, 90, 255 };
            pal.hazard    = Color{ 230, 45, 80, 255 };
            pal.accent    = Color{ 50, 230, 190, 255 };
            break;
        case 3: // Winter
            pal.skyTop    = Color{ 110, 145, 185, 255 };
            pal.skyBottom = Color{ 215, 230, 245, 255 };
            pal.colors[0] = Color{ 235, 245, 255, 255 };
            pal.colors[1] = Color{ 145, 195, 225, 255 };
            pal.colors[2] = Color{ 110, 130, 155, 255 };
            pal.colors[3] = Color{ 190, 220, 235, 255 };
            pal.hazard    = Color{ 220, 60, 95, 255 };
            pal.accent    = Color{ 90, 220, 255, 255 };
            break;
        case 4: // Factory
            pal.skyTop    = Color{ 70, 75, 85, 255 };
            pal.skyBottom = Color{ 140, 135, 130, 255 };
            pal.colors[0] = Color{ 180, 100, 40, 255 };
            pal.colors[1] = Color{ 90, 95, 105, 255 };
            pal.colors[2] = Color{ 230, 160, 30, 255 };
            pal.colors[3] = Color{ 60, 65, 75, 255 };
            pal.hazard    = Color{ 245, 50, 40, 255 };
            pal.accent    = Color{ 70, 230, 120, 255 };
            break;
        case 1: // Meadow (Default)
        default:
            pal.skyTop    = Color{ 120, 190, 245, 255 };
            pal.skyBottom = Color{ 200, 235, 255, 255 };
            pal.colors[0] = Color{ 90, 185, 75, 255 };
            pal.colors[1] = Color{ 185, 130, 80, 255 };
            pal.colors[2] = Color{ 160, 175, 180, 255 };
            pal.colors[3] = Color{ 245, 205, 60, 255 };
            pal.hazard    = Color{ 235, 65, 50, 255 };
            pal.accent    = Color{ 50, 220, 140, 255 };
            break;
    }
    return pal;
}

AABB Mover::GetAABB() const {
    return AABB::FromCenterSize(currentPos, size);
}

bool Spinner::CheckOverlapWithPlayer(const AABB& playerAABB) const {
    float halfH = height * 0.5f;
    if (playerAABB.max.y < (center.y - halfH) || playerAABB.min.y > (center.y + halfH)) {
        return false;
    }

    float cosA = cosf(currentAngle);
    float sinA = sinf(currentAngle);
    float halfL = length * 0.5f;

    Vector2 p1 = { center.x - cosA * halfL, center.z - sinA * halfL };
    Vector2 p2 = { center.x + cosA * halfL, center.z + sinA * halfL };

    Vector3 pCenter = playerAABB.GetCenter();
    Vector2 pPos = { pCenter.x, pCenter.z };

    Vector2 seg = { p2.x - p1.x, p2.y - p1.y };
    float segLenSq = seg.x * seg.x + seg.y * seg.y;
    float t = 0.0f;
    if (segLenSq > 1e-5f) {
        t = ((pPos.x - p1.x) * seg.x + (pPos.y - p1.y) * seg.y) / segLenSq;
        t = std::clamp(t, 0.0f, 1.0f);
    }
    Vector2 closest = { p1.x + seg.x * t, p1.y + seg.y * t };

    float dx = pPos.x - closest.x;
    float dz = pPos.y - closest.y;
    float distSq = dx * dx + dz * dz;

    constexpr float HIT_RADIUS = 0.72f;
    return distSq <= (HIT_RADIUS * HIT_RADIUS);
}

AABB CrumbleTile::GetAABB() const {
    return AABB::FromCenterSize(currentPos, size);
}

bool CrumbleTile::IsSolid() const {
    return state != CrumbleState::FALLEN;
}

// --- GhostRecorder Implementation ---
GhostRecorder::GhostRecorder()
    : state(State::IDLE),
      playbackIndex(0),
      recordTimer(0.0f) {
    frames.reserve(2400); // 20s at 120Hz
}

void GhostRecorder::StartRecording() {
    frames.clear();
    playbackIndex = 0;
    recordTimer = 0.0f;
    state = State::RECORDING;
    std::cout << "[GhostRecorder] Started recording ghost actions..." << std::endl;
}

void GhostRecorder::StopRecording() {
    if (state == State::RECORDING) {
        if (!frames.empty()) {
            StartPlayback();
        } else {
            state = State::IDLE;
        }
    }
}

void GhostRecorder::StartPlayback() {
    if (!frames.empty()) {
        state = State::PLAYING;
        playbackIndex = 0;
        std::cout << "[GhostRecorder] Started ghost playback (" << frames.size() << " frames)." << std::endl;
    } else {
        state = State::IDLE;
    }
}

void GhostRecorder::Reset() {
    state = State::IDLE;
    frames.clear();
    playbackIndex = 0;
    recordTimer = 0.0f;
}

void GhostRecorder::RecordTick(Vector3 pos, float yaw, bool grounded) {
    if (state == State::RECORDING) {
        frames.push_back(GhostFrame{ pos, yaw, grounded });
        recordTimer += Constants::PHYSICS_DT;
        if (recordTimer >= MAX_RECORD_TIME) {
            StopRecording();
        }
    }
}

void GhostRecorder::UpdatePlayback() {
    if (state == State::PLAYING && !frames.empty()) {
        playbackIndex++;
        if (playbackIndex >= frames.size()) {
            playbackIndex = 0; // Loop ghost recording
        }
    }
}

Vector3 GhostRecorder::GetCurrentPos() const {
    if (frames.empty() || state != State::PLAYING) {
        return Vector3{ 0.0f, -999.0f, 0.0f };
    }
    return frames[playbackIndex].position;
}

float GhostRecorder::GetCurrentYaw() const {
    if (frames.empty() || state != State::PLAYING) return 0.0f;
    return frames[playbackIndex].yaw;
}

bool GhostRecorder::HasActiveGhost() const {
    return state == State::PLAYING && !frames.empty();
}

AABB GhostRecorder::GetAABB() const {
    Vector3 p = GetCurrentPos();
    return AABB{
        Vector3{ p.x - Constants::HALF_PLAYER_WIDTH, p.y, p.z - Constants::HALF_PLAYER_DEPTH },
        Vector3{ p.x + Constants::HALF_PLAYER_WIDTH, p.y + Constants::PLAYER_HEIGHT, p.z + Constants::HALF_PLAYER_DEPTH }
    };
}

void GhostRecorder::Draw() const {
    if (!HasActiveGhost()) return;

    Vector3 pos = GetCurrentPos();
    float yaw = GetCurrentYaw();

    rlPushMatrix();
    rlTranslatef(pos.x, pos.y, pos.z);
    rlRotatef(yaw * RAD2DEG, 0.0f, 1.0f, 0.0f);

    Color ghostBody = Color{ 80, 220, 255, 140 };
    Color ghostWire = Color{ 160, 245, 255, 220 };

    // Torso
    DrawCube(Vector3{ 0.0f, 1.20f, 0.0f }, 0.8f, 0.9f, 0.45f, ghostBody);
    DrawCubeWires(Vector3{ 0.0f, 1.20f, 0.0f }, 0.82f, 0.92f, 0.47f, ghostWire);

    // Head
    DrawCube(Vector3{ 0.0f, 1.95f, 0.0f }, 0.55f, 0.55f, 0.55f, ghostBody);
    DrawCubeWires(Vector3{ 0.0f, 1.95f, 0.0f }, 0.57f, 0.57f, 0.57f, ghostWire);

    // Limbs
    DrawCube(Vector3{ 0.22f, 0.375f, 0.0f }, 0.34f, 0.75f, 0.34f, ghostBody);
    DrawCube(Vector3{ -0.22f, 0.375f, 0.0f }, 0.34f, 0.75f, 0.34f, ghostBody);
    DrawCube(Vector3{ 0.55f, 1.25f, 0.0f }, 0.28f, 0.75f, 0.28f, ghostBody);
    DrawCube(Vector3{ -0.55f, 1.25f, 0.0f }, 0.28f, 0.75f, 0.28f, ghostBody);

    rlPopMatrix();
}

// --- Level Implementation ---
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
    movers.clear();
    spinners.clear();
    crumbleTiles.clear();
    spikesList.clear();
    bouncePads.clear();
    buttons.clear();
    doors.clear();
    ghost.Reset();
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
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) continue;
        if (line[first] == '#') continue;

        std::stringstream ss(line.substr(first));
        std::string token;
        ss >> token;

        if (token == "name") {
            std::string rem;
            std::getline(ss, rem);
            size_t remFirst = rem.find_first_not_of(" \t\r\n");
            if (remFirst != std::string::npos) name = rem.substr(remFirst);
        } else if (token == "world") {
            if (!(ss >> worldId)) std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid world\n";
        } else if (token == "killY") {
            if (!(ss >> killY)) std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid killY\n";
        } else if (token == "par") {
            if (!(ss >> parTime)) std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid par\n";
        } else if (token == "spawn") {
            float x, y, z;
            if (ss >> x >> y >> z) spawnPos = Vector3{ x, y, z };
            else std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid spawn\n";
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
        } else if (token == "mover") {
            float x, y, z, sx, sy, sz, dx, dy, dz, period;
            int cIndex = 0;
            if (ss >> x >> y >> z >> sx >> sy >> sz >> dx >> dy >> dz >> period) {
                if (!(ss >> cIndex)) cIndex = 0;
                Mover m;
                m.basePos = Vector3{ x, y, z };
                m.size = Vector3{ sx, sy, sz };
                m.delta = Vector3{ dx, dy, dz };
                m.period = std::max(0.1f, period);
                m.colorIndex = std::clamp(cIndex, 0, 3);
                m.currentPos = m.basePos;
                m.frameDisplacement = Vector3{ 0.0f, 0.0f, 0.0f };
                movers.push_back(m);
            } else {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid mover\n";
            }
        } else if (token == "spinner") {
            float x, y, z, length, height, speed;
            if (ss >> x >> y >> z >> length >> height >> speed) {
                Spinner s;
                s.center = Vector3{ x, y, z };
                s.length = length;
                s.height = height;
                s.speed = speed;
                s.currentAngle = 0.0f;
                spinners.push_back(s);
            } else {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid spinner\n";
            }
        } else if (token == "crumble") {
            float x, y, z, sx, sy, sz, delay, respawnTime;
            if (ss >> x >> y >> z >> sx >> sy >> sz >> delay >> respawnTime) {
                CrumbleTile c;
                c.basePos = Vector3{ x, y, z };
                c.size = Vector3{ sx, sy, sz };
                c.delay = std::max(0.1f, delay);
                c.respawnTime = std::max(0.1f, respawnTime);
                c.state = CrumbleState::STABLE;
                c.timer = 0.0f;
                c.currentPos = c.basePos;
                crumbleTiles.push_back(c);
            } else {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid crumble\n";
            }
        } else if (token == "spikes") {
            float x, y, z, sx, sy, sz;
            if (ss >> x >> y >> z >> sx >> sy >> sz) {
                Spikes sp;
                sp.center = Vector3{ x, y, z };
                sp.size = Vector3{ sx, sy, sz };
                sp.aabb = AABB::FromCenterSize(sp.center, sp.size);
                spikesList.push_back(sp);
            } else {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid spikes\n";
            }
        } else if (token == "bounce") {
            float x, y, z, sx, sy, sz, strength;
            if (ss >> x >> y >> z >> sx >> sy >> sz >> strength) {
                BouncePad b;
                b.center = Vector3{ x, y, z };
                b.size = Vector3{ sx, sy, sz };
                b.strength = strength;
                b.aabb = AABB::FromCenterSize(b.center, b.size);
                bouncePads.push_back(b);
            } else {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid bounce\n";
            }
        } else if (token == "button") {
            int bId = 0;
            float x, y, z, dur;
            if (ss >> bId >> x >> y >> z >> dur) {
                ButtonEntity btn;
                btn.id = bId;
                btn.position = Vector3{ x, y, z };
                btn.duration = std::max(0.0f, dur);
                btn.currentTimer = 0.0f;
                btn.isPressed = false;
                btn.isActive = false;
                btn.baseAABB = AABB::FromCenterSize(Vector3{ x, y - 0.15f, z }, Vector3{ 2.2f, 0.3f, 2.2f });
                btn.triggerAABB = AABB::FromCenterSize(Vector3{ x, y + 0.3f, z }, Vector3{ 2.0f, 0.7f, 2.0f });
                buttons.push_back(btn);
            } else {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid button\n";
            }
        } else if (token == "door") {
            int dId = 0;
            float x, y, z, sx, sy, sz;
            if (ss >> dId >> x >> y >> z >> sx >> sy >> sz) {
                DoorEntity dr;
                dr.id = dId;
                dr.center = Vector3{ x, y, z };
                dr.size = Vector3{ sx, sy, sz };
                dr.aabb = AABB::FromCenterSize(dr.center, dr.size);
                dr.isOpen = false;
                doors.push_back(dr);
            } else {
                std::cerr << "[Level Parser] Error in " << filepath << ":" << lineNum << ": invalid door\n";
            }
        }
    }

    std::cout << "[Level Parser] Loaded '" << name << "': "
              << boxes.size() << " boxes, " << movers.size() << " movers, "
              << buttons.size() << " buttons, " << doors.size() << " doors." << std::endl;
    return true;
}

void Level::Reset(Player& player) {
    elapsedTime = 0.0f;
    checkpointsActivated = 0;
    deaths = 0;
    isCompleted = false;
    starRating = 0;

    for (auto& cp : checkpoints) cp.activated = false;
    for (auto& m : movers) {
        m.currentPos = m.basePos;
        m.frameDisplacement = Vector3{ 0.0f, 0.0f, 0.0f };
    }
    for (auto& s : spinners) s.currentAngle = 0.0f;
    for (auto& c : crumbleTiles) {
        c.state = CrumbleState::STABLE;
        c.timer = 0.0f;
        c.currentPos = c.basePos;
    }
    for (auto& btn : buttons) {
        btn.currentTimer = 0.0f;
        btn.isPressed = false;
        btn.isActive = false;
    }
    for (auto& dr : doors) dr.isOpen = false;

    ghost.Reset();
    player.Init(spawnPos);
}

void Level::Update(float dt, Player& player) {
    if (!isCompleted) {
        elapsedTime += dt;
    }

    // 1. Ghost Recorder Tick / Playback
    if (ghost.state == GhostRecorder::State::RECORDING) {
        ghost.RecordTick(player.position, player.modelYaw, player.isGrounded);
    } else if (ghost.state == GhostRecorder::State::PLAYING) {
        ghost.UpdatePlayback();
    }

    // 2. Movers
    AABB playerAABB = player.GetAABB();
    for (auto& m : movers) {
        float phase = fmodf(elapsedTime, m.period) / m.period;
        float s = 0.5f * (1.0f - cosf(phase * 2.0f * PI));
        Vector3 nextPos = Vector3Add(m.basePos, Vector3Scale(m.delta, s));
        m.frameDisplacement = Vector3Subtract(nextPos, m.currentPos);
        m.currentPos = nextPos;

        AABB moverAABB = m.GetAABB();
        bool standingOnTop = player.isGrounded &&
                             (std::abs(player.position.y - moverAABB.max.y) < 0.05f) &&
                             (playerAABB.min.x < moverAABB.max.x && playerAABB.max.x > moverAABB.min.x) &&
                             (playerAABB.min.z < moverAABB.max.z && playerAABB.max.z > moverAABB.min.z);

        if (standingOnTop) {
            player.position = Vector3Add(player.position, m.frameDisplacement);
        }
    }

    // 3. Spinners
    for (auto& s : spinners) {
        s.currentAngle += s.speed * dt;
        if (s.CheckOverlapWithPlayer(player.GetAABB())) {
            deaths++;
            player.deathCount = deaths;
            player.Respawn();
            AudioManager::Play(SoundId::DEATH);
            return;
        }
    }

    // 4. Crumble Tiles
    playerAABB = player.GetAABB();
    for (auto& c : crumbleTiles) {
        AABB cAABB = c.GetAABB();
        bool playerStanding = player.isGrounded &&
                              (std::abs(player.position.y - cAABB.max.y) < 0.05f) &&
                              (playerAABB.min.x < cAABB.max.x && playerAABB.max.x > cAABB.min.x) &&
                              (playerAABB.min.z < cAABB.max.z && playerAABB.max.z > cAABB.min.z);

        switch (c.state) {
            case CrumbleState::STABLE:
                if (playerStanding) {
                    c.state = CrumbleState::CRUMBLING;
                    c.timer = c.delay;
                }
                break;
            case CrumbleState::CRUMBLING:
                c.timer -= dt;
                c.currentPos.x = c.basePos.x + sinf(c.timer * 60.0f) * 0.06f;
                c.currentPos.z = c.basePos.z + cosf(c.timer * 60.0f) * 0.06f;
                if (c.timer <= 0.0f) {
                    c.state = CrumbleState::FALLEN;
                    c.timer = c.respawnTime;
                    c.currentPos = c.basePos;
                }
                break;
            case CrumbleState::FALLEN:
                c.timer -= dt;
                if (c.timer <= 0.0f) {
                    c.state = CrumbleState::STABLE;
                    c.timer = 0.0f;
                    c.currentPos = c.basePos;
                }
                break;
        }
    }

    // 5. Spikes
    for (const auto& sp : spikesList) {
        if (CheckAABBOverlap(player.GetAABB(), sp.aabb)) {
            deaths++;
            player.deathCount = deaths;
            player.Respawn();
            AudioManager::Play(SoundId::DEATH);
            return;
        }
    }

    // 6. Bounce Pads
    for (const auto& bp : bouncePads) {
        if (CheckAABBOverlap(player.GetAABB(), bp.aabb)) {
            if (player.velocity.y <= 1.0f) {
                player.velocity.y = bp.strength;
                player.isGrounded = false;
                AudioManager::Play(SoundId::JUMP);
            }
        }
    }

    // 7. Buttons & Doors Logic (Milestone 6)
    AABB ghostAABB = ghost.GetAABB();
    playerAABB = player.GetAABB();

    for (auto& btn : buttons) {
        bool playerOn = CheckAABBOverlap(playerAABB, btn.triggerAABB);
        bool ghostOn = ghost.HasActiveGhost() && CheckAABBOverlap(ghostAABB, btn.triggerAABB);
        btn.isPressed = playerOn || ghostOn;

        if (btn.isPressed) {
            btn.currentTimer = btn.duration;
            btn.isActive = true;
        } else if (btn.currentTimer > 0.0f) {
            btn.currentTimer -= dt;
            btn.isActive = (btn.currentTimer > 0.0f);
        } else {
            btn.isActive = false;
        }
    }

    for (auto& dr : doors) {
        bool shouldOpen = false;
        for (const auto& btn : buttons) {
            if (btn.id == dr.id && btn.isActive) {
                shouldOpen = true;
                break;
            }
        }
        dr.isOpen = shouldOpen;
    }

    // 8. Kill Plane
    if (player.position.y < killY) {
        deaths++;
        player.deathCount = deaths;
        player.Respawn();
        AudioManager::Play(SoundId::DEATH);
        return;
    }

    // 9. Checkpoints
    playerAABB = player.GetAABB();
    for (auto& cp : checkpoints) {
        if (!cp.activated && CheckAABBOverlap(playerAABB, cp.triggerAABB)) {
            cp.activated = true;
            player.spawnPoint = cp.position;
            checkpointsActivated++;
            AudioManager::Play(SoundId::CHECKPOINT);
        }
    }

    // 10. Finish Zone
    if (hasFinish && !isCompleted) {
        if (CheckAABBOverlap(playerAABB, finish.triggerAABB)) {
            isCompleted = true;
            AudioManager::Play(SoundId::FINISH);
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
    solids.reserve(boxes.size() + checkpoints.size() + movers.size() +
                   crumbleTiles.size() + bouncePads.size() + buttons.size() + doors.size());

    for (const auto& b : boxes) solids.push_back(b.aabb);
    for (const auto& cp : checkpoints) {
        solids.push_back(AABB::FromCenterSize(Vector3{ cp.position.x, cp.position.y - 0.25f, cp.position.z }, Vector3{ 3.0f, 0.5f, 3.0f }));
    }
    for (const auto& m : movers) solids.push_back(m.GetAABB());
    for (const auto& c : crumbleTiles) {
        if (c.IsSolid()) solids.push_back(c.GetAABB());
    }
    for (const auto& bp : bouncePads) solids.push_back(bp.aabb);

    // Button base plates
    for (const auto& btn : buttons) solids.push_back(btn.baseAABB);

    // Doors: only solid when CLOSED
    for (const auto& dr : doors) {
        if (!dr.isOpen) {
            solids.push_back(dr.aabb);
        }
    }

    return solids;
}

void Level::DrawSkybox() const {
    WorldPalette pal = GetWorldPalette(worldId);
    ClearBackground(pal.skyTop);

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

    // 1. Static Boxes
    for (const auto& b : boxes) {
        Vector3 center = b.aabb.GetCenter();
        Vector3 size = b.aabb.GetSize();
        Color faceColor = pal.colors[b.colorIndex];
        Color wireColor = DarkenColor(faceColor, 0.65f);
        DrawCube(center, size.x, size.y, size.z, faceColor);
        DrawCubeWires(center, size.x + 0.02f, size.y + 0.02f, size.z + 0.02f, wireColor);
    }

    // 2. Movers
    for (const auto& m : movers) {
        Color faceColor = pal.colors[m.colorIndex];
        Color wireColor = DarkenColor(faceColor, 0.6f);
        DrawCube(m.currentPos, m.size.x, m.size.y, m.size.z, faceColor);
        DrawCubeWires(m.currentPos, m.size.x + 0.03f, m.size.y + 0.03f, m.size.z + 0.03f, wireColor);
        DrawCube(Vector3{ m.currentPos.x, m.currentPos.y + m.size.y * 0.5f + Constants::SURFACE_DECAL_OFFSET_Y, m.currentPos.z },
                 m.size.x * 0.4f, 0.05f, m.size.z * 0.4f, pal.accent);
    }

    // 3. Spinners
    for (const auto& s : spinners) {
        DrawCube(s.center, 0.6f, s.height, 0.6f, Color{ 60, 65, 75, 255 });
        DrawCubeWires(s.center, 0.62f, s.height + 0.02f, 0.62f, Color{ 35, 40, 50, 255 });

        rlPushMatrix();
        rlTranslatef(s.center.x, s.center.y, s.center.z);
        rlRotatef(s.currentAngle * RAD2DEG, 0.0f, 1.0f, 0.0f);
        DrawCube(Vector3{ 0.0f, 0.0f, 0.0f }, s.length, 0.4f, 0.4f, pal.hazard);
        DrawCubeWires(Vector3{ 0.0f, 0.0f, 0.0f }, s.length + 0.02f, 0.42f, 0.42f, DarkenColor(pal.hazard, 0.6f));
        rlPopMatrix();
    }

    // 4. Crumble Tiles
    for (const auto& c : crumbleTiles) {
        if (c.state != CrumbleState::FALLEN) {
            Color cColor = (c.state == CrumbleState::CRUMBLING) ? Color{ 230, 100, 70, 255 } : pal.colors[1];
            DrawCube(c.currentPos, c.size.x, c.size.y, c.size.z, cColor);
            DrawCubeWires(c.currentPos, c.size.x + 0.02f, c.size.y + 0.02f, c.size.z + 0.02f, DarkenColor(cColor, 0.6f));
        } else {
            DrawCubeWires(c.basePos, c.size.x, c.size.y, c.size.z, Color{ 150, 150, 160, 60 });
        }
    }

    // 5. Spikes
    for (const auto& sp : spikesList) {
        DrawCube(Vector3{ sp.center.x, sp.center.y - sp.size.y * 0.4f, sp.center.z },
                 sp.size.x, sp.size.y * 0.2f, sp.size.z, Color{ 50, 50, 60, 255 });
        int countX = std::max(1, static_cast<int>(sp.size.x / 1.0f));
        int countZ = std::max(1, static_cast<int>(sp.size.z / 1.0f));
        float stepX = sp.size.x / countX;
        float stepZ = sp.size.z / countZ;
        for (int ix = 0; ix < countX; ++ix) {
            for (int iz = 0; iz < countZ; ++iz) {
                float sx = (sp.center.x - sp.size.x * 0.5f) + (ix + 0.5f) * stepX;
                float sz = (sp.center.z - sp.size.z * 0.5f) + (iz + 0.5f) * stepZ;
                Vector3 spikePos = { sx, sp.center.y, sz };
                DrawCube(spikePos, stepX * 0.7f, sp.size.y * 0.8f, stepZ * 0.7f, pal.hazard);
                DrawCubeWires(spikePos, stepX * 0.72f, sp.size.y * 0.82f, stepZ * 0.72f, DarkenColor(pal.hazard, 0.6f));
            }
        }
    }

    // 6. Bounce Pads
    for (const auto& bp : bouncePads) {
        DrawCube(bp.center, bp.size.x, bp.size.y, bp.size.z, Color{ 250, 220, 50, 255 });
        DrawCubeWires(bp.center, bp.size.x + 0.03f, bp.size.y + 0.03f, bp.size.z + 0.03f, Color{ 200, 160, 20, 255 });
        Vector3 springPos = { bp.center.x, bp.center.y + bp.size.y * 0.5f + Constants::SURFACE_DECAL_OFFSET_Y, bp.center.z };
        DrawCube(springPos, bp.size.x * 0.5f, 0.06f, bp.size.z * 0.5f, Color{ 255, 60, 160, 255 });
    }

    // 7. Buttons (Milestone 6)
    for (const auto& btn : buttons) {
        // Base rim
        DrawCube(btn.baseAABB.GetCenter(), 2.3f, 0.3f, 2.3f, Color{ 55, 60, 70, 255 });
        DrawCubeWires(btn.baseAABB.GetCenter(), 2.32f, 0.32f, 2.32f, Color{ 35, 40, 50, 255 });

        // Depress plate if active
        float plateY = btn.position.y + (btn.isActive ? 0.02f : 0.12f);
        Color btnCol = btn.isActive ? Color{ 60, 240, 120, 255 } : Color{ 245, 140, 40, 255 };
        DrawCube(Vector3{ btn.position.x, plateY, btn.position.z }, 1.8f, 0.2f, 1.8f, btnCol);
        DrawCubeWires(Vector3{ btn.position.x, plateY, btn.position.z }, 1.82f, 0.22f, 1.82f, DarkenColor(btnCol, 0.6f));

        // Visible Countdown Ring around the button
        if (btn.isActive && btn.duration > 0.0f) {
            float ratio = std::clamp(btn.currentTimer / btn.duration, 0.0f, 1.0f);
            int segments = 16;
            int litSegments = static_cast<int>(ratio * segments);
            float ringRadius = 1.6f;
            for (int i = 0; i < litSegments; ++i) {
                float a = (2.0f * PI * i) / segments;
                Vector3 dotPos = {
                    btn.position.x + cosf(a) * ringRadius,
                    btn.position.y + Constants::SURFACE_DECAL_OFFSET_Y + 0.02f,
                    btn.position.z + sinf(a) * ringRadius
                };
                DrawCube(dotPos, 0.2f, 0.06f, 0.2f, Color{ 80, 255, 150, 240 });
            }
        }
    }

    // 8. Doors (Milestone 6)
    for (const auto& dr : doors) {
        if (!dr.isOpen) {
            // Closed: Solid barrier with caution pattern
            DrawCube(dr.center, dr.size.x, dr.size.y, dr.size.z, Color{ 175, 55, 65, 255 });
            DrawCubeWires(dr.center, dr.size.x + 0.02f, dr.size.y + 0.02f, dr.size.z + 0.02f, Color{ 235, 90, 100, 255 });
            // Center lock icon
            DrawCube(dr.center, 0.6f, 0.6f, dr.size.z + 0.08f, Color{ 255, 220, 60, 255 });
        } else {
            // Open: Holographic doorway frame with green glow
            DrawCubeWires(dr.center, dr.size.x, dr.size.y, dr.size.z, Color{ 70, 240, 150, 180 });
            DrawCube(dr.center, dr.size.x, dr.size.y, dr.size.z, Color{ 60, 240, 140, 40 });
        }
    }

    // 9. Ghost Clone (Milestone 6)
    ghost.Draw();

    // 10. Checkpoints
    for (const auto& cp : checkpoints) {
        Vector3 padCenter = { cp.position.x, cp.position.y + Constants::CHECKPOINT_PAD_OFFSET_Y, cp.position.z };
        Color padColor = cp.activated ? Color{ 80, 220, 100, 255 } : Color{ 180, 180, 190, 255 };
        DrawCube(padCenter, 3.0f, Constants::CHECKPOINT_PAD_HEIGHT, 3.0f, padColor);
        DrawCubeWires(padCenter, 3.02f, Constants::CHECKPOINT_PAD_HEIGHT + 0.01f, 3.02f, DarkenColor(padColor, 0.65f));

        Vector3 poleCenter = { cp.position.x - 1.0f, cp.position.y + 1.25f, cp.position.z - 1.0f };
        DrawCube(poleCenter, 0.15f, 2.5f, 0.15f, Color{ 70, 70, 80, 255 });

        Vector3 flagCenter = { cp.position.x - 0.6f, cp.position.y + 2.1f, cp.position.z - 1.0f };
        Color flagColor = cp.activated ? Color{ 255, 215, 0, 255 } : Color{ 140, 150, 160, 255 };
        DrawCube(flagCenter, 0.7f, 0.5f, 0.05f, flagColor);
        DrawCubeWires(flagCenter, 0.72f, 0.52f, 0.07f, DarkenColor(flagColor, 0.6f));
    }

    // 11. Finish zone
    if (hasFinish) {
        Vector3 fCenter = finish.triggerAABB.GetCenter();
        Vector3 fSize = finish.triggerAABB.GetSize();
        Vector3 renderCenter = { fCenter.x, fCenter.y + Constants::SURFACE_DECAL_OFFSET_Y, fCenter.z };
        DrawCube(renderCenter, fSize.x, fSize.y, fSize.z, Color{ 255, 215, 0, 160 });
        DrawCubeWires(renderCenter, fSize.x + 0.04f, fSize.y + 0.04f, fSize.z + 0.04f, Color{ 220, 160, 0, 255 });

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
