#pragma once
#include "sound_bank.h"

enum class SoundId {
    JUMP,
    LAND,
    CHECKPOINT,
    DEATH,
    FINISH
};

class AudioManager {
public:
    static void Init();
    static void Close();
    static void Play(SoundId id);
    static void SetVolume(float volume);
    static float GetVolume();
    static SoundBank& GetSoundBank();
};
