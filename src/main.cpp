#include "raylib.h"
#include "constants.h"
#include "collision.h"
#include "camera.h"
#include "player.h"

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
            // Consecutive boxes from X = 0 to 20, touching at X = 2, 4, 6, 8, ...
            seamBoxes.push_back(AABB::FromMinSize(Vector3{ i * 2.0f, -1.0f, -2.0f }, Vector3{ 2.0f, 1.0f, 4.0f }));
        }

        PlayerInput input;
        input.moveRight = 1.0f; // cameraYaw = 0 -> right is +X
        float simTime = 0.0f;
        while (simTime < 2.0f) {
            player.UpdatePhysics(dt, input, 0.0f, seamBoxes);
            // Verify speed is maintained smoothly once accelerated
            if (simTime > 0.3f) {
                assert(player.velocity.x > 7.0f);
            }
            simTime += dt;
        }
        std::cout << "[SMOKE-TEST] Seam test passed: reached X = " << player.position.x << " without snagging." << std::endl;
        assert(player.position.x > 14.0f);
    }

    // 3. High-Speed Wall Test: Cannot clip into wall
    {
        std::cout << "[SMOKE-TEST] Testing high-speed wall collision..." << std::endl;
        Player player;
        player.Init(Vector3{ 0.0f, 0.0f, 0.0f });

        // Wall placed at X = 8 to 10
        std::vector<AABB> wallBoxes = {
            AABB::FromMinSize(Vector3{ -5.0f, -1.0f, -5.0f }, Vector3{ 20.0f, 1.0f, 10.0f }), // ground
            AABB::FromMinSize(Vector3{ 8.0f, 0.0f, -5.0f }, Vector3{ 2.0f, 4.0f, 10.0f })     // wall
        };

        player.velocity.x = 80.0f; // Extremely high horizontal velocity
        PlayerInput input;
        for (int step = 0; step < 60; ++step) {
            player.UpdatePhysics(dt, input, 0.0f, wallBoxes);
        }
        float expectedX = 8.0f - Constants::HALF_PLAYER_WIDTH; // 7.5
        std::cout << "[SMOKE-TEST] Wall impact stopped at X = " << player.position.x << " (expected " << expectedX << ")" << std::endl;
        assert(std::abs(player.position.x - expectedX) < 0.001f);
        assert(player.velocity.x == 0.0f);
    }

    // 4. Thin Platform Tunneling Test: Terminal velocity drop onto 0.05m platform
    {
        std::cout << "[SMOKE-TEST] Testing thin platform tunneling prevention..." << std::endl;
        Player player;
        player.Init(Vector3{ 0.0f, 8.0f, 0.0f });
        player.velocity.y = Constants::TERMINAL_VELOCITY; // -40 m/s

        // Thin platform: 0.05m thickness from Y = -0.05 to 0.0
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
        // Platform ends at X = 2.0
        std::vector<AABB> ledgeBoxes = {
            AABB::FromMinSize(Vector3{ -5.0f, -1.0f, -5.0f }, Vector3{ 7.0f, 1.0f, 10.0f }) // [ -5, 2 ]
        };

        PlayerInput input;
        // Step once to establish grounded state
        player.UpdatePhysics(dt, input, 0.0f, ledgeBoxes);
        assert(player.isGrounded);

        input.moveRight = 1.0f; // Walk off ledge towards +X
        // Walk until airborne
        int safetyTicks = 0;
        while (player.isGrounded && safetyTicks < 200) {
            player.UpdatePhysics(dt, input, 0.0f, ledgeBoxes);
            safetyTicks++;
        }
        assert(!player.isGrounded);
        assert(player.position.x > 2.0f);

        // Advance 4 ticks in air (~0.033s, well within 0.1s coyote window)
        for (int step = 0; step < 4; ++step) {
            player.UpdatePhysics(dt, input, 0.0f, ledgeBoxes);
        }
        assert(!player.isGrounded);
        assert(player.coyoteTimer > 0.0f);

        // Press jump while in mid-air
        input.jumpPressed = true;
        player.UpdatePhysics(dt, input, 0.0f, ledgeBoxes);
        std::cout << "[SMOKE-TEST] Coyote jump velocity Y = " << player.velocity.y << " m/s" << std::endl;
        assert(player.velocity.y > 10.0f); // Jump succeeded!
    }

    // 6. Jump Buffering Test
    {
        std::cout << "[SMOKE-TEST] Testing jump buffering..." << std::endl;
        Player player;
        player.Init(Vector3{ 0.0f, 0.3f, 0.0f }); // Falling 0.3m above ground
        player.velocity.y = -5.0f;

        std::vector<AABB> groundBoxes = {
            AABB::FromMinSize(Vector3{ -5.0f, -1.0f, -5.0f }, Vector3{ 10.0f, 1.0f, 10.0f })
        };

        PlayerInput input;
        input.jumpPressed = true; // Buffer jump before landing
        player.UpdatePhysics(dt, input, 0.0f, groundBoxes);
        input.jumpPressed = false;
        assert(player.jumpBufferTimer > 0.0f);

        // Advance until landed and buffered jump executes (should execute in < 15 ticks)
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

        // Obstacle at Z = 3.0 between player and camera eye (which sits at +Z)
        std::vector<AABB> obstacles = {
            AABB::FromCenterSize(Vector3{ 0.0f, Constants::CAM_TARGET_HEIGHT, 3.0f }, Vector3{ 2.0f, 2.0f, 0.4f })
        };

        // Update camera multiple frames to smooth pull-in
        for (int f = 0; f < 30; ++f) {
            cam.Update(playerBase, 0.016f, obstacles);
        }

        std::cout << "[SMOKE-TEST] Camera pulled in to distance: " << cam.currentDistance << " m (target was 10.0)" << std::endl;
        assert(cam.currentDistance < 4.0f);
    }

    std::cout << "[SMOKE-TEST] All Milestone 2 collision and physics assertions passed!" << std::endl;
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
    InitWindow(screenWidth, screenHeight, "Roblox Obby Adventure - M2 Collision");
    if (!IsWindowReady()) {
        std::cerr << "Window could not be initialized. Running smoke test instead." << std::endl;
        return RunSmokeTest();
    }
    SetTargetFPS(60);

    Player player;
    player.Init(Vector3{ 0.0f, 2.0f, 0.0f });

    // Hardcoded set of boxes showcasing seams, steps, walls, and narrow platforms
    std::vector<AABB> colliders = {
        // Main spawn platform
        AABB::FromCenterSize(Vector3{ 0.0f, -0.5f, 0.0f }, Vector3{ 14.0f, 1.0f, 14.0f }),
        // Seam platforms (3 adjacent boxes side by side)
        AABB::FromMinSize(Vector3{ 7.0f, -1.0f, -3.0f }, Vector3{ 4.0f, 1.0f, 6.0f }),
        AABB::FromMinSize(Vector3{ 11.0f, -1.0f, -3.0f }, Vector3{ 4.0f, 1.0f, 6.0f }),
        AABB::FromMinSize(Vector3{ 15.0f, -1.0f, -3.0f }, Vector3{ 4.0f, 1.0f, 6.0f }),
        // Elevated steps
        AABB::FromMinSize(Vector3{ 19.0f, 0.0f, -3.0f }, Vector3{ 3.0f, 1.0f, 6.0f }),
        AABB::FromMinSize(Vector3{ 22.0f, 1.0f, -3.0f }, Vector3{ 3.0f, 1.0f, 6.0f }),
        AABB::FromMinSize(Vector3{ 25.0f, 2.0f, -3.0f }, Vector3{ 4.0f, 1.0f, 6.0f }),
        // High wall for camera pull-in and wall collision testing
        AABB::FromCenterSize(Vector3{ 0.0f, 2.5f, -7.0f }, Vector3{ 14.0f, 5.0f, 1.0f }),
        // Thin floating platform
        AABB::FromCenterSize(Vector3{ 0.0f, 4.0f, 5.0f }, Vector3{ 4.0f, 0.1f, 4.0f })
    };

    OrbitCamera camera;
    camera.Init(player.position);

    float accumulator = 0.0f;

    while (!WindowShouldClose()) {
        float frameDelta = GetFrameTime();
        if (frameDelta > Constants::MAX_FRAME_DELTA) {
            frameDelta = Constants::MAX_FRAME_DELTA;
        }

        camera.Update(player.position, frameDelta, colliders);

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

            if (player.position.y < -15.0f) {
                player.deathCount++;
                player.Respawn();
            }

            accumulator -= Constants::PHYSICS_DT;
        }

        BeginDrawing();
        ClearBackground(Color{ 135, 206, 235, 255 });

        BeginMode3D(camera.GetRaylibCamera());

        // Draw colliders
        for (const auto& box : colliders) {
            Vector3 center = box.GetCenter();
            Vector3 size = box.GetSize();

            Color faceColor = (center.y < 0.0f) ? Color{ 80, 180, 70, 255 } : Color{ 230, 140, 50, 255 };
            Color wireColor = (center.y < 0.0f) ? Color{ 50, 130, 40, 255 } : Color{ 160, 90, 30, 255 };

            DrawCube(center, size.x, size.y, size.z, faceColor);
            DrawCubeWires(center, size.x + 0.01f, size.y + 0.01f, size.z + 0.01f, wireColor);
        }

        DrawGrid(40, 1.0f);
        player.Draw();

        EndMode3D();

        DrawFPS(20, 20);
        DrawText("M2: Collision - Walk across seams, test walls & thin platforms", 20, 50, 18, DARKGRAY);

        char statsBuf[160];
        snprintf(statsBuf, sizeof(statsBuf), "Pos: (%.2f, %.2f, %.2f) | Vel: (%.1f, %.1f, %.1f) | Grounded: %s | CamDist: %.1fm",
                 player.position.x, player.position.y, player.position.z,
                 player.velocity.x, player.velocity.y, player.velocity.z,
                 player.isGrounded ? "YES" : "NO", camera.currentDistance);
        DrawText(statsBuf, 20, 75, 18, DARKBLUE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
