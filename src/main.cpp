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

    // 8. Milestone 4: Movers and Carrying Physics
    {
        std::cout << "[SMOKE-TEST] Testing mover platform with player carrying..." << std::endl;
        Level level;
        Mover m;
        m.basePos = Vector3{ 0.0f, 0.0f, 0.0f };
        m.size = Vector3{ 4.0f, 1.0f, 4.0f };
        m.delta = Vector3{ 8.0f, 0.0f, 0.0f };
        m.period = 4.0f;
        m.colorIndex = 0;
        m.currentPos = m.basePos;
        m.frameDisplacement = Vector3{ 0.0f, 0.0f, 0.0f };
        level.movers.push_back(m);

        Player player;
        player.Init(Vector3{ 0.0f, 0.5f, 0.0f });

        std::vector<AABB> colliders = level.GetSolidColliders();
        PlayerInput input;
        player.UpdatePhysics(dt, input, 0.0f, colliders);
        assert(player.isGrounded);

        float sim = 0.0f;
        while (sim < 1.0f) {
            level.Update(dt, player);
            colliders = level.GetSolidColliders();
            player.UpdatePhysics(dt, input, 0.0f, colliders);
            sim += dt;
        }

        std::cout << "[SMOKE-TEST] Mover X: " << level.movers[0].currentPos.x
                  << ", Player X: " << player.position.x
                  << ", Grounded: " << player.isGrounded << std::endl;
        assert(player.isGrounded);
        assert(std::abs(player.position.x - level.movers[0].currentPos.x) < 0.05f);
        assert(player.position.x > 3.0f);
    }

    // 9. Milestone 5: Polish, Particle System & Audio Verification
    {
        std::cout << "[SMOKE-TEST] Testing Particle System puffs..." << std::endl;
        ParticleSystem particles;
        particles.SpawnJumpPuff(Vector3{ 0.0f, 0.0f, 0.0f });
        particles.SpawnLandPuff(Vector3{ 0.0f, 0.0f, 0.0f }, 10.0f);
        particles.Update(0.1f);

        // Test volume controls
        AudioManager::SetVolume(0.65f);
        assert(std::abs(AudioManager::GetVolume() - 0.65f) < 0.001f);

        // Test character customization
        Player player;
        player.Init(Vector3{ 0.0f, 0.0f, 0.0f });
        player.colorIndex = 2; // Emerald
        Color c = player.GetShirtColor();
        assert(c.g > 150);
        std::cout << "[SMOKE-TEST] Particle and Character customization checks passed!" << std::endl;
    }

    std::cout << "[SMOKE-TEST] All Milestone 5 tests passed successfully!" << std::endl;
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
    InitWindow(screenWidth, screenHeight, "Roblox Obby Adventure - M5 Polish & Audio");
    if (!IsWindowReady()) {
        std::cerr << "Window could not be initialized. Running smoke test instead." << std::endl;
        return RunSmokeTest();
    }
    SetTargetFPS(60);

    // Initialize procedural audio system
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

    // Polish effects
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

        // Screen Shake calculation
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

        // Respawn fade decay
        if (respawnFadeAlpha > 0.0f) {
            respawnFadeAlpha = std::max(0.0f, respawnFadeAlpha - frameDelta * 2.8f);
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

                // Check death event for screen shake & fade
                if (level.deaths > previousDeaths) {
                    previousDeaths = level.deaths;
                    screenShakeTimer = 0.35f;
                    respawnFadeAlpha = 0.85f;
                }

                accumulator -= Constants::PHYSICS_DT;
            }
        }

        // Quick character shirt color toggle (1-6 keys for testing in M5)
        if (IsKeyPressed(KEY_ONE)) player.colorIndex = 0;
        if (IsKeyPressed(KEY_TWO)) player.colorIndex = 1;
        if (IsKeyPressed(KEY_THREE)) player.colorIndex = 2;
        if (IsKeyPressed(KEY_FOUR)) player.colorIndex = 3;
        if (IsKeyPressed(KEY_FIVE)) player.colorIndex = 4;
        if (IsKeyPressed(KEY_SIX)) player.colorIndex = 5;

        // Render Frame
        BeginDrawing();
        level.DrawSkybox();

        BeginMode3D(camera.GetRaylibCamera());
        level.Draw();
        player.DrawShadow(colliders);
        player.Draw();
        particleSystem.Draw();
        EndMode3D();

        // Respawn flash/fade overlay
        if (respawnFadeAlpha > 0.001f) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                          Color{ 20, 10, 15, static_cast<unsigned char>(respawnFadeAlpha * 255.0f) });
        }

        // HUD & Volume Slider
        UI::DrawHUD(level.name, level.elapsedTime, level.parTime,
                   level.GetActivatedCheckpointCount(), level.GetTotalCheckpointCount(), level.deaths);

        UI::DrawVolumeControl(GetScreenWidth() - 320, 20, 150.0f);

        // Character color hotkeys hint
        DrawText("Shirt Color: Keys 1-6", 20, 140, 15, Color{ 230, 240, 255, 200 });

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
