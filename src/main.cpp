#include "raylib.h"
#include "constants.h"
#include "collision.h"
#include "camera.h"
#include "player.h"
#include "level.h"
#include "ui.h"

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

    // 2. Seam Test: Walk across 10 adjacent platforms without snagging
    {
        std::cout << "[SMOKE-TEST] Testing seam crossing across adjacent boxes..." << std::endl;
        Player player;
        player.Init(Vector3{ 0.5f, 0.0f, 0.0f });

        std::vector<AABB> seamBoxes;
        for (int i = 0; i < 10; ++i) {
            seamBoxes.push_back(AABB::FromMinSize(Vector3{ i * 2.0f, -1.0f, -2.0f }, Vector3{ 2.0f, 1.0f, 4.0f }));
        }

        PlayerInput input;
        input.moveRight = 1.0f; // cameraYaw = 0 -> right is +X
        float simTime = 0.0f;
        while (simTime < 2.0f) {
            player.UpdatePhysics(dt, input, 0.0f, seamBoxes);
            if (simTime > 0.3f) {
                assert(player.velocity.x > 7.0f);
            }
            simTime += dt;
        }
        std::cout << "[SMOKE-TEST] Seam test passed: reached X = " << player.position.x << " without snagging." << std::endl;
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
        while (player.isGrounded && safetyTicks < 200) {
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

    // 8. Milestone 3: Level Loading and Flow Verification
    {
        std::cout << "[SMOKE-TEST] Testing Level loading and flow on w1_l1.lvl..." << std::endl;
        Level level;
        bool loaded = level.LoadFromFile("assets/levels/w1_l1.lvl");
        assert(loaded);
        assert(level.boxes.size() >= 25);
        assert(level.checkpoints.size() == 2);
        assert(level.hasFinish == true);
        assert(level.name == "Meadow Steps");

        Player player;
        level.Reset(player);
        assert(player.position.x == level.spawnPos.x && player.position.z == level.spawnPos.z);
        assert(level.GetActivatedCheckpointCount() == 0);
        assert(!level.isCompleted);

        // Teleport player into checkpoint 1
        player.position = level.checkpoints[0].position;
        level.Update(dt, player);
        std::cout << "[SMOKE-TEST] Checkpoint 1 activated: " << level.checkpoints[0].activated << std::endl;
        assert(level.checkpoints[0].activated);
        assert(level.GetActivatedCheckpointCount() == 1);
        assert(player.spawnPoint.x == level.checkpoints[0].position.x);

        // Test respawn at checkpoint 1 on death
        player.position.y = level.killY - 2.0f; // fall below killY
        level.Update(dt, player);
        assert(level.deaths == 1);
        assert(player.position.x == level.checkpoints[0].position.x);
        assert(player.position.z == level.checkpoints[0].position.z);

        // Teleport player into finish zone
        player.position = level.finish.triggerAABB.GetCenter();
        level.Update(dt, player);
        std::cout << "[SMOKE-TEST] Level completed: " << level.isCompleted << ", Star rating: " << level.starRating << std::endl;
        assert(level.isCompleted);
        assert(level.starRating >= 1 && level.starRating <= 3);
    }

    std::cout << "[SMOKE-TEST] All Milestone 3 level loading and flow assertions passed!" << std::endl;
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
    InitWindow(screenWidth, screenHeight, "Roblox Obby Adventure - M3 Level Flow");
    if (!IsWindowReady()) {
        std::cerr << "Window could not be initialized. Running smoke test instead." << std::endl;
        return RunSmokeTest();
    }
    SetTargetFPS(60);

    Level level;
    if (!level.LoadFromFile("assets/levels/w1_l1.lvl")) {
        std::cerr << "Failed to load w1_l1.lvl, running with fallback ground." << std::endl;
    }

    Player player;
    level.Reset(player);

    OrbitCamera camera;
    camera.Init(player.position);

    float accumulator = 0.0f;

    while (!WindowShouldClose()) {
        float frameDelta = GetFrameTime();
        if (frameDelta > Constants::MAX_FRAME_DELTA) {
            frameDelta = Constants::MAX_FRAME_DELTA;
        }

        std::vector<AABB> colliders = level.GetSolidColliders();

        // Only update camera and player physics while actively playing
        camera.Update(player.position, frameDelta, colliders);

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

                player.UpdatePhysics(Constants::PHYSICS_DT, input, camera.GetYawRadians(), colliders);
                level.Update(Constants::PHYSICS_DT, player);

                accumulator -= Constants::PHYSICS_DT;
            }
        }

        // Render Frame
        BeginDrawing();

        // Skybox & 3D world
        level.DrawSkybox();

        BeginMode3D(camera.GetRaylibCamera());
        level.Draw();
        player.Draw();
        EndMode3D();

        // HUD (always drawn during play)
        UI::DrawHUD(level.name, level.elapsedTime, level.parTime,
                   level.GetActivatedCheckpointCount(), level.GetTotalCheckpointCount(), level.deaths);

        // Results screen modal on level completion
        if (level.isCompleted) {
            ResultsAction action = UI::DrawResultsScreen(
                GetScreenWidth(), GetScreenHeight(),
                level.elapsedTime, level.parTime, level.deaths, level.starRating
            );

            if (action == ResultsAction::RETRY) {
                level.Reset(player);
                camera.Init(player.position);
                accumulator = 0.0f;
            } else if (action == ResultsAction::NEXT_LEVEL) {
                // For M3, restarts the course cleanly
                level.Reset(player);
                camera.Init(player.position);
                accumulator = 0.0f;
            }
        }

        DrawFPS(GetScreenWidth() - 90, 20);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
