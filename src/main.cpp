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

    // 8. Milestone 4: Movers and Carrying Physics Test
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
        player.Init(Vector3{ 0.0f, 0.5f, 0.0f }); // On top of mover (top surface at Y=0.5)

        // Settle player on mover
        std::vector<AABB> colliders = level.GetSolidColliders();
        PlayerInput input;
        player.UpdatePhysics(dt, input, 0.0f, colliders);
        assert(player.isGrounded);

        // Advance 1.0s (mover travels ~4m in X)
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
        // Verify player remained on top of platform and followed its displacement with no jitter
        assert(player.isGrounded);
        assert(std::abs(player.position.x - level.movers[0].currentPos.x) < 0.05f);
        assert(player.position.x > 3.0f);
    }

    // 9. Milestone 4: Spinner Hazard Test
    {
        std::cout << "[SMOKE-TEST] Testing spinner hazard collision..." << std::endl;
        Level level;
        level.spawnPos = Vector3{ 0.0f, 0.0f, -10.0f };
        Spinner s;
        s.center = Vector3{ 0.0f, 1.0f, 0.0f };
        s.length = 6.0f;
        s.height = 1.0f;
        s.speed = 4.0f;
        s.currentAngle = 0.0f;
        level.spinners.push_back(s);

        Player player;
        level.Reset(player);
        // Place player directly in the path of the rotating bar
        player.position = Vector3{ 2.0f, 0.0f, 0.0f };
        assert(level.deaths == 0);

        // Advance ticks until spinner sweeps through player
        for (int step = 0; step < 60; ++step) {
            level.Update(dt, player);
            if (level.deaths > 0) break;
        }

        std::cout << "[SMOKE-TEST] Spinner collision caused death: " << (level.deaths > 0 ? "YES" : "NO") << std::endl;
        assert(level.deaths > 0);
        assert(player.position.z == level.spawnPos.z); // Respawned at spawn
    }

    // 10. Milestone 4: Crumble Tile Test
    {
        std::cout << "[SMOKE-TEST] Testing crumble tile states (STABLE -> CRUMBLING -> FALLEN -> STABLE)..." << std::endl;
        Level level;
        CrumbleTile c;
        c.basePos = Vector3{ 0.0f, 0.0f, 0.0f };
        c.size = Vector3{ 3.0f, 1.0f, 3.0f };
        c.delay = 0.2f;
        c.respawnTime = 0.4f;
        c.state = CrumbleState::STABLE;
        c.timer = 0.0f;
        c.currentPos = c.basePos;
        level.crumbleTiles.push_back(c);

        Player player;
        player.Init(Vector3{ 0.0f, 0.5f, 0.0f });

        PlayerInput input;
        std::vector<AABB> colliders = level.GetSolidColliders();
        player.UpdatePhysics(dt, input, 0.0f, colliders);
        assert(player.isGrounded);

        // Step 1: Landing triggers CRUMBLING
        level.Update(dt, player);
        assert(level.crumbleTiles[0].state == CrumbleState::CRUMBLING);

        // Step 2: After delay (0.2s), transitions to FALLEN
        float sim = 0.0f;
        while (sim < 0.25f) {
            level.Update(dt, player);
            sim += dt;
        }
        assert(level.crumbleTiles[0].state == CrumbleState::FALLEN);
        assert(!level.crumbleTiles[0].IsSolid());

        // Move player away while tile is fallen
        player.position = Vector3{ 10.0f, 0.0f, 0.0f };

        // Step 3: After respawnTime (0.4s), transitions back to STABLE
        sim = 0.0f;
        while (sim < 0.45f) {
            level.Update(dt, player);
            sim += dt;
        }
        assert(level.crumbleTiles[0].state == CrumbleState::STABLE);
        assert(level.crumbleTiles[0].IsSolid());
        std::cout << "[SMOKE-TEST] Crumble tile full cycle passed!" << std::endl;
    }

    // 11. Milestone 4: Spikes & Bounce Pads
    {
        std::cout << "[SMOKE-TEST] Testing spikes and bounce pads..." << std::endl;
        Level level;
        level.spawnPos = Vector3{ 0.0f, 0.0f, -10.0f };

        // Spikes at (5, 0, 0)
        Spikes sp;
        sp.center = Vector3{ 5.0f, 0.5f, 0.0f };
        sp.size = Vector3{ 2.0f, 1.0f, 2.0f };
        sp.aabb = AABB::FromCenterSize(sp.center, sp.size);
        level.spikesList.push_back(sp);

        // Bounce pad at (10, 0, 0)
        BouncePad bp;
        bp.center = Vector3{ 10.0f, 0.25f, 0.0f };
        bp.size = Vector3{ 2.0f, 0.5f, 2.0f };
        bp.strength = 20.0f;
        bp.aabb = AABB::FromCenterSize(bp.center, bp.size);
        level.bouncePads.push_back(bp);

        Player player;
        level.Reset(player);

        // Test Spikes
        player.position = Vector3{ 5.0f, 0.5f, 0.0f };
        level.Update(dt, player);
        assert(level.deaths == 1);
        assert(player.position.z == level.spawnPos.z);

        // Test Bounce Pad
        player.position = Vector3{ 10.0f, 0.5f, 0.0f };
        player.velocity.y = -2.0f;
        level.Update(dt, player);
        std::cout << "[SMOKE-TEST] Bounce pad launched player to vel Y: " << player.velocity.y << " m/s" << std::endl;
        assert(player.velocity.y == 20.0f);
    }

    std::cout << "[SMOKE-TEST] All Milestone 4 moving entities & hazard assertions passed!" << std::endl;
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
    InitWindow(screenWidth, screenHeight, "Roblox Obby Adventure - M4 Hazards & Movers");
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

                level.Update(Constants::PHYSICS_DT, player);
                colliders = level.GetSolidColliders();
                player.UpdatePhysics(Constants::PHYSICS_DT, input, camera.GetYawRadians(), colliders);

                accumulator -= Constants::PHYSICS_DT;
            }
        }

        BeginDrawing();
        level.DrawSkybox();

        BeginMode3D(camera.GetRaylibCamera());
        level.Draw();
        player.Draw();
        EndMode3D();

        UI::DrawHUD(level.name, level.elapsedTime, level.parTime,
                   level.GetActivatedCheckpointCount(), level.GetTotalCheckpointCount(), level.deaths);

        if (level.isCompleted) {
            ResultsAction action = UI::DrawResultsScreen(
                GetScreenWidth(), GetScreenHeight(),
                level.elapsedTime, level.parTime, level.deaths, level.starRating
            );

            if (action == ResultsAction::RETRY || action == ResultsAction::NEXT_LEVEL) {
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
