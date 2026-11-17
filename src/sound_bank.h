#pragma once
#include "raylib.h"
#include <unordered_map>
#include <string>

namespace obby {

enum class SoundType {
    JUMP,
    LAND,
    CHECKPOINT,
    DEATH,
    FINISH
};

/**
 * @brief RAII Resource Bank managing procedural audio assets and sound device lifetime.
 * Demonstrates:
 * - RAII resource encapsulation (InitAudioDevice in constructor, CloseAudioDevice in destructor)
 * - STL container utilization (std::unordered_map<std::string, Sound>)
 * - Deleted copy constructor / copy assignment
 * - Safe procedural sound synthesis and cleanup
 */
class SoundBank {
private:
    std::unordered_map<std::string, Sound> m_sounds;
    bool m_initialized;
    float m_volume;

    static Wave generateJumpWave();
    static Wave generateLandWave();
    static Wave generateCheckpointWave();
    static Wave generateDeathWave();
    static Wave generateFinishWave();

public:
    SoundBank();
    ~SoundBank();

    // RAII semantics: Non-copyable
    SoundBank(const SoundBank&) = delete;
    SoundBank& operator=(const SoundBank&) = delete;

    void play(const std::string& name) const;
    void play(SoundType type) const;

    void setVolume(float volume);
    float getVolume() const { return m_volume; }
    bool isInitialized() const { return m_initialized; }
};

} // namespace obby

using SoundBank = obby::SoundBank;
using SoundType = obby::SoundType;
