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
#include "game.h"
#include "rlgl.h"

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cmath>
#include <cstdio>

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
        player.init(Vec3{ 0.0f, 5.0f, 0.0f });
        std::vector<AABB> colliders = {
            AABB::FromCenterSize(Vector3{ 0.0f, -0.5f, 0.0f }, Vector3{ 20.0f, 1.0f, 20.0f })
        };
        PlayerInput input;
        float simTime = 0.0f;
        while (simTime < 1.0f) {
            player.updatePhysics(dt, input, 0.0f, colliders);
            simTime += dt;
        }
        std::cout << "[SMOKE-TEST] Landed Y: " << player.getPosition().y << ", Grounded: " << player.isGrounded() << std::endl;
        assert(std::abs(player.getPosition().y - 0.0f) < 0.001f);
        assert(player.isGrounded() == true);
        assert(std::abs(player.getVelocity().y) < 0.001f);
    }

    // 2. Seam Test
    {
        std::cout << "[SMOKE-TEST] Testing seam crossing across adjacent boxes..." << std::endl;
        Player player;
        player.init(Vec3{ 0.5f, 0.0f, 0.0f });

        std::vector<AABB> seamBoxes;
        for (int i = 0; i < 10; ++i) {
            seamBoxes.push_back(AABB::FromMinSize(Vector3{ i * 2.0f, -1.0f, -2.0f }, Vector3{ 2.0f, 1.0f, 4.0f }));
        }

        PlayerInput input;
        input.moveRight = 1.0f;
        float simTime = 0.0f;
        while (simTime < 2.0f) {
            player.updatePhysics(dt, input, 0.0f, seamBoxes);
            if (simTime > 0.3f) {
                assert(player.getVelocity().x > 7.0f);
            }
            simTime += dt;
        }
        std::cout << "[SMOKE-TEST] Seam test passed: reached X = " << player.getPosition().x << std::endl;
        assert(player.getPosition().x > 14.0f);
    }

    // 3. High-Speed Wall Test
    {
        std::cout << "[SMOKE-TEST] Testing high-speed wall collision..." << std::endl;
        Player player;
        player.init(Vec3{ 0.0f, 0.0f, 0.0f });

        std::vector<AABB> wallBoxes = {
            AABB::FromMinSize(Vector3{ -5.0f, -1.0f, -5.0f }, Vector3{ 20.0f, 1.0f, 10.0f }),
            AABB::FromMinSize(Vector3{ 8.0f, 0.0f, -5.0f }, Vector3{ 2.0f, 4.0f, 10.0f })
        };

        player.setVelocity(Vec3{ 80.0f, 0.0f, 0.0f });
        PlayerInput input;
        for (int step = 0; step < 60; ++step) {
            player.updatePhysics(dt, input, 0.0f, wallBoxes);
        }
        float expectedX = 8.0f - Constants::HALF_PLAYER_WIDTH;
        std::cout << "[SMOKE-TEST] Wall impact stopped at X = " << player.getPosition().x << " (expected " << expectedX << ")" << std::endl;
        assert(std::abs(player.getPosition().x - expectedX) < 0.001f);
        assert(player.getVelocity().x == 0.0f);
    }

    // 4. Thin Platform Tunneling Test
    {
        std::cout << "[SMOKE-TEST] Testing thin platform tunneling prevention..." << std::endl;
        Player player;
        player.init(Vec3{ 0.0f, 8.0f, 0.0f });
        player.setVelocity(Vec3{ 0.0f, Constants::TERMINAL_VELOCITY, 0.0f });

        std::vector<AABB> thinBoxes = {
            AABB::FromMinSize(Vector3{ -5.0f, -0.05f, -5.0f }, Vector3{ 10.0f, 0.05f, 10.0f })
        };

        PlayerInput input;
        for (int step = 0; step < 60; ++step) {
            player.updatePhysics(dt, input, 0.0f, thinBoxes);
        }
        std::cout << "[SMOKE-TEST] Landed on thin platform at Y = " << player.getPosition().y << ", Grounded: " << player.isGrounded() << std::endl;
        assert(std::abs(player.getPosition().y - 0.0f) < 0.001f);
        assert(player.isGrounded() == true);
    }

    // 5. Coyote Time Test
    {
        std::cout << "[SMOKE-TEST] Testing coyote time..." << std::endl;
        Player player;
        player.init(Vec3{ 0.0f, 0.0f, 0.0f });
        std::vector<AABB> ledgeBoxes = {
            AABB::FromMinSize(Vector3{ -5.0f, -1.0f, -5.0f }, Vector3{ 7.0f, 1.0f, 10.0f })
        };

        PlayerInput input;
        player.updatePhysics(dt, input, 0.0f, ledgeBoxes);
        assert(player.isGrounded());

        input.moveRight = 1.0f;
        int safetyTicks = 0;
        while (player.isGrounded() && safetyTicks < 400) {
            player.updatePhysics(dt, input, 0.0f, ledgeBoxes);
            safetyTicks++;
        }
        assert(!player.isGrounded());
        assert(player.getPosition().x > 2.0f);

        for (int step = 0; step < 4; ++step) {
            player.updatePhysics(dt, input, 0.0f, ledgeBoxes);
        }
        assert(!player.isGrounded());
        assert(player.getCoyoteTimer() > 0.0f);

        input.jumpPressed = true;
        player.updatePhysics(dt, input, 0.0f, ledgeBoxes);
        std::cout << "[SMOKE-TEST] Coyote jump velocity Y = " << player.getVelocity().y << " m/s" << std::endl;
        assert(player.getVelocity().y > 10.0f);
    }

    // 6. Jump Buffering Test
    {
        std::cout << "[SMOKE-TEST] Testing jump buffering..." << std::endl;
        Player player;
        player.init(Vec3{ 0.0f, 0.3f, 0.0f });
        player.setVelocity(Vec3{ 0.0f, -5.0f, 0.0f });

        std::vector<AABB> groundBoxes = {
            AABB::FromMinSize(Vector3{ -5.0f, -1.0f, -5.0f }, Vector3{ 10.0f, 1.0f, 10.0f })
        };

        PlayerInput input;
        input.jumpPressed = true;
        player.updatePhysics(dt, input, 0.0f, groundBoxes);
        input.jumpPressed = false;
        assert(player.getJumpBufferTimer() > 0.0f);

        bool jumpFired = false;
        for (int step = 0; step < 15; ++step) {
            player.updatePhysics(dt, input, 0.0f, groundBoxes);
            if (player.getVelocity().y > 8.0f) {
                jumpFired = true;
                break;
            }
        }
        std::cout << "[SMOKE-TEST] Buffered jump fired: " << (jumpFired ? "YES" : "NO")
                  << " (vel Y = " << player.getVelocity().y << " m/s)" << std::endl;
        assert(jumpFired);
    }

    // 7. Camera Obstacle Pull-in Test
    {
        std::cout << "[SMOKE-TEST] Testing camera obstacle pull-in..." << std::endl;
        OrbitCamera cam;
        Vec3 playerBase = { 0.0f, 0.0f, 0.0f };
        cam.init(playerBase);
        cam.setYaw(0.0f);
        cam.setPitch(0.0f);
        cam.setTargetDistance(10.0f);

        std::vector<AABB> obstacles = {
            AABB::FromCenterSize(Vector3{ 0.0f, Constants::CAM_TARGET_HEIGHT, 3.0f }, Vector3{ 2.0f, 2.0f, 0.4f })
        };

        for (int f = 0; f < 30; ++f) {
            cam.update(playerBase, 0.016f, obstacles);
        }

        std::cout << "[SMOKE-TEST] Camera pulled in to distance: " << cam.getCurrentDistance() << " m (target was 10.0)" << std::endl;
        assert(cam.getCurrentDistance() < 4.0f);
    }

    // 8. Milestone 6: Timed Route Puzzle Verification
    {
        std::cout << "[SMOKE-TEST] Testing Puzzle A: Timed Route (puzzle_timed.lvl)..." << std::endl;
        Level level;
        bool loaded = level.LoadFromFile("assets/levels/puzzle_timed.lvl");
        assert(loaded);
        Player player;
        level.Reset(player);
        assert(!level.getDoor(0)->isOpen());

        player.setPosition(level.getButton(0)->getPosition());
        level.Update(dt, player);
        assert(level.getButton(0)->isActive());
        assert(level.getDoor(0)->isOpen());
    }

    // 9. Milestone 6: Ghost-Clone Puzzle Verification
    {
        std::cout << "[SMOKE-TEST] Testing Puzzle B: Ghost-Clone Puzzle (puzzle_ghost.lvl)..." << std::endl;
        Level level;
        bool loaded = level.LoadFromFile("assets/levels/puzzle_ghost.lvl");
        assert(loaded);
        Player player;
        level.Reset(player);

        level.ghost.StartRecording();
        player.setPosition(level.getButton(0)->getPosition());
        for (int i = 0; i < 120; ++i) level.Update(dt, player);
        level.ghost.StopRecording();

        player.setPosition(level.getSpawnPos());
        for (int i = 0; i < 60; ++i) level.Update(dt, player);
        assert(level.getButton(0)->isPressed());
        assert(level.getDoor(0)->isOpen());
    }

    // 10. Milestone 7: 20-Level Reachability Validator
    {
        std::cout << "[SMOKE-TEST] Running Milestone 7 Reachability Validator on all 20 levels..." << std::endl;
        bool validationPassed = LevelValidator::ValidateAllLevels(ALL_LEVEL_FILES);
        assert(validationPassed);
        std::cout << "[SMOKE-TEST] All 20 levels passed reachability verification!" << std::endl;
    }

    // 11. Milestone 8: Save System & Persistence Test
    {
        std::cout << "[SMOKE-TEST] Testing SaveData file serialization..." << std::endl;
        SaveData testSave;
        testSave.InitDefaults();
        testSave.chosenColor = 3;
        testSave.volume = 0.65f;
        testSave.sensitivity = 0.22f;
        testSave.invertY = true;
        testSave.unlockedWorlds = 3;
        testSave.levels[0].bestTime = 22.4f;
        testSave.levels[0].stars = 3;
        testSave.levels[5].bestTime = 34.1f;
        testSave.levels[5].stars = 2;

        std::string testPath = "test_savegame.txt";
        bool saved = testSave.Save(testPath);
        assert(saved);

        SaveData loadedSave;
        bool loaded = loadedSave.Load(testPath);
        assert(loaded);
        assert(loadedSave.chosenColor == 3);
        assert(std::abs(loadedSave.volume - 0.65f) < 0.01f);
        assert(std::abs(loadedSave.sensitivity - 0.22f) < 0.01f);
        assert(loadedSave.invertY == true);
        assert(loadedSave.unlockedWorlds == 3);
        assert(std::abs(loadedSave.levels[0].bestTime - 22.4f) < 0.01f);
        assert(loadedSave.levels[0].stars == 3);
        assert(std::abs(loadedSave.levels[5].bestTime - 34.1f) < 0.01f);
        assert(loadedSave.levels[5].stars == 2);

        std::remove(testPath.c_str());
        std::cout << "[SMOKE-TEST] SaveData file serialization passed cleanly!" << std::endl;
    }

    std::cout << "[SMOKE-TEST] ALL MILESTONES (M1 through M8) VERIFIED AND PASSED!" << std::endl;
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

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "Blocky Obby Adventure");
    if (!IsWindowReady()) {
        std::cerr << "Window could not be initialized. Running smoke test instead." << std::endl;
        return RunSmokeTest();
    }
    rlSetClipPlanes(Constants::CLIP_NEAR, Constants::CLIP_FAR);
    SetTargetFPS(60);

    AudioManager::Init();

    Game game;
    game.Init();

    while (!WindowShouldClose()) {
        float frameDelta = GetFrameTime();
        if (frameDelta > Constants::MAX_FRAME_DELTA) {
            frameDelta = Constants::MAX_FRAME_DELTA;
        }

        game.Update(frameDelta);

        BeginDrawing();
        game.Draw();
        EndDrawing();
    }

    game.Shutdown();
    AudioManager::Close();
    CloseWindow();
    return 0;
}
