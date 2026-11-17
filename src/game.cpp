#include "game.h"
#include "menu_state.h"
#include "level_select_state.h"
#include "character_select_state.h"
#include "settings_state.h"
#include "play_state.h"
#include "pause_state.h"
#include "results_state.h"
#include "render_utils.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>

static const std::vector<std::string> LEVEL_PATHS = {
    // World 1: Meadow (Levels 1 - 5)
    "assets/levels/w1_l1.lvl",
    "assets/levels/w1_l2.lvl",
    "assets/levels/w1_l3.lvl",
    "assets/levels/w1_l4.lvl",
    "assets/levels/w1_l5.lvl",

    // World 2: Ocean / Lagoon (Levels 6 - 10)
    "assets/levels/w2_l1.lvl",
    "assets/levels/w2_l2.lvl",
    "assets/levels/w2_l3.lvl",
    "assets/levels/w2_l4.lvl",
    "assets/levels/w2_l5.lvl",

    // World 3: Winter / Ice Peak (Levels 11 - 15)
    "assets/levels/w3_l1.lvl",
    "assets/levels/w3_l2.lvl",
    "assets/levels/w3_l3.lvl",
    "assets/levels/w3_l4.lvl",
    "assets/levels/w3_l5.lvl",

    // World 4: Clockwork Factory (Levels 16 - 20)
    "assets/levels/w4_l1.lvl",
    "assets/levels/w4_l2.lvl",
    "assets/levels/w4_l3.lvl",
    "assets/levels/w4_l4.lvl",
    "assets/levels/w4_l5.lvl",
};

// =============================================================================
// SaveData Implementation
// =============================================================================

void SaveData::InitDefaults() {
    chosenColor = 0;
    volume = 0.8f;
    sensitivity = 0.18f;
    invertY = false;
    isFullscreen = false;
    unlockedWorlds = 1;
    levels.assign(20, LevelProgress{ 0.0f, 0 });
}

bool SaveData::Load(const std::string& path) {
    InitDefaults();
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream ss(line);
        std::string key;
        ss >> key;
        if (key == "color") ss >> chosenColor;
        else if (key == "volume") ss >> volume;
        else if (key == "sens") ss >> sensitivity;
        else if (key == "inverty") ss >> invertY;
        else if (key == "unlocked_worlds") ss >> unlockedWorlds;
        else if (key == "level") {
            int idx = -1;
            float bTime = 0.0f;
            int stars = 0;
            if (ss >> idx >> bTime >> stars) {
                if (idx >= 0 && idx < static_cast<int>(levels.size())) {
                    levels[idx].bestTime = bTime;
                    levels[idx].stars = stars;
                }
            }
        }
    }
    std::cout << "[SaveData] Loaded savegame from " << path << " (unlocked worlds: " << unlockedWorlds << ")" << std::endl;
    return true;
}

bool SaveData::Save(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[SaveData] Failed to write savegame to " << path << std::endl;
        return false;
    }

    file << "# Blocky Obby Adventure Save File\n";
    file << "color " << chosenColor << "\n";
    file << "volume " << volume << "\n";
    file << "sens " << sensitivity << "\n";
    file << "inverty " << (invertY ? 1 : 0) << "\n";
    file << "unlocked_worlds " << unlockedWorlds << "\n";
    for (size_t i = 0; i < levels.size(); ++i) {
        file << "level " << i << " " << levels[i].bestTime << " " << levels[i].stars << "\n";
    }
    std::cout << "[SaveData] Saved progress to " << path << std::endl;
    return true;
}

// =============================================================================
// Game Implementation
// =============================================================================

namespace obby {

Game::Game()
    : m_currentState(nullptr),
      m_pendingState(nullptr),
      state(AppState::MAIN_MENU),
      previousState(AppState::MAIN_MENU),
      currentLevelIdx(0),
      noclipMode(false),
      screenShakeTimer(0.0f),
      respawnFadeAlpha(0.0f),
      levelLoadFadeAlpha(0.0f),
      displayedCheckpoints(0.0f),
      levelPlayTime(0.0f),
      showFps(false),
      previousDeaths(0),
      accumulator(0.0f),
      lightingShader(Shader{ 0, nullptr }),
      shaderLoaded(false),
      lightDirLoc(-1),
      lightColorLoc(-1),
      ambientColorLoc(-1),
      viewPosLoc(-1),
      fogColorLoc(-1),
      fogDensityLoc(-1) {}

void Game::Init() {
    save.Load();
    AudioManager::SetVolume(save.volume);

    player.setColorIndex(save.chosenColor);
    LoadLevelByIndex(0);

    // Load custom lighting shader if graphical window is ready
    if (IsWindowReady()) {
        lightingShader = LoadShader("assets/shaders/lighting.vs", "assets/shaders/lighting.fs");
        if (lightingShader.id > 0) {
            shaderLoaded = true;

            // Uniform locations
            lightDirLoc = GetShaderLocation(lightingShader, "lightDir");
            lightColorLoc = GetShaderLocation(lightingShader, "lightColor");
            ambientColorLoc = GetShaderLocation(lightingShader, "ambientColor");
            viewPosLoc = GetShaderLocation(lightingShader, "viewPos");
            fogColorLoc = GetShaderLocation(lightingShader, "fogColor");
            fogDensityLoc = GetShaderLocation(lightingShader, "fogDensity");

            // Directional sun light uniforms
            float lDir[3] = { Constants::LIGHT_DIR_X, Constants::LIGHT_DIR_Y, Constants::LIGHT_DIR_Z };
            float lCol[4] = { Constants::SUN_LIGHT_INTENSITY, Constants::SUN_LIGHT_INTENSITY, Constants::SUN_LIGHT_INTENSITY, 1.0f };
            float ambCol[4] = { Constants::AMBIENT_LIGHT_INTENSITY, Constants::AMBIENT_LIGHT_INTENSITY, Constants::AMBIENT_LIGHT_INTENSITY, 1.0f };
            float fogDens = Constants::DEFAULT_FOG_DENSITY;

            SetShaderValue(lightingShader, lightDirLoc, lDir, SHADER_UNIFORM_VEC3);
            SetShaderValue(lightingShader, lightColorLoc, lCol, SHADER_UNIFORM_VEC4);
            SetShaderValue(lightingShader, ambientColorLoc, ambCol, SHADER_UNIFORM_VEC4);
            SetShaderValue(lightingShader, fogDensityLoc, &fogDens, SHADER_UNIFORM_FLOAT);
        }
    }

    // Set initial polymorphic state
    ChangeState(CreateState(StateType::MAIN_MENU));
}

void Game::Shutdown() {
    if (shaderLoaded) {
        UnloadShader(lightingShader);
        shaderLoaded = false;
    }

    save.chosenColor = player.getColorIndex();
    save.volume = AudioManager::GetVolume();
    save.Save();
}

void Game::ApplyLightingShader(Vector3 eyePos, int worldId) const {
    if (!shaderLoaded) return;
    WorldPalette pal = GetWorldPalette(worldId);

    float lCol[4] = {
        pal.lightColor.r / 255.0f,
        pal.lightColor.g / 255.0f,
        pal.lightColor.b / 255.0f,
        1.0f
    };
    float aCol[4] = {
        pal.ambientColor.r / 255.0f,
        pal.ambientColor.g / 255.0f,
        pal.ambientColor.b / 255.0f,
        1.0f
    };
    float camPos[3] = { eyePos.x, eyePos.y, eyePos.z };
    float horizonCol[4] = {
        pal.fogColor.r / 255.0f,
        pal.fogColor.g / 255.0f,
        pal.fogColor.b / 255.0f,
        1.0f
    };

    SetShaderValue(lightingShader, lightColorLoc, lCol, SHADER_UNIFORM_VEC4);
    SetShaderValue(lightingShader, ambientColorLoc, aCol, SHADER_UNIFORM_VEC4);
    SetShaderValue(lightingShader, viewPosLoc, camPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(lightingShader, fogColorLoc, horizonCol, SHADER_UNIFORM_VEC4);
    SetShaderValue(lightingShader, fogDensityLoc, &pal.fogDensity, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(lightingShader);
}

void Game::LoadLevelByIndex(int index) {
    if (index < 0 || index >= static_cast<int>(LEVEL_PATHS.size())) return;
    currentLevelIdx = index;
    level.LoadFromFile(LEVEL_PATHS[currentLevelIdx]);
    level.Reset(player);
    player.setColorIndex(save.chosenColor);
    camera.init(player.getPosition());
    particles.Clear();
    previousDeaths = 0;
    screenShakeTimer = 0.0f;
    respawnFadeAlpha = 0.0f;
    levelLoadFadeAlpha = 1.0f;
    displayedCheckpoints = 0.0f;
    levelPlayTime = 0.0f;
    accumulator = 0.0f;
}

void Game::CompleteLevel() {
    auto& prog = save.levels[currentLevelIdx];
    if (prog.bestTime <= 0.0f || level.elapsedTime < prog.bestTime) {
        prog.bestTime = level.elapsedTime;
    }
    if (level.starRating > prog.stars) {
        prog.stars = level.starRating;
    }

    int completedWorld = (currentLevelIdx / 5) + 1;
    if ((currentLevelIdx % 5) == 4 && completedWorld >= save.unlockedWorlds && save.unlockedWorlds < 4) {
        save.unlockedWorlds = completedWorld + 1;
    }

    save.Save();
    RequestStateChange(StateType::RESULTS);
}

std::unique_ptr<GameState> Game::CreateState(StateType type) {
    switch (type) {
        case StateType::MAIN_MENU:        return std::make_unique<MenuState>(*this);
        case StateType::LEVEL_SELECT:     return std::make_unique<LevelSelectState>(*this);
        case StateType::CHARACTER_SELECT: return std::make_unique<CharacterSelectState>(*this);
        case StateType::SETTINGS:         return std::make_unique<SettingsState>(*this);
        case StateType::PLAYING:          return std::make_unique<PlayState>(*this);
        case StateType::PAUSED:           return std::make_unique<PauseState>(*this);
        case StateType::RESULTS:          return std::make_unique<ResultsState>(*this);
    }
    return nullptr;
}

void Game::RequestStateChange(StateType newType) {
    switch (newType) {
        case StateType::MAIN_MENU:        state = AppState::MAIN_MENU; break;
        case StateType::LEVEL_SELECT:     state = AppState::LEVEL_SELECT; break;
        case StateType::CHARACTER_SELECT: state = AppState::CHARACTER_SELECT; break;
        case StateType::SETTINGS:         state = AppState::SETTINGS; break;
        case StateType::PLAYING:          state = AppState::PLAYING; break;
        case StateType::PAUSED:           state = AppState::PAUSED; break;
        case StateType::RESULTS:          state = AppState::RESULTS; break;
    }
    m_pendingState = CreateState(newType);
}

void Game::ChangeState(std::unique_ptr<GameState> newState) {
    if (m_currentState) {
        m_currentState->exit();
    }
    m_currentState = std::move(newState);
    if (m_currentState) {
        m_currentState->enter();
    }
}

void Game::Update(float frameDelta) {
    if (m_pendingState) {
        ChangeState(std::move(m_pendingState));
    }
    if (m_currentState) {
        m_currentState->update(frameDelta);
    }
}

void Game::Draw() const {
    if (m_currentState) {
        m_currentState->draw();
    }
}

void Game::DrawPlaying() const {
    std::vector<AABB> colliders = level.GetSolidColliders();

    level.DrawSkybox();
    BeginMode3D(camera.getRaylibCamera());
    ApplyLightingShader(camera.getCurrentEye().toRaylib(), level.worldId);
    level.Draw();
    player.draw();
    if (shaderLoaded) EndShaderMode();
    player.drawShadow(colliders);
    particles.Draw();
    EndMode3D();
}

} // namespace obby

