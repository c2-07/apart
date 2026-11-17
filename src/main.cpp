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

    Player player;
    player.Init(Vector3{ 0.0f, 5.0f, 0.0f });

    // Ground platform: centered at (0, -0.5, 0), size (20, 1, 20), so top surface is at Y = 0.0
    std::vector<AABB> colliders = {
        AABB::FromCenterSize(Vector3{ 0.0f, -0.5f, 0.0f }, Vector3{ 20.0f, 1.0f, 20.0f })
    };

    PlayerInput input;
    const float dt = Constants::PHYSICS_DT;

    // 1. Test Gravity and Landing
    float simTime = 0.0f;
    while (simTime < 1.0f) {
        player.UpdatePhysics(dt, input, 0.0f, colliders);
        simTime += dt;
    }

    std::cout << "[SMOKE-TEST] Landed Y: " << player.position.y << ", Grounded: " << player.isGrounded << std::endl;
    assert(std::abs(player.position.y - 0.0f) < 0.001f);
    assert(player.isGrounded == true);
    assert(std::abs(player.velocity.y) < 0.001f);

    // 2. Test Movement Acceleration on Ground
    input.moveForward = 1.0f; // pressing W
    simTime = 0.0f;
    while (simTime < 0.5f) {
        player.UpdatePhysics(dt, input, 0.0f, colliders);
        simTime += dt;
    }
    // cameraYaw = 0 -> forward is -Z
    float forwardSpeed = -player.velocity.z;
    std::cout << "[SMOKE-TEST] Forward speed after 0.5s: " << forwardSpeed << " m/s (target " << Constants::RUN_SPEED << ")" << std::endl;
    assert(forwardSpeed > 7.9f && forwardSpeed <= Constants::RUN_SPEED + 0.01f);

    // 3. Test Jump Height
    input.moveForward = 0.0f;
    input.jumpPressed = true;
    player.UpdatePhysics(dt, input, 0.0f, colliders);
    input.jumpPressed = false;

    float maxHeight = player.position.y;
    simTime = 0.0f;
    while (simTime < 1.0f) {
        player.UpdatePhysics(dt, input, 0.0f, colliders);
        if (player.position.y > maxHeight) {
            maxHeight = player.position.y;
        }
        simTime += dt;
    }
    std::cout << "[SMOKE-TEST] Peak jump height: " << maxHeight << " m (expected ~" << Constants::JUMP_HEIGHT << ")" << std::endl;
    assert(std::abs(maxHeight - Constants::JUMP_HEIGHT) < 0.05f);

    // 4. Test Camera pitch clamp
    OrbitCamera cam;
    cam.Init(player.position);
    cam.pitch = 85.0f;
    cam.Update(player.position, 0.016f, colliders);
    assert(cam.pitch <= Constants::CAM_MAX_PITCH);

    std::cout << "[SMOKE-TEST] All smoke test assertions passed successfully!" << std::endl;
    return 0;
}

int main(int argc, char* argv[]) {
    // Check for headless smoke test mode
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--smoke-test") {
            return RunSmokeTest();
        }
    }

    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Roblox Obby Adventure - M1 Skeleton");
    if (!IsWindowReady()) {
        std::cerr << "Window could not be initialized (no display server available). Running headless smoke test instead." << std::endl;
        return RunSmokeTest();
    }
    SetTargetFPS(60);

    // Initialize player and world
    Player player;
    player.Init(Vector3{ 0.0f, 2.0f, 0.0f });

    // Environment colliders: Ground platform + a few stepping test blocks
    std::vector<AABB> colliders = {
        AABB::FromCenterSize(Vector3{ 0.0f, -0.5f, 0.0f }, Vector3{ 30.0f, 1.0f, 30.0f }),
        AABB::FromCenterSize(Vector3{ 0.0f, 1.0f, -6.0f }, Vector3{ 4.0f, 2.0f, 4.0f }),
        AABB::FromCenterSize(Vector3{ 6.0f, 2.0f, -6.0f }, Vector3{ 4.0f, 4.0f, 4.0f })
    };

    OrbitCamera camera;
    camera.Init(player.position);

    float accumulator = 0.0f;

    // Main game loop
    while (!WindowShouldClose()) {
        float frameDelta = GetFrameTime();
        if (frameDelta > Constants::MAX_FRAME_DELTA) {
            frameDelta = Constants::MAX_FRAME_DELTA;
        }

        // Handle camera update (per frame for smooth rendering)
        camera.Update(player.position, frameDelta, colliders);

        // Accumulate physics steps at fixed 120 Hz
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

            // Kill plane check (e.g. falling off platform)
            if (player.position.y < -15.0f) {
                player.deathCount++;
                player.Respawn();
            }

            accumulator -= Constants::PHYSICS_DT;
        }

        // Render
        BeginDrawing();
        ClearBackground(Color{ 135, 206, 235, 255 }); // Sky blue

        BeginMode3D(camera.GetRaylibCamera());

        // Draw ground and test platforms with Roblox-like aesthetic
        for (const auto& box : colliders) {
            Vector3 center = box.GetCenter();
            Vector3 size = box.GetExtents();
            size.x *= 2.0f; size.y *= 2.0f; size.z *= 2.0f;

            Color faceColor = (center.y < 0.0f) ? Color{ 80, 180, 70, 255 } : Color{ 220, 120, 50, 255 };
            Color wireColor = (center.y < 0.0f) ? Color{ 50, 130, 40, 255 } : Color{ 160, 80, 30, 255 };

            DrawCube(center, size.x, size.y, size.z, faceColor);
            DrawCubeWires(center, size.x, size.y, size.z, wireColor);
        }

        // Draw grid around ground
        DrawGrid(30, 1.0f);

        // Draw Player
        player.Draw();

        EndMode3D();

        // 2D HUD
        DrawFPS(20, 20);
        DrawText("Controls: WASD Move | Space Jump | Mouse Right-Drag / Arrows Orbit | Scroll Zoom | R Respawn", 20, 50, 18, DARKGRAY);

        char statsBuf[128];
        snprintf(statsBuf, sizeof(statsBuf), "Pos: (%.1f, %.1f, %.1f) | Grounded: %s | Deaths: %d",
                 player.position.x, player.position.y, player.position.z,
                 player.isGrounded ? "YES" : "NO", player.deathCount);
        DrawText(statsBuf, 20, 75, 18, DARKBLUE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
