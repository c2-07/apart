#include "game.h"
#include "raymath.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

static const std::vector<std::string> LEVEL_PATHS = {
    "assets/levels/w1_l1.lvl", "assets/levels/w1_l2.lvl", "assets/levels/w1_l3.lvl", "assets/levels/w1_l4.lvl", "assets/levels/w1_l5.lvl",
    "assets/levels/w2_l1.lvl", "assets/levels/w2_l2.lvl", "assets/levels/w2_l3.lvl", "assets/levels/w2_l4.lvl", "assets/levels/w2_l5.lvl",
    "assets/levels/w3_l1.lvl", "assets/levels/w3_l2.lvl", "assets/levels/w3_l3.lvl", "assets/levels/w3_l4.lvl", "assets/levels/w3_l5.lvl",
    "assets/levels/w4_l1.lvl", "assets/levels/w4_l2.lvl", "assets/levels/w4_l3.lvl", "assets/levels/w4_l4.lvl", "assets/levels/w4_l5.lvl"
};

// --- SaveData Implementation ---
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
    std::ifstream f(path);
    if (!f.is_open()) return false;

    std::string line;
    while (std::getline(f, line)) {
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos || line[first] == '#') continue;

        std::stringstream ss(line);
        std::string key;
        ss >> key;
        if (key == "color") ss >> chosenColor;
        else if (key == "volume") ss >> volume;
        else if (key == "sensitivity") ss >> sensitivity;
        else if (key == "invertY") ss >> invertY;
        else if (key == "fullscreen") ss >> isFullscreen;
        else if (key == "unlockedWorlds") ss >> unlockedWorlds;
        else if (key == "level") {
            int idx = 0;
            float time = 0.0f;
            int stars = 0;
            if (ss >> idx >> time >> stars) {
                if (idx >= 0 && idx < 20) {
                    levels[idx].bestTime = time;
                    levels[idx].stars = stars;
                }
            }
        }
    }
    std::cout << "[SaveData] Loaded savegame from " << path << " (unlocked worlds: " << unlockedWorlds << ")" << std::endl;
    return true;
}

bool SaveData::Save(const std::string& path) const {
    std::ofstream f(path);
    if (!f.is_open()) return false;

    f << "# Blocky Obby Adventure Save File\n";
    f << "color " << chosenColor << "\n";
    f << "volume " << volume << "\n";
    f << "sensitivity " << sensitivity << "\n";
    f << "invertY " << (invertY ? 1 : 0) << "\n";
    f << "fullscreen " << (isFullscreen ? 1 : 0) << "\n";
    f << "unlockedWorlds " << unlockedWorlds << "\n";

    for (size_t i = 0; i < levels.size(); ++i) {
        f << "level " << i << " " << levels[i].bestTime << " " << levels[i].stars << "\n";
    }
    std::cout << "[SaveData] Saved progress to " << path << std::endl;
    return true;
}

// --- Game Implementation ---
Game::Game()
    : state(AppState::MAIN_MENU),
      previousState(AppState::MAIN_MENU),
      currentLevelIdx(0),
      noclipMode(false),
      screenShakeTimer(0.0f),
      respawnFadeAlpha(0.0f),
      previousDeaths(0),
      accumulator(0.0f) {}

void Game::Init() {
    save.Load();
    AudioManager::SetVolume(save.volume);

    player.colorIndex = save.chosenColor;
    LoadLevelByIndex(0);
    state = AppState::MAIN_MENU;
}

void Game::Shutdown() {
    save.chosenColor = player.colorIndex;
    save.volume = AudioManager::GetVolume();
    save.Save();
}

void Game::LoadLevelByIndex(int index) {
    if (index < 0 || index >= static_cast<int>(LEVEL_PATHS.size())) return;
    currentLevelIdx = index;
    level.LoadFromFile(LEVEL_PATHS[currentLevelIdx]);
    level.Reset(player);
    player.colorIndex = save.chosenColor;
    camera.Init(player.position);
    particles.Clear();
    previousDeaths = 0;
    screenShakeTimer = 0.0f;
    respawnFadeAlpha = 0.0f;
    accumulator = 0.0f;
}

void Game::CompleteLevel() {
    // Record stars and best time
    auto& prog = save.levels[currentLevelIdx];
    if (prog.bestTime <= 0.0f || level.elapsedTime < prog.bestTime) {
        prog.bestTime = level.elapsedTime;
    }
    if (level.starRating > prog.stars) {
        prog.stars = level.starRating;
    }

    // World unlock progression: completing level 4 (World 1 boss) unlocks World 2, etc.
    int completedWorld = (currentLevelIdx / 5) + 1;
    if ((currentLevelIdx % 5) == 4 && completedWorld >= save.unlockedWorlds && save.unlockedWorlds < 4) {
        save.unlockedWorlds = completedWorld + 1;
    }

    save.Save();
    state = AppState::RESULTS;
}

void Game::Update(float frameDelta) {
    switch (state) {
        case AppState::MAIN_MENU:        UpdateMainMenu(); break;
        case AppState::LEVEL_SELECT:     UpdateLevelSelect(); break;
        case AppState::CHARACTER_SELECT: UpdateCharacterSelect(); break;
        case AppState::SETTINGS:         UpdateSettings(); break;
        case AppState::PLAYING:          UpdatePlaying(frameDelta); break;
        case AppState::PAUSED:           UpdatePaused(); break;
        case AppState::RESULTS:          UpdateResults(); break;
    }
}

void Game::Draw() {
    switch (state) {
        case AppState::MAIN_MENU:        DrawMainMenu(); break;
        case AppState::LEVEL_SELECT:     DrawLevelSelect(); break;
        case AppState::CHARACTER_SELECT: DrawCharacterSelect(); break;
        case AppState::SETTINGS:         DrawSettings(); break;
        case AppState::PLAYING:          DrawPlaying(); break;
        case AppState::PAUSED:           DrawPaused(); break;
        case AppState::RESULTS:          DrawResults(); break;
    }
}

// =============================================================================
// State Handlers
// =============================================================================

void Game::UpdateMainMenu() {
    // Background slow camera orbit
    camera.yaw += 12.0f * GetFrameTime();
    std::vector<AABB> colliders = level.GetSolidColliders();
    camera.Update(player.position, GetFrameTime(), colliders);
}

void Game::DrawMainMenu() {
    level.DrawSkybox();
    BeginMode3D(camera.GetRaylibCamera());
    level.Draw();
    player.DrawShadow(level.GetSolidColliders());
    player.Draw();
    EndMode3D();

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    // Backdrop shadow panel for menu
    Rectangle panel = { (sw - 380) * 0.5f, (sh - 480) * 0.5f, 380, 480 };
    DrawRectangleRounded(panel, 0.12f, 4, Color{ 15, 20, 30, 215 });
    DrawRectangleRoundedLines(panel, 0.12f, 4, Color{ 70, 95, 140, 240 });

    const char* title = "BLOCKY OBBY";
    const char* sub = "3D SOLO ADVENTURE";
    int tw = MeasureText(title, 34);
    int sww = MeasureText(sub, 16);
    DrawText(title, (int)(panel.x + (panel.width - tw) * 0.5f), (int)(panel.y + 35), 34, Color{ 255, 220, 50, 255 });
    DrawText(sub, (int)(panel.x + (panel.width - sww) * 0.5f), (int)(panel.y + 75), 16, Color{ 180, 215, 255, 230 });

    float btnY = panel.y + 120;
    float btnW = 280;
    float btnH = 48;
    float btnX = panel.x + (panel.width - btnW) * 0.5f;

    if (UI::Button(Rectangle{ btnX, btnY, btnW, btnH }, "PLAY", Color{ 45, 150, 85, 255 }, Color{ 65, 185, 110, 255 }, WHITE, 22)) {
        LoadLevelByIndex(currentLevelIdx);
        state = AppState::PLAYING;
    }
    if (UI::Button(Rectangle{ btnX, btnY + 65, btnW, btnH }, "LEVEL SELECT", Color{ 55, 80, 120, 255 }, Color{ 75, 105, 155, 255 }, WHITE, 20)) {
        state = AppState::LEVEL_SELECT;
    }
    if (UI::Button(Rectangle{ btnX, btnY + 130, btnW, btnH }, "CHARACTER", Color{ 120, 70, 140, 255 }, Color{ 150, 95, 175, 255 }, WHITE, 20)) {
        state = AppState::CHARACTER_SELECT;
    }
    if (UI::Button(Rectangle{ btnX, btnY + 195, btnW, btnH }, "SETTINGS", Color{ 70, 80, 95, 255 }, Color{ 95, 105, 125, 255 }, WHITE, 20)) {
        previousState = AppState::MAIN_MENU;
        state = AppState::SETTINGS;
    }
    if (UI::Button(Rectangle{ btnX, btnY + 260, btnW, btnH }, "QUIT", Color{ 140, 50, 60, 255 }, Color{ 180, 70, 80, 255 }, WHITE, 20)) {
        CloseWindow();
    }
}

void Game::UpdateLevelSelect() {}

void Game::DrawLevelSelect() {
    ClearBackground(Color{ 20, 25, 35, 255 });
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    const char* title = "LEVEL SELECT - 4 WORLDS";
    int tw = MeasureText(title, 26);
    DrawText(title, (sw - tw) / 2, 25, 26, Color{ 255, 225, 60, 255 });

    const char* worldNames[4] = { "World 1: Meadow", "World 2: Ocean", "World 3: Winter", "World 4: Factory" };
    Color worldColors[4] = { Color{ 85, 180, 75, 255 }, Color{ 50, 160, 220, 255 }, Color{ 140, 195, 235, 255 }, Color{ 230, 130, 40, 255 } };

    float startY = 75.0f;
    for (int w = 0; w < 4; ++w) {
        bool worldUnlocked = (w + 1) <= save.unlockedWorlds;

        float rowY = startY + w * 125.0f;
        DrawText(worldNames[w], 60, (int)rowY, 20, worldUnlocked ? worldColors[w] : Color{ 110, 115, 125, 255 });

        if (!worldUnlocked) {
            DrawText("[ LOCKED - Complete previous world to unlock ]", 250, (int)rowY + 2, 16, Color{ 140, 145, 155, 200 });
        }

        for (int l = 0; l < 5; ++l) {
            int lvlIdx = w * 5 + l;
            float btnX = 60.0f + l * 230.0f;
            float btnY = rowY + 30.0f;
            Rectangle tile = { btnX, btnY, 210, 75 };

            if (worldUnlocked) {
                char btnLabel[32];
                snprintf(btnLabel, sizeof(btnLabel), "Level %d-%d", w + 1, l + 1);

                int stars = save.levels[lvlIdx].stars;
                const char* starTxt = (stars >= 3) ? "***" : (stars == 2) ? "**-" : (stars == 1) ? "*--" : "---";

                Color bg = (currentLevelIdx == lvlIdx) ? Color{ 50, 120, 180, 255 } : Color{ 40, 50, 70, 240 };
                if (UI::Button(tile, btnLabel, bg, Color{ 60, 140, 210, 255 }, WHITE, 18)) {
                    LoadLevelByIndex(lvlIdx);
                    state = AppState::PLAYING;
                }

                // Stars and best time below button label
                char statBuf[48];
                if (save.levels[lvlIdx].bestTime > 0.0f) {
                    snprintf(statBuf, sizeof(statBuf), "[%s] %.1fs", starTxt, save.levels[lvlIdx].bestTime);
                } else {
                    snprintf(statBuf, sizeof(statBuf), "[%s] No Record", starTxt);
                }
                DrawText(statBuf, (int)(tile.x + 35), (int)(tile.y + 48), 14, Color{ 255, 215, 80, 240 });
            } else {
                DrawRectangleRounded(tile, 0.2f, 4, Color{ 35, 40, 50, 180 });
                DrawRectangleRoundedLines(tile, 0.2f, 4, Color{ 60, 65, 75, 200 });
                char lockTxt[32];
                snprintf(lockTxt, sizeof(lockTxt), "%d-%d (Locked)", w + 1, l + 1);
                DrawText(lockTxt, (int)(tile.x + 40), (int)(tile.y + 28), 16, Color{ 110, 115, 125, 200 });
            }
        }
    }

    // Back Button
    if (UI::Button(Rectangle{ (sw - 180) * 0.5f, (float)(sh - 65), 180, 42 }, "BACK TO MENU", Color{ 70, 80, 95, 255 }, Color{ 95, 110, 130, 255 }, WHITE, 16)) {
        state = AppState::MAIN_MENU;
    }
}

void Game::UpdateCharacterSelect() {
    player.modelYaw += 1.5f * GetFrameTime();
}

void Game::DrawCharacterSelect() {
    ClearBackground(Color{ 25, 30, 42, 255 });
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    const char* title = "CHARACTER CUSTOMIZATION";
    int tw = MeasureText(title, 26);
    DrawText(title, (sw - tw) / 2, 35, 26, Color{ 255, 220, 60, 255 });

    // 3D Preview of character
    Camera3D previewCam;
    previewCam.position = Vector3{ 0.0f, 1.4f, 4.0f };
    previewCam.target = Vector3{ 0.0f, 1.2f, 0.0f };
    previewCam.up = Vector3{ 0.0f, 1.0f, 0.0f };
    previewCam.fovy = 45.0f;
    previewCam.projection = CAMERA_PERSPECTIVE;

    BeginMode3D(previewCam);
    Vector3 oldPos = player.position;
    player.position = Vector3{ 0.0f, 0.0f, 0.0f };
    player.Draw();
    player.position = oldPos;
    EndMode3D();

    // Color Swatches
    const char* colorNames[6] = { "Classic Blue", "Crimson Red", "Emerald Green", "Royal Purple", "Sunset Orange", "Sunshine Yellow" };
    Color shirtColors[6] = {
        Color{ 55, 130, 235, 255 }, Color{ 225, 45, 55, 255 }, Color{ 45, 185, 95, 255 },
        Color{ 145, 65, 215, 255 }, Color{ 245, 125, 35, 255 }, Color{ 245, 215, 45, 255 }
    };

    float startX = (sw - (6 * 140)) * 0.5f;
    for (int i = 0; i < 6; ++i) {
        Rectangle swatch = { startX + i * 140.0f, (float)(sh - 180), 120, 50 };
        bool selected = (player.colorIndex == i);

        if (UI::Button(swatch, colorNames[i], shirtColors[i], shirtColors[i], WHITE, 14)) {
            player.colorIndex = i;
            save.chosenColor = i;
            save.Save();
        }
        if (selected) {
            DrawRectangleLinesEx(Rectangle{ swatch.x - 3, swatch.y - 3, swatch.width + 6, swatch.height + 6 }, 3.0f, WHITE);
        }
    }

    if (UI::Button(Rectangle{ (sw - 180) * 0.5f, (float)(sh - 80), 180, 42 }, "CONFIRM & BACK", Color{ 45, 140, 85, 255 }, Color{ 65, 175, 110, 255 }, WHITE, 16)) {
        state = AppState::MAIN_MENU;
    }
}

void Game::UpdateSettings() {}

void Game::DrawSettings() {
    ClearBackground(Color{ 20, 25, 35, 255 });
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    float panelW = 460.0f;
    float panelH = 420.0f;
    Rectangle panel = { (sw - panelW) * 0.5f, (sh - panelH) * 0.5f, panelW, panelH };

    DrawRectangleRounded(panel, 0.15f, 6, Color{ 28, 35, 48, 250 });
    DrawRectangleRoundedLines(panel, 0.15f, 6, Color{ 90, 115, 160, 255 });

    const char* title = "GAME SETTINGS";
    int tw = MeasureText(title, 26);
    DrawText(title, (int)(panel.x + (panelW - tw) * 0.5f), (int)(panel.y + 30), 26, Color{ 255, 220, 50, 255 });

    float curY = panel.y + 90;
    float fieldW = 260.0f;
    float fieldX = panel.x + 50.0f;

    // Master Volume
    float curVol = AudioManager::GetVolume();
    if (UI::Slider(Rectangle{ fieldX, curY, fieldW, 18 }, "Master Volume", &curVol, 0.0f, 1.0f, "%.0f%%")) {
        AudioManager::SetVolume(curVol);
        save.volume = curVol;
    }

    // Mouse Sensitivity
    curY += 65;
    if (UI::Slider(Rectangle{ fieldX, curY, fieldW, 18 }, "Camera Sensitivity", &save.sensitivity, 0.05f, 0.50f, "%.2fx")) {
        // Sensitivity updated
    }

    // Invert Y
    curY += 65;
    UI::Toggle(Rectangle{ fieldX, curY, fieldW + 80, 32 }, "Invert Pitch Y", &save.invertY);

    // Fullscreen Toggle
    curY += 50;
    if (UI::Toggle(Rectangle{ fieldX, curY, fieldW + 80, 32 }, "Fullscreen Mode", &save.isFullscreen)) {
        ToggleFullscreen();
    }

    // Back Button
    if (UI::Button(Rectangle{ (sw - 180) * 0.5f, panel.y + panelH - 65.0f, 180, 42 }, "SAVE & BACK", Color{ 45, 140, 85, 255 }, Color{ 65, 175, 110, 255 }, WHITE, 16)) {
        save.Save();
        state = previousState;
    }
}

void Game::UpdatePlaying(float frameDelta) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        state = AppState::PAUSED;
        return;
    }

    // F1 Cheat: Noclip
    if (IsKeyPressed(KEY_F1)) {
        noclipMode = !noclipMode;
        player.velocity = Vector3{ 0.0f, 0.0f, 0.0f };
    }

    // T Cheat: Teleport
    if (IsKeyPressed(KEY_T)) {
        bool teleported = false;
        for (const auto& cp : level.checkpoints) {
            if (!cp.activated) {
                player.position = cp.position;
                player.velocity = Vector3{ 0.0f, 0.0f, 0.0f };
                teleported = true;
                break;
            }
        }
        if (!teleported && level.hasFinish) {
            player.position = level.finish.triggerAABB.GetCenter();
            player.velocity = Vector3{ 0.0f, 0.0f, 0.0f };
        }
    }

    // G: Ghost Clone Toggle
    if (IsKeyPressed(KEY_G)) {
        if (level.ghost.state == GhostRecorder::State::IDLE) {
            level.ghost.StartRecording();
        } else if (level.ghost.state == GhostRecorder::State::RECORDING) {
            level.ghost.StopRecording();
            player.position = level.spawnPos;
            player.velocity = Vector3{ 0.0f, 0.0f, 0.0f };
        } else {
            level.ghost.Reset();
        }
    }

    std::vector<AABB> colliders = level.GetSolidColliders();

    // Camera update with screen shake
    Vector3 camFocus = player.position;
    if (screenShakeTimer > 0.0f) {
        screenShakeTimer -= frameDelta;
        float shakeProgress = screenShakeTimer / 0.35f;
        float shakeMag = 0.25f * shakeProgress;
        auto randOffset = [shakeMag]() {
            return (static_cast<float>(rand() % 2000) / 1000.0f - 1.0f) * shakeMag;
        };
        camFocus.x += randOffset();
        camFocus.y += randOffset();
        camFocus.z += randOffset();
    }

    camera.Update(camFocus, frameDelta, colliders, save.sensitivity, save.invertY);
    particles.Update(frameDelta);

    if (respawnFadeAlpha > 0.0f) {
        respawnFadeAlpha = std::max(0.0f, respawnFadeAlpha - frameDelta * 2.8f);
    }

    if (noclipMode) {
        float flySpeed = 16.0f;
        float yaw = camera.GetYawRadians();
        Vector3 forward = { -sinf(yaw), 0.0f, -cosf(yaw) };
        Vector3 right   = {  cosf(yaw), 0.0f, -sinf(yaw) };

        if (IsKeyDown(KEY_W)) player.position = Vector3Add(player.position, Vector3Scale(forward, flySpeed * frameDelta));
        if (IsKeyDown(KEY_S)) player.position = Vector3Subtract(player.position, Vector3Scale(forward, flySpeed * frameDelta));
        if (IsKeyDown(KEY_D)) player.position = Vector3Add(player.position, Vector3Scale(right, flySpeed * frameDelta));
        if (IsKeyDown(KEY_A)) player.position = Vector3Subtract(player.position, Vector3Scale(right, flySpeed * frameDelta));
        if (IsKeyDown(KEY_SPACE)) player.position.y += flySpeed * frameDelta;
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_C)) player.position.y -= flySpeed * frameDelta;

        level.Update(frameDelta, player);
    } else {
        accumulator += frameDelta;
        while (accumulator >= Constants::PHYSICS_DT) {
            PlayerInput input;
            if (IsKeyDown(KEY_W)) input.moveForward += 1.0f;
            if (IsKeyDown(KEY_S)) input.moveForward -= 1.0f;
            if (IsKeyDown(KEY_D)) input.moveRight   += 1.0f;
            if (IsKeyDown(KEY_A)) input.moveRight   -= 1.0f;
            input.jumpPressed    = IsKeyPressed(KEY_SPACE);
            input.jumpHeld       = IsKeyDown(KEY_SPACE);
            input.respawnPressed = IsKeyPressed(KEY_R);

            level.Update(Constants::PHYSICS_DT, player);
            colliders = level.GetSolidColliders();
            player.UpdatePhysics(Constants::PHYSICS_DT, input, camera.GetYawRadians(), colliders, &particles);

            if (level.deaths > previousDeaths) {
                previousDeaths = level.deaths;
                screenShakeTimer = 0.35f;
                respawnFadeAlpha = 0.85f;
            }

            accumulator -= Constants::PHYSICS_DT;
        }
    }

    if (level.isCompleted) {
        CompleteLevel();
    }
}

void Game::DrawPlaying() {
    std::vector<AABB> colliders = level.GetSolidColliders();

    level.DrawSkybox();
    BeginMode3D(camera.GetRaylibCamera());
    level.Draw();
    player.DrawShadow(colliders);
    player.Draw();
    particles.Draw();
    EndMode3D();

    if (respawnFadeAlpha > 0.001f) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                      Color{ 20, 10, 15, static_cast<unsigned char>(respawnFadeAlpha * 255.0f) });
    }

    UI::DrawHUD(level.name, level.elapsedTime, level.parTime,
               level.GetActivatedCheckpointCount(), level.GetTotalCheckpointCount(), level.deaths);

    // Context hint banner
    if (noclipMode) {
        DrawRectangle(20, 135, 460, 28, Color{ 220, 80, 40, 220 });
        DrawText("[NOCLIP ON] Fly: WASD+Space/Shift | T: Teleport to Checkpoint", 30, 141, 14, WHITE);
    } else if (level.ghost.state == GhostRecorder::State::RECORDING) {
        DrawRectangle(20, 135, 420, 28, Color{ 230, 50, 60, 200 });
        char ghostBuf[64];
        snprintf(ghostBuf, sizeof(ghostBuf), "[REC] RECORDING GHOST (%.1fs / 20s) - Press G to Finish", level.ghost.recordTimer);
        DrawText(ghostBuf, 30, 141, 14, WHITE);
    } else if (level.ghost.state == GhostRecorder::State::PLAYING) {
        DrawRectangle(20, 135, 420, 28, Color{ 40, 160, 220, 200 });
        DrawText("[PLAY] GHOST ACTIVE (Holding buttons) - Press G to Reset", 30, 141, 14, WHITE);
    }

    DrawFPS(GetScreenWidth() - 80, 20);
}

void Game::UpdatePaused() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        state = AppState::PLAYING;
    }
}

void Game::DrawPaused() {
    DrawPlaying(); // Draw frozen game scene behind pause modal

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    DrawRectangle(0, 0, sw, sh, Color{ 10, 15, 25, 175 });

    float cardW = 360.0f;
    float cardH = 340.0f;
    Rectangle card = { (sw - cardW) * 0.5f, (sh - cardH) * 0.5f, cardW, cardH };

    DrawRectangleRounded(card, 0.15f, 6, Color{ 25, 32, 45, 250 });
    DrawRectangleRoundedLines(card, 0.15f, 6, Color{ 90, 120, 170, 255 });

    const char* title = "PAUSED";
    int tw = MeasureText(title, 28);
    DrawText(title, (int)(card.x + (cardW - tw) * 0.5f), (int)(card.y + 30), 28, Color{ 255, 220, 50, 255 });

    float btnY = card.y + 85;
    float btnW = 260;
    float btnH = 44;
    float btnX = card.x + (cardW - btnW) * 0.5f;

    if (UI::Button(Rectangle{ btnX, btnY, btnW, btnH }, "RESUME", Color{ 45, 140, 85, 255 }, Color{ 65, 175, 110, 255 }, WHITE, 18)) {
        state = AppState::PLAYING;
    }
    if (UI::Button(Rectangle{ btnX, btnY + 55, btnW, btnH }, "RESTART LEVEL", Color{ 55, 80, 120, 255 }, Color{ 75, 105, 155, 255 }, WHITE, 18)) {
        LoadLevelByIndex(currentLevelIdx);
        state = AppState::PLAYING;
    }
    if (UI::Button(Rectangle{ btnX, btnY + 110, btnW, btnH }, "SETTINGS", Color{ 70, 80, 95, 255 }, Color{ 95, 105, 125, 255 }, WHITE, 18)) {
        previousState = AppState::PAUSED;
        state = AppState::SETTINGS;
    }
    if (UI::Button(Rectangle{ btnX, btnY + 165, btnW, btnH }, "QUIT TO MENU", Color{ 140, 50, 60, 255 }, Color{ 180, 70, 80, 255 }, WHITE, 18)) {
        state = AppState::MAIN_MENU;
    }
}

void Game::UpdateResults() {}

void Game::DrawResults() {
    DrawPlaying();

    ResultsAction action = UI::DrawResultsScreen(
        GetScreenWidth(), GetScreenHeight(),
        level.elapsedTime, level.parTime, level.deaths, level.starRating
    );

    if (action == ResultsAction::RETRY) {
        LoadLevelByIndex(currentLevelIdx);
        state = AppState::PLAYING;
    } else if (action == ResultsAction::NEXT_LEVEL) {
        int nextIdx = (currentLevelIdx + 1) % LEVEL_PATHS.size();
        LoadLevelByIndex(nextIdx);
        state = AppState::PLAYING;
    } else if (action == ResultsAction::MENU) {
        state = AppState::MAIN_MENU;
    }
}
