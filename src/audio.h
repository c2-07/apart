#pragma once
#include "raylib.h"

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

private:
    static bool isInitialized;
    static float masterVolume;
    static Sound soundJump;
    static Sound soundLand;
    static Sound soundCheckpoint;
    static Sound soundDeath;
    static Sound soundFinish;

    static Wave GenerateJumpWave();
    static Wave GenerateLandWave();
    static Wave GenerateCheckpointWave();
    static Wave GenerateDeathWave();
    static Wave GenerateFinishWave();
};
