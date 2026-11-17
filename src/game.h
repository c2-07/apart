#pragma once
#include "raylib.h"
#include "constants.h"
#include "collision.h"
#include "camera.h"
#include "player.h"
#include "level.h"
#include "ui.h"
#include "audio.h"
#include "particle.h"
#include "validator.h"

#include <string>
#include <vector>

enum class AppState {
    MAIN_MENU,
    LEVEL_SELECT,
    CHARACTER_SELECT,
    SETTINGS,
    PLAYING,
    PAUSED,
    RESULTS
};

struct LevelProgress {
    float bestTime = 0.0f;
    int stars = 0;
};

struct SaveData {
    int chosenColor = 0;
    float volume = 0.8f;
    float sensitivity = 0.18f;
    bool invertY = false;
    bool isFullscreen = false;
    int unlockedWorlds = 1;
    std::vector<LevelProgress> levels; // 20 entries

    void InitDefaults();
    bool Load(const std::string& path = "savegame.txt");
    bool Save(const std::string& path = "savegame.txt") const;
};

class Game {
public:
    AppState state;
    AppState previousState; // For returning from settings
    SaveData save;

    Level level;
    Player player;
    OrbitCamera camera;
    ParticleSystem particles;

    int currentLevelIdx;
    bool noclipMode;
    float screenShakeTimer;
    float respawnFadeAlpha;
    float levelLoadFadeAlpha;
    int previousDeaths;
    float accumulator;

    // Visual Pass Shaders
    Shader lightingShader;
    bool shaderLoaded;
    int lightDirLoc;
    int lightColorLoc;
    int ambientColorLoc;
    int viewPosLoc;
    int fogColorLoc;
    int fogDensityLoc;

    Game();
    void Init();
    void Shutdown();
    void ApplyLightingShader(Vector3 eyePos, int worldId);

    void Update(float frameDelta);
    void Draw();

    void LoadLevelByIndex(int index);
    void CompleteLevel();

private:
    void UpdateMainMenu();
    void DrawMainMenu();

    void UpdateLevelSelect();
    void DrawLevelSelect();

    void UpdateCharacterSelect();
    void DrawCharacterSelect();

    void UpdateSettings();
    void DrawSettings();

    void UpdatePlaying(float frameDelta);
    void DrawPlaying();

    void UpdatePaused();
    void DrawPaused();

    void UpdateResults();
    void DrawResults();
};
