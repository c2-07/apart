#include "sound_bank.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <algorithm>

namespace obby {

SoundBank::SoundBank()
    : m_initialized(false),
      m_volume(0.8f) {
    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        std::cerr << "[SoundBank] Audio device unavailable (running silent)." << std::endl;
        m_initialized = false;
        return;
    }

    SetMasterVolume(m_volume);

    // Procedurally synthesize and store sound waves into STL unordered_map
    Wave wJump = generateJumpWave();
    m_sounds["jump"] = LoadSoundFromWave(wJump);
    UnloadWave(wJump);

    Wave wLand = generateLandWave();
    m_sounds["land"] = LoadSoundFromWave(wLand);
    UnloadWave(wLand);

    Wave wCp = generateCheckpointWave();
    m_sounds["checkpoint"] = LoadSoundFromWave(wCp);
    UnloadWave(wCp);

    Wave wDeath = generateDeathWave();
    m_sounds["death"] = LoadSoundFromWave(wDeath);
    UnloadWave(wDeath);

    Wave wFinish = generateFinishWave();
    m_sounds["finish"] = LoadSoundFromWave(wFinish);
    UnloadWave(wFinish);

    m_initialized = true;
    std::cout << "[SoundBank] Loaded " << m_sounds.size() << " sound assets into RAII sound bank." << std::endl;
}

SoundBank::~SoundBank() {
    if (m_initialized) {
        for (auto& pair : m_sounds) {
            if (pair.second.frameCount > 0) {
                UnloadSound(pair.second);
            }
        }
        m_sounds.clear();
        CloseAudioDevice();
        m_initialized = false;
    }
}

void SoundBank::play(const std::string& name) const {
    if (!m_initialized) return;
    auto it = m_sounds.find(name);
    if (it != m_sounds.end()) {
        PlaySound(it->second);
    }
}

void SoundBank::play(SoundType type) const {
    switch (type) {
        case SoundType::JUMP:       play("jump"); break;
        case SoundType::LAND:       play("land"); break;
        case SoundType::CHECKPOINT: play("checkpoint"); break;
        case SoundType::DEATH:      play("death"); break;
        case SoundType::FINISH:     play("finish"); break;
    }
}

void SoundBank::setVolume(float volume) {
    m_volume = std::clamp(volume, 0.0f, 1.0f);
    if (m_initialized) {
        SetMasterVolume(m_volume);
    }
}

Wave SoundBank::generateJumpWave() {
    constexpr unsigned int sampleRate = 44100;
    constexpr float duration = 0.13f;
    unsigned int frameCount = static_cast<unsigned int>(sampleRate * duration);
    short* data = static_cast<short*>(malloc(frameCount * sizeof(short)));

    float phase = 0.0f;
    for (unsigned int i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float progress = t / duration;
        float freq = 250.0f + 300.0f * progress;
        phase += 2.0f * PI * freq / sampleRate;
        float envelope = 1.0f - progress;
        float sample = sinf(phase) * envelope;
        data[i] = static_cast<short>(sample * 28000.0f);
    }

    Wave w;
    w.frameCount = frameCount;
    w.sampleRate = sampleRate;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = data;
    return w;
}

Wave SoundBank::generateLandWave() {
    constexpr unsigned int sampleRate = 44100;
    constexpr float duration = 0.08f;
    unsigned int frameCount = static_cast<unsigned int>(sampleRate * duration);
    short* data = static_cast<short*>(malloc(frameCount * sizeof(short)));

    float phase = 0.0f;
    for (unsigned int i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float progress = t / duration;
        float freq = 120.0f * (1.0f - progress * 0.5f);
        phase += 2.0f * PI * freq / sampleRate;
        float envelope = expf(-18.0f * t);
        float noise = (static_cast<float>(rand() % 2000) / 1000.0f - 1.0f) * 0.2f;
        float sample = (sinf(phase) + noise) * envelope;
        data[i] = static_cast<short>(sample * 25000.0f);
    }

    Wave w;
    w.frameCount = frameCount;
    w.sampleRate = sampleRate;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = data;
    return w;
}

Wave SoundBank::generateCheckpointWave() {
    constexpr unsigned int sampleRate = 44100;
    constexpr float duration = 0.28f;
    unsigned int frameCount = static_cast<unsigned int>(sampleRate * duration);
    short* data = static_cast<short*>(malloc(frameCount * sizeof(short)));

    float phase = 0.0f;
    for (unsigned int i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float freq = (t < 0.14f) ? 523.25f : 659.25f;
        float noteT = (t < 0.14f) ? t : (t - 0.14f);
        phase += 2.0f * PI * freq / sampleRate;
        float envelope = 1.0f - (noteT / 0.14f);
        float sample = sinf(phase) * envelope;
        data[i] = static_cast<short>(sample * 26000.0f);
    }

    Wave w;
    w.frameCount = frameCount;
    w.sampleRate = sampleRate;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = data;
    return w;
}

Wave SoundBank::generateDeathWave() {
    constexpr unsigned int sampleRate = 44100;
    constexpr float duration = 0.32f;
    unsigned int frameCount = static_cast<unsigned int>(sampleRate * duration);
    short* data = static_cast<short*>(malloc(frameCount * sizeof(short)));

    float phase = 0.0f;
    for (unsigned int i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float progress = t / duration;
        float freq = 350.0f * (1.0f - progress * 0.85f);
        phase += 2.0f * PI * freq / sampleRate;
        float envelope = 1.0f - progress;
        float raw = sinf(phase) + 0.3f * sinf(phase * 3.0f);
        float saturated = std::clamp(raw * 1.5f, -1.0f, 1.0f);
        data[i] = static_cast<short>(saturated * envelope * 28000.0f);
    }

    Wave w;
    w.frameCount = frameCount;
    w.sampleRate = sampleRate;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = data;
    return w;
}

Wave SoundBank::generateFinishWave() {
    constexpr unsigned int sampleRate = 44100;
    constexpr float duration = 0.52f;
    unsigned int frameCount = static_cast<unsigned int>(sampleRate * duration);
    short* data = static_cast<short*>(malloc(frameCount * sizeof(short)));

    float freqs[4] = { 523.25f, 659.25f, 783.99f, 1046.50f };
    float noteDur = duration / 4.0f;
    float phase = 0.0f;

    for (unsigned int i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        int noteIdx = std::min(3, static_cast<int>(t / noteDur));
        float noteT = t - noteIdx * noteDur;
        phase += 2.0f * PI * freqs[noteIdx] / sampleRate;
        float envelope = 1.0f - (noteT / noteDur);
        float sample = sinf(phase) * envelope;
        data[i] = static_cast<short>(sample * 27000.0f);
    }

    Wave w;
    w.frameCount = frameCount;
    w.sampleRate = sampleRate;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = data;
    return w;
}

} // namespace obby
