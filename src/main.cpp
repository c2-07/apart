#include "raylib.h"
#include "constants.h"
#include "collision.h"
#include "camera.h"
#include "player.h"
#include "level.h"
#include "ui.h"
#include "audio.h"
#include "particle.h"

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cmath>

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
        assert(level.buttons.size() == 1);
        assert(level.doors.size() == 1);

        Player player;
        level.Reset(player);

        // Before stepping on button: Door 1 is closed and solid
        std::vector<AABB> colliders = level.GetSolidColliders();
        bool doorFoundInSolids = false;
        for (const auto& c : colliders) {
            if (std::abs(c.min.z - level.doors[0].aabb.min.z) < 0.01f) {
                doorFoundInSolids = true;
                break;
            }
        }
        assert(doorFoundInSolids);
        assert(!level.doors[0].isOpen);

        // Step on Button 1
        player.position = level.buttons[0].position;
        level.Update(dt, player);
        assert(level.buttons[0].isActive);
        assert(level.doors[0].isOpen);

        // Door 1 is now open -> not in solid colliders
        colliders = level.GetSolidColliders();
        doorFoundInSolids = false;
        for (const auto& c : colliders) {
            if (std::abs(c.min.z - level.doors[0].aabb.min.z) < 0.01f) {
                doorFoundInSolids = true;
                break;
            }
        }
        assert(!doorFoundInSolids); // Door is passable!

        // Move player away from button
        player.position = Vector3{ 0.0f, 2.0f, -20.0f };
        level.Update(dt, player);
        assert(level.buttons[0].isActive); // Still active due to timer
        assert(level.buttons[0].currentTimer > 4.5f);

        // Run past timer (5.0s) -> Door closes
        float sim = 0.0f;
        while (sim < 5.2f) {
            level.Update(dt, player);
            sim += dt;
        }
        assert(!level.buttons[0].isActive);
        assert(!level.doors[0].isOpen); // Door shut!
        std::cout << "[SMOKE-TEST] Puzzle A Timed Route passed!" << std::endl;
    }

    // 9. Milestone 6: Ghost-Clone Puzzle Verification (puzzle_ghost.lvl)
    {
        std::cout << "[SMOKE-TEST] Testing Puzzle B: Ghost-Clone Puzzle (puzzle_ghost.lvl)..." << std::endl;
        Level level;
        bool loaded = level.LoadFromFile("assets/levels/puzzle_ghost.lvl");
        assert(loaded);
        assert(level.buttons.size() == 1);
        assert(level.doors.size() == 1);

        Player player;
        level.Reset(player);
        assert(!level.doors[0].isOpen);

        // Step 1: Start recording ghost clone
        level.ghost.StartRecording();
        assert(level.ghost.state == GhostRecorder::State::RECORDING);

        // Player runs to Button 2 at (-14, 0, -12)
        player.position = level.buttons[0].position;
        for (int i = 0; i < 120; ++i) { // 1 second of standing on button
            level.Update(dt, player);
        }
        assert(level.buttons[0].isActive);

        // Step 2: Stop recording, ghost playback begins
        level.ghost.StopRecording();
        assert(level.ghost.state == GhostRecorder::State::PLAYING);

        // Real player returns to spawn
        player.position = level.spawnPos;

        // Step 3: Advance 0.5s into ghost playback
        for (int i = 0; i < 60; ++i) {
            level.Update(dt, player);
        }

        // Ghost clone is now standing on Button 2 -> opens Door 2!
        std::cout << "[SMOKE-TEST] Ghost active: " << level.ghost.HasActiveGhost()
                  << ", Button 2 pressed: " << level.buttons[0].isPressed
                  << ", Door 2 open: " << level.doors[0].isOpen << std::endl;
        assert(level.buttons[0].isPressed);
        assert(level.doors[0].isOpen);

        // Real player runs through Door 2 to Finish Zone
        player.position = level.finish.triggerAABB.GetCenter();
        level.Update(dt, player);
        assert(level.isCompleted);
        std::cout << "[SMOKE-TEST] Puzzle B Ghost Duo passed successfully!" << std::endl;
    }

    std::cout << "[SMOKE-TEST] All Milestone 6 solo cooperative puzzle assertions passed!" << std::endl;
    return 0;
}

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--smoke-test") {
            return RunSmokeTest();
        }
    }

    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Roblox Obby Adventure - M6 Co-op Puzzles");
    if (!IsWindowReady()) {
        std::cerr << "Window could not be initialized. Running smoke test instead." << std::endl;
        return RunSmokeTest();
    }
    SetTargetFPS(60);

    AudioManager::Init();

    Level level;
    if (!level.LoadFromFile("assets/levels/w1_l1.lvl")) {
        std::cerr << "Failed to load w1_l1.lvl, running with fallback ground." << std::endl;
    }

    Player player;
    level.Reset(player);

    OrbitCamera camera;
    camera.Init(player.position);

    ParticleSystem particleSystem;

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

        // Level selection hotkeys for quick testing of puzzles in M6
        if (IsKeyPressed(KEY_F2)) {
            level.LoadFromFile("assets/levels/puzzle_timed.lvl");
            level.Reset(player);
            camera.Init(player.position);
            particleSystem.Clear();
        }
        if (IsKeyPressed(KEY_F3)) {
            level.LoadFromFile("assets/levels/puzzle_ghost.lvl");
            level.Reset(player);
            camera.Init(player.position);
            particleSystem.Clear();
        }
        if (IsKeyPressed(KEY_F4)) {
            level.LoadFromFile("assets/levels/w1_l1.lvl");
            level.Reset(player);
            camera.Init(player.position);
            particleSystem.Clear();
        }

        if (!level.isCompleted) {
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

        // Ghost Status Banner
        if (level.ghost.state == GhostRecorder::State::RECORDING) {
            DrawRectangle(20, 135, 360, 28, Color{ 230, 50, 60, 200 });
            char ghostBuf[64];
            snprintf(ghostBuf, sizeof(ghostBuf), "[REC] RECORDING GHOST (%.1fs / 20s) - Press G to Finish", level.ghost.recordTimer);
            DrawText(ghostBuf, 30, 141, 14, WHITE);
        } else if (level.ghost.state == GhostRecorder::State::PLAYING) {
            DrawRectangle(20, 135, 360, 28, Color{ 40, 160, 220, 200 });
            DrawText("[PLAY] GHOST ACTIVE (Holding buttons) - Press G to Reset", 30, 141, 14, WHITE);
        } else {
            DrawText("G: Record Ghost Clone | F2: Timed Puzzle | F3: Ghost Puzzle | F4: W1L1", 20, 140, 14, Color{ 230, 240, 255, 210 });
        }

        if (level.isCompleted) {
            ResultsAction action = UI::DrawResultsScreen(
                GetScreenWidth(), GetScreenHeight(),
                level.elapsedTime, level.parTime, level.deaths, level.starRating
            );

            if (action == ResultsAction::RETRY || action == ResultsAction::NEXT_LEVEL) {
                level.Reset(player);
                camera.Init(player.position);
                particleSystem.Clear();
                previousDeaths = level.deaths;
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
