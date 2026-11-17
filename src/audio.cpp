#include "audio.h"
#include <cmath>
#include <cstdlib>
#include <iostream>

bool AudioManager::isInitialized = false;
float AudioManager::masterVolume = 0.8f;
Sound AudioManager::soundJump = {};
Sound AudioManager::soundLand = {};
Sound AudioManager::soundCheckpoint = {};
Sound AudioManager::soundDeath = {};
Sound AudioManager::soundFinish = {};

void AudioManager::Init() {
    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        std::cerr << "[AudioManager] Audio device could not be initialized (running silent mode)." << std::endl;
        isInitialized = false;
        return;
    }

    SetMasterVolume(masterVolume);

    Wave wJump = GenerateJumpWave();
    soundJump = LoadSoundFromWave(wJump);
    UnloadWave(wJump);

    Wave wLand = GenerateLandWave();
    soundLand = LoadSoundFromWave(wLand);
    UnloadWave(wLand);

    Wave wCp = GenerateCheckpointWave();
    soundCheckpoint = LoadSoundFromWave(wCp);
    UnloadWave(wCp);

    Wave wDeath = GenerateDeathWave();
    soundDeath = LoadSoundFromWave(wDeath);
    UnloadWave(wDeath);

    Wave wFinish = GenerateFinishWave();
    soundFinish = LoadSoundFromWave(wFinish);
    UnloadWave(wFinish);

    isInitialized = true;
    std::cout << "[AudioManager] Procedural sound effects generated and loaded successfully." << std::endl;
}

void AudioManager::Close() {
    if (!isInitialized) return;

    if (soundJump.frameCount > 0) UnloadSound(soundJump);
    if (soundLand.frameCount > 0) UnloadSound(soundLand);
    if (soundCheckpoint.frameCount > 0) UnloadSound(soundCheckpoint);
    if (soundDeath.frameCount > 0) UnloadSound(soundDeath);
    if (soundFinish.frameCount > 0) UnloadSound(soundFinish);

    CloseAudioDevice();
    isInitialized = false;
}

void AudioManager::Play(SoundId id) {
    if (!isInitialized) return;

    switch (id) {
        case SoundId::JUMP:
            PlaySound(soundJump);
            break;
        case SoundId::LAND:
            PlaySound(soundLand);
            break;
        case SoundId::CHECKPOINT:
            PlaySound(soundCheckpoint);
            break;
        case SoundId::DEATH:
            PlaySound(soundDeath);
            break;
        case SoundId::FINISH:
            PlaySound(soundFinish);
            break;
    }
}

void AudioManager::SetVolume(float volume) {
    masterVolume = std::clamp(volume, 0.0f, 1.0f);
    if (isInitialized) {
        SetMasterVolume(masterVolume);
    }
}

float AudioManager::GetVolume() {
    return masterVolume;
}

Wave AudioManager::GenerateJumpWave() {
    constexpr unsigned int sampleRate = 44100;
    constexpr float duration = 0.13f;
    unsigned int frameCount = static_cast<unsigned int>(sampleRate * duration);
    short* data = static_cast<short*>(malloc(frameCount * sizeof(short)));

    float phase = 0.0f;
    for (unsigned int i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float progress = t / duration;
        // Pitch sweep from 250 Hz to 550 Hz
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

Wave AudioManager::GenerateLandWave() {
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
        // Mix sine with a dash of noise for realistic thud
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

Wave AudioManager::GenerateCheckpointWave() {
    constexpr unsigned int sampleRate = 44100;
    constexpr float duration = 0.28f;
    unsigned int frameCount = static_cast<unsigned int>(sampleRate * duration);
    short* data = static_cast<short*>(malloc(frameCount * sizeof(short)));

    float phase = 0.0f;
    for (unsigned int i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float freq = (t < 0.14f) ? 523.25f : 659.25f; // C5 to E5
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

Wave AudioManager::GenerateDeathWave() {
    constexpr unsigned int sampleRate = 44100;
    constexpr float duration = 0.32f;
    unsigned int frameCount = static_cast<unsigned int>(sampleRate * duration);
    short* data = static_cast<short*>(malloc(frameCount * sizeof(short)));

    float phase = 0.0f;
    for (unsigned int i = 0; i < frameCount; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float progress = t / duration;
        // Pitch plunges from 350 Hz to 60 Hz with bitcrush distortion
        float freq = 350.0f * (1.0f - progress * 0.85f);
        phase += 2.0f * PI * freq / sampleRate;
        float envelope = 1.0f - progress;
        float raw = sinf(phase) + 0.3f * sinf(phase * 3.0f);
        // Slight saturation / distortion
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

Wave AudioManager::GenerateFinishWave() {
    constexpr unsigned int sampleRate = 44100;
    constexpr float duration = 0.52f;
    unsigned int frameCount = static_cast<unsigned int>(sampleRate * duration);
    short* data = static_cast<short*>(malloc(frameCount * sizeof(short)));

    float freqs[4] = { 523.25f, 659.25f, 783.99f, 1046.50f }; // C5, E5, G5, C6
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
