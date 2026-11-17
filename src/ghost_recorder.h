#pragma once
#include "raylib.h"
#include "aabb.h"
#include <vector>

namespace obby {

struct GhostFrame {
    Vector3 position;
    float yaw;
    bool isGrounded;
};

/**
 * @brief Records and plays back player ghost movements for cooperative puzzle solving.
 * Demonstrates state management, frame sampling, and replay simulation.
 */
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

} // namespace obby

using GhostRecorder = obby::GhostRecorder;
using GhostFrame = obby::GhostFrame;
