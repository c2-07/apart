#include "raylib.h"
#include "raymath.h"
#include "constants.h"
#include "collision.h"
#include "camera.h"
#include "player.h"
#include "level.h"
#include "ui.h"
#include "audio.h"
#include "particle.h"
#include "validator.h"

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cmath>

static const std::vector<std::string> ALL_LEVEL_FILES = {
    "assets/levels/w1_l1.lvl", "assets/levels/w1_l2.lvl", "assets/levels/w1_l3.lvl", "assets/levels/w1_l4.lvl", "assets/levels/w1_l5.lvl",
    "assets/levels/w2_l1.lvl", "assets/levels/w2_l2.lvl", "assets/levels/w2_l3.lvl", "assets/levels/w2_l4.lvl", "assets/levels/w2_l5.lvl",
    "assets/levels/w3_l1.lvl", "assets/levels/w3_l2.lvl", "assets/levels/w3_l3.lvl", "assets/levels/w3_l4.lvl", "assets/levels/w3_l5.lvl",
    "assets/levels/w4_l1.lvl", "assets/levels/w4_l2.lvl", "assets/levels/w4_l3.lvl", "assets/levels/w4_l4.lvl", "assets/levels/w4_l5.lvl"
};

static int RunSmokeTest() {
    std::cout << "[SMOKE-TEST] Starting headless physics and logic verification..." << std::endl;
    const float dt = Constants::PHYSICS_DT;

    // 1. Basic Gravity and Landing
    {
        Player player;
        player.Init(Vector3{ 0.0f, 5.0f, 0.0f });
        std::vector<AABB> colliders = {
            AABB::FromCenterSize(Vector3{ 0.0f, -0.5f, 0.0f }, Vector3{ 20.0f, 1.0f, 20.0f })
        };
        PlayerInput input;
        float simTime = 0.0f;
        while (simTime < 1.0f) {
            player.UpdatePhysics(dt, input, 0.0f, colliders);
            simTime += dt;
        }
        std::cout << "[SMOKE-TEST] Landed Y: " << player.position.y << ", Grounded: " << player.isGrounded << std::endl;
        assert(std::abs(player.position.y - 0.0f) < 0.001f);
        assert(player.isGrounded == true);
        assert(std::abs(player.velocity.y) < 0.001f);
    }

    // 2. Seam Test
    {
        std::cout << "[SMOKE-TEST] Testing seam crossing across adjacent boxes..." << std::endl;
        Player player;
        player.Init(Vector3{ 0.5f, 0.0f, 0.0f });

        std::vector<AABB> seamBoxes;
        for (int i = 0; i < 10; ++i) {
            seamBoxes.push_back(AABB::FromMinSize(Vector3{ i * 2.0f, -1.0f, -2.0f }, Vector3{ 2.0f, 1.0f, 4.0f }));
        }

        PlayerInput input;
        input.moveRight = 1.0f;
        float simTime = 0.0f;
        while (simTime < 2.0f) {
            player.UpdatePhysics(dt, input, 0.0f, seamBoxes);
            if (simTime > 0.3f) {
                assert(player.velocity.x > 7.0f);
            }
            simTime += dt;
        }
        std::cout << "[SMOKE-TEST] Seam test passed: reached X = " << player.position.x << std::endl;
        assert(player.position.x > 14.0f);
    }

    // 3. High-Speed Wall Test
    {
        std::cout << "[SMOKE-TEST] Testing high-speed wall collision..." << std::endl;
        Player player;
        player.Init(Vector3{ 0.0f, 0.0f, 0.0f });

        std::vector<AABB> wallBoxes = {
            AABB::FromMinSize(Vector3{ -5.0f, -1.0f, -5.0f }, Vector3{ 20.0f, 1.0f, 10.0f }),
            AABB::FromMinSize(Vector3{ 8.0f, 0.0f, -5.0f }, Vector3{ 2.0f, 4.0f, 10.0f })
        };

        player.velocity.x = 80.0f;
        PlayerInput input;
        for (int step = 0; step < 60; ++step) {
            player.UpdatePhysics(dt, input, 0.0f, wallBoxes);
        }
        float expectedX = 8.0f - Constants::HALF_PLAYER_WIDTH;
        std::cout << "[SMOKE-TEST] Wall impact stopped at X = " << player.position.x << " (expected " << expectedX << ")" << std::endl;
        assert(std::abs(player.position.x - expectedX) < 0.001f);
        assert(player.velocity.x == 0.0f);
    }

    // 4. Thin Platform Tunneling Test
    {
        std::cout << "[SMOKE-TEST] Testing thin platform tunneling prevention..." << std::endl;
        Player player;
        player.Init(Vector3{ 0.0f, 8.0f, 0.0f });
        player.velocity.y = Constants::TERMINAL_VELOCITY;

        std::vector<AABB> thinBoxes = {
            AABB::FromMinSize(Vector3{ -5.0f, -0.05f, -5.0f }, Vector3{ 10.0f, 0.05f, 10.0f })
        };

        PlayerInput input;
        for (int step = 0; step < 60; ++step) {
            player.UpdatePhysics(dt, input, 0.0f, thinBoxes);
        }
        std::cout << "[SMOKE-TEST] Landed on thin platform at Y = " << player.position.y << ", Grounded: " << player.isGrounded << std::endl;
        assert(std::abs(player.position.y - 0.0f) < 0.001f);
        assert(player.isGrounded == true);
    }

    // 5. Coyote Time Test
    {
        std::cout << "[SMOKE-TEST] Testing coyote time..." << std::endl;
        Player player;
        player.Init(Vector3{ 0.0f, 0.0f, 0.0f });
        std::vector<AABB> ledgeBoxes = {
            AABB::FromMinSize(Vector3{ -5.0f, -1.0f, -5.0f }, Vector3{ 7.0f, 1.0f, 10.0f })
        };

        PlayerInput input;
        player.UpdatePhysics(dt, input, 0.0f, ledgeBoxes);
        assert(player.isGrounded);

        input.moveRight = 1.0f;
        int safetyTicks = 0;
        while (player.isGrounded && safetyTicks < 400) {
            player.UpdatePhysics(dt, input, 0.0f, ledgeBoxes);
            safetyTicks++;
        }
        assert(!player.isGrounded);
        assert(player.position.x > 2.0f);

        for (int step = 0; step < 4; ++step) {
            player.UpdatePhysics(dt, input, 0.0f, ledgeBoxes);
        }
        assert(!player.isGrounded);
        assert(player.coyoteTimer > 0.0f);

        input.jumpPressed = true;
        player.UpdatePhysics(dt, input, 0.0f, ledgeBoxes);
        std::cout << "[SMOKE-TEST] Coyote jump velocity Y = " << player.velocity.y << " m/s" << std::endl;
        assert(player.velocity.y > 10.0f);
    }

    // 6. Jump Buffering Test
    {
        std::cout << "[SMOKE-TEST] Testing jump buffering..." << std::endl;
        Player player;
        player.Init(Vector3{ 0.0f, 0.3f, 0.0f });
        player.velocity.y = -5.0f;

        std::vector<AABB> groundBoxes = {
            AABB::FromMinSize(Vector3{ -5.0f, -1.0f, -5.0f }, Vector3{ 10.0f, 1.0f, 10.0f })
        };

        PlayerInput input;
        input.jumpPressed = true;
        player.UpdatePhysics(dt, input, 0.0f, groundBoxes);
        input.jumpPressed = false;
        assert(player.jumpBufferTimer > 0.0f);

        bool jumpFired = false;
        for (int step = 0; step < 15; ++step) {
            player.UpdatePhysics(dt, input, 0.0f, groundBoxes);
            if (player.velocity.y > 8.0f) {
                jumpFired = true;
                break;
            }
        }
        std::cout << "[SMOKE-TEST] Buffered jump fired: " << (jumpFired ? "YES" : "NO")
                  << " (vel Y = " << player.velocity.y << " m/s)" << std::endl;
        assert(jumpFired);
    }

    // 7. Camera Obstacle Pull-in Test
    {
        std::cout << "[SMOKE-TEST] Testing camera obstacle pull-in..." << std::endl;
        OrbitCamera cam;
        Vector3 playerBase = { 0.0f, 0.0f, 0.0f };
        cam.Init(playerBase);
        cam.yaw = 0.0f;
        cam.pitch = 0.0f;
        cam.targetDistance = 10.0f;

        std::vector<AABB> obstacles = {
            AABB::FromCenterSize(Vector3{ 0.0f, Constants::CAM_TARGET_HEIGHT, 3.0f }, Vector3{ 2.0f, 2.0f, 0.4f })
        };

        for (int f = 0; f < 30; ++f) {
            cam.Update(playerBase, 0.016f, obstacles);
        }

        std::cout << "[SMOKE-TEST] Camera pulled in to distance: " << cam.currentDistance << " m (target was 10.0)" << std::endl;
        assert(cam.currentDistance < 4.0f);
    }

    // 8. Milestone 6: Timed Route Puzzle Verification (puzzle_timed.lvl)
    {
        std::cout << "[SMOKE-TEST] Testing Puzzle A: Timed Route (puzzle_timed.lvl)..." << std::endl;
        Level level;
        bool loaded = level.LoadFromFile("assets/levels/puzzle_timed.lvl");
        assert(loaded);
        Player player;
        level.Reset(player);
        assert(!level.doors[0].isOpen);

        player.position = level.buttons[0].position;
        level.Update(dt, player);
        assert(level.buttons[0].isActive);
        assert(level.doors[0].isOpen);
    }

    // 9. Milestone 6: Ghost-Clone Puzzle Verification (puzzle_ghost.lvl)
    {
        std::cout << "[SMOKE-TEST] Testing Puzzle B: Ghost-Clone Puzzle (puzzle_ghost.lvl)..." << std::endl;
        Level level;
        bool loaded = level.LoadFromFile("assets/levels/puzzle_ghost.lvl");
        assert(loaded);
        Player player;
        level.Reset(player);

        level.ghost.StartRecording();
        player.position = level.buttons[0].position;
        for (int i = 0; i < 120; ++i) level.Update(dt, player);
        level.ghost.StopRecording();

        player.position = level.spawnPos;
        for (int i = 0; i < 60; ++i) level.Update(dt, player);
        assert(level.buttons[0].isPressed);
        assert(level.doors[0].isOpen);
    }

    // 10. Milestone 7: Level Validator 20-Level Reachability Suite
    {
        std::cout << "[SMOKE-TEST] Running Milestone 7 Reachability Validator on all 20 levels..." << std::endl;
        bool validationPassed = LevelValidator::ValidateAllLevels(ALL_LEVEL_FILES);
        assert(validationPassed);
        std::cout << "[SMOKE-TEST] All 20 levels passed reachability verification!" << std::endl;
    }

    std::cout << "[SMOKE-TEST] All Milestone 1 through 7 assertions passed successfully!" << std::endl;
    return 0;
}

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--smoke-test") {
            return RunSmokeTest();
        }
        if (arg == "--validate-levels") {
            bool ok = LevelValidator::ValidateAllLevels(ALL_LEVEL_FILES);
            return ok ? 0 : 1;
        }
    }

    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Roblox Obby Adventure - M7 Worlds & Levels");
    if (!IsWindowReady()) {
        std::cerr << "Window could not be initialized. Running smoke test instead." << std::endl;
        return RunSmokeTest();
    }
    SetTargetFPS(60);

    AudioManager::Init();

    int currentLevelIndex = 0;
    Level level;
    if (!level.LoadFromFile(ALL_LEVEL_FILES[currentLevelIndex])) {
        std::cerr << "Failed to load " << ALL_LEVEL_FILES[currentLevelIndex] << std::endl;
    }

    Player player;
    level.Reset(player);

    OrbitCamera camera;
    camera.Init(player.position);

    ParticleSystem particleSystem;

    // Debug cheat modes
    bool noclipMode = false;

    float screenShakeTimer = 0.0f;
    float respawnFadeAlpha = 0.0f;
    int previousDeaths = 0;
    float accumulator = 0.0f;

    while (!WindowShouldClose()) {
        float frameDelta = GetFrameTime();
        if (frameDelta > Constants::MAX_FRAME_DELTA) {
            frameDelta = Constants::MAX_FRAME_DELTA;
        }

        std::vector<AABB> colliders = level.GetSolidColliders();

        // F1: Toggle Noclip Fly Cheat
        if (IsKeyPressed(KEY_F1)) {
            noclipMode = !noclipMode;
            player.velocity = Vector3{ 0.0f, 0.0f, 0.0f };
        }

        // T: Teleport to next checkpoint or finish (Debug cheat)
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

        // Ghost Clone toggle with key G
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

        // Level selection bracket keys ([ and ] or PageUp/PageDown)
        if (IsKeyPressed(KEY_RIGHT_BRACKET) || IsKeyPressed(KEY_PAGE_DOWN)) {
            currentLevelIndex = (currentLevelIndex + 1) % ALL_LEVEL_FILES.size();
            level.LoadFromFile(ALL_LEVEL_FILES[currentLevelIndex]);
            level.Reset(player);
            camera.Init(player.position);
            particleSystem.Clear();
            previousDeaths = 0;
        }
        if (IsKeyPressed(KEY_LEFT_BRACKET) || IsKeyPressed(KEY_PAGE_UP)) {
            currentLevelIndex = (currentLevelIndex - 1 + static_cast<int>(ALL_LEVEL_FILES.size())) % ALL_LEVEL_FILES.size();
            level.LoadFromFile(ALL_LEVEL_FILES[currentLevelIndex]);
            level.Reset(player);
            camera.Init(player.position);
            particleSystem.Clear();
            previousDeaths = 0;
        }

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

        camera.Update(camFocus, frameDelta, colliders);
        particleSystem.Update(frameDelta);

        if (respawnFadeAlpha > 0.0f) {
            respawnFadeAlpha = std::max(0.0f, respawnFadeAlpha - frameDelta * 2.8f);
        }

        if (!level.isCompleted) {
            if (noclipMode) {
                // Free flying in noclip mode
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
                    player.UpdatePhysics(Constants::PHYSICS_DT, input, camera.GetYawRadians(), colliders, &particleSystem);

                    if (level.deaths > previousDeaths) {
                        previousDeaths = level.deaths;
                        screenShakeTimer = 0.35f;
                        respawnFadeAlpha = 0.85f;
                    }

                    accumulator -= Constants::PHYSICS_DT;
                }
            }
        }

        // Shirt Color hotkeys (1-6)
        if (IsKeyPressed(KEY_ONE)) player.colorIndex = 0;
        if (IsKeyPressed(KEY_TWO)) player.colorIndex = 1;
        if (IsKeyPressed(KEY_THREE)) player.colorIndex = 2;
        if (IsKeyPressed(KEY_FOUR)) player.colorIndex = 3;
        if (IsKeyPressed(KEY_FIVE)) player.colorIndex = 4;
        if (IsKeyPressed(KEY_SIX)) player.colorIndex = 5;

        BeginDrawing();
        level.DrawSkybox();

        BeginMode3D(camera.GetRaylibCamera());
        level.Draw();
        player.DrawShadow(colliders);
        player.Draw();
        particleSystem.Draw();
        EndMode3D();

        if (respawnFadeAlpha > 0.001f) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                          Color{ 20, 10, 15, static_cast<unsigned char>(respawnFadeAlpha * 255.0f) });
        }

        UI::DrawHUD(level.name, level.elapsedTime, level.parTime,
                   level.GetActivatedCheckpointCount(), level.GetTotalCheckpointCount(), level.deaths);

        UI::DrawVolumeControl(GetScreenWidth() - 320, 20, 150.0f);

        // Debug & Navigation HUD
        if (noclipMode) {
            DrawRectangle(20, 135, 460, 28, Color{ 220, 80, 40, 220 });
            DrawText("[NOCLIP ON] Fly: WASD + Space/Shift | T: Teleport to Checkpoint", 30, 141, 14, WHITE);
        } else if (level.ghost.state == GhostRecorder::State::RECORDING) {
            DrawRectangle(20, 135, 420, 28, Color{ 230, 50, 60, 200 });
            char ghostBuf[64];
            snprintf(ghostBuf, sizeof(ghostBuf), "[REC] RECORDING GHOST (%.1fs / 20s) - Press G to Finish", level.ghost.recordTimer);
            DrawText(ghostBuf, 30, 141, 14, WHITE);
        } else if (level.ghost.state == GhostRecorder::State::PLAYING) {
            DrawRectangle(20, 135, 420, 28, Color{ 40, 160, 220, 200 });
            DrawText("[PLAY] GHOST ACTIVE (Holding buttons) - Press G to Reset", 30, 141, 14, WHITE);
        } else {
            char navBuf[128];
            snprintf(navBuf, sizeof(navBuf), "Level [%d/20]: [ / ] Switch | F1: Noclip Cheat | T: Teleport | G: Ghost", currentLevelIndex + 1);
            DrawText(navBuf, 20, 140, 14, Color{ 230, 240, 255, 210 });
        }

        if (level.isCompleted) {
            ResultsAction action = UI::DrawResultsScreen(
                GetScreenWidth(), GetScreenHeight(),
                level.elapsedTime, level.parTime, level.deaths, level.starRating
            );

            if (action == ResultsAction::RETRY) {
                level.Reset(player);
                camera.Init(player.position);
                particleSystem.Clear();
                previousDeaths = level.deaths;
                accumulator = 0.0f;
            } else if (action == ResultsAction::NEXT_LEVEL) {
                currentLevelIndex = (currentLevelIndex + 1) % ALL_LEVEL_FILES.size();
                level.LoadFromFile(ALL_LEVEL_FILES[currentLevelIndex]);
                level.Reset(player);
                camera.Init(player.position);
                particleSystem.Clear();
                previousDeaths = 0;
                accumulator = 0.0f;
            }
        }

        DrawFPS(GetScreenWidth() - 80, 55);
        EndDrawing();
    }

    AudioManager::Close();
    CloseWindow();
    return 0;
}
