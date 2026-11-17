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
#include "game_state.h"
#include "shader_holder.h"
#include "saveable.h"

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

struct SaveData : public ISaveable {
    int chosenColor = 0;
    float volume = 0.8f;
    float sensitivity = 0.18f;
    bool invertY = false;
    bool isFullscreen = false;
    int unlockedWorlds = 1;
    std::vector<LevelProgress> levels; // 20 entries

    virtual ~SaveData() override = default;

    void InitDefaults();
    virtual bool load(const std::string& path = "savegame.txt") override;
    virtual bool save(const std::string& path = "savegame.txt") const override;

    bool Load(const std::string& path = "savegame.txt") { return load(path); }
    bool Save(const std::string& path = "savegame.txt") const { return save(path); }
};

namespace obby {

class Game {
private:
    std::unique_ptr<GameState> m_currentState;
    std::unique_ptr<GameState> m_pendingState;

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
    float displayedCheckpoints;
    float levelPlayTime;
    bool showFps;
    int previousDeaths;
    float accumulator;

    // Visual Pass Shader (RAII Resource Holder)
    ShaderHolder lightingShader;
    bool shaderLoaded;

    Game();
    ~Game() = default;

    // Non-copyable context
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    void Init();
    void Shutdown();
    void ApplyLightingShader(Vector3 eyePos, int worldId) const;

    void Update(float frameDelta);
    void Draw() const;

    void LoadLevelByIndex(int index);
    void CompleteLevel();
    int GetTotalLevels() const { return 20; }

    // State Pattern dispatch & transition management
    void RequestStateChange(StateType newType);
    void ChangeState(std::unique_ptr<GameState> newState);
    std::unique_ptr<GameState> CreateState(StateType type);
    GameState* GetCurrentState() const { return m_currentState.get(); }

    // Rendering helper for sub-states (PauseState, ResultsState)
    void DrawPlaying() const;
};

} // namespace obby

using Game = obby::Game;
