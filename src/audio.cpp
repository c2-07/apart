#include "audio.h"
#include <memory>

static std::unique_ptr<SoundBank> s_soundBank = nullptr;

SoundBank& AudioManager::GetSoundBank() {
    if (!s_soundBank) {
        s_soundBank = std::make_unique<SoundBank>();
    }
    return *s_soundBank;
}

void AudioManager::Init() {
    GetSoundBank();
}

void AudioManager::Close() {
    s_soundBank.reset();
}

void AudioManager::Play(SoundId id) {
    if (!s_soundBank) {
        GetSoundBank();
    }
    switch (id) {
        case SoundId::JUMP:       s_soundBank->play(SoundType::JUMP); break;
        case SoundId::LAND:       s_soundBank->play(SoundType::LAND); break;
        case SoundId::CHECKPOINT: s_soundBank->play(SoundType::CHECKPOINT); break;
        case SoundId::DEATH:      s_soundBank->play(SoundType::DEATH); break;
        case SoundId::FINISH:     s_soundBank->play(SoundType::FINISH); break;
    }
}

void AudioManager::SetVolume(float volume) {
    GetSoundBank().setVolume(volume);
}

float AudioManager::GetVolume() {
    return GetSoundBank().getVolume();
}
