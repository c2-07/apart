#include "ghost_recorder.h"
#include "constants.h"
#include "rlgl.h"
#include <iostream>

namespace obby {

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
    return AABB::FromCenterSize(
        Vector3{ p.x, p.y + Constants::PLAYER_HEIGHT * 0.5f, p.z },
        Vector3{ Constants::PLAYER_WIDTH, Constants::PLAYER_HEIGHT, Constants::PLAYER_DEPTH }
    );
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

} // namespace obby
