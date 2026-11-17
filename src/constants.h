#pragma once
#include <cmath>

namespace Constants {
    // Timing & Physics Loop
    constexpr float PHYSICS_TICK_RATE = 120.0f;
    constexpr float PHYSICS_DT = 1.0f / PHYSICS_TICK_RATE;
    constexpr float MAX_FRAME_DELTA = 0.1f;

    // Player Dimensions (meters)
    constexpr float PLAYER_WIDTH = 1.0f;
    constexpr float PLAYER_HEIGHT = 2.0f;
    constexpr float PLAYER_DEPTH = 1.0f;
    constexpr float HALF_PLAYER_WIDTH = PLAYER_WIDTH * 0.5f;
    constexpr float HALF_PLAYER_DEPTH = PLAYER_DEPTH * 0.5f;

    // Movement & Kinematics
    constexpr float RUN_SPEED = 8.0f;               // m/s
    constexpr float GROUND_ACCEL = 60.0f;           // m/s^2
    constexpr float GROUND_FRICTION = 50.0f;        // m/s^2 deceleration
    constexpr float AIR_ACCEL = 24.0f;              // m/s^2 (40% of ground accel)
    constexpr float AIR_DRAG = 2.0f;                // minimal air resistance
    constexpr float GRAVITY = -30.0f;               // m/s^2
    constexpr float TERMINAL_VELOCITY = -40.0f;     // m/s
    constexpr float JUMP_HEIGHT = 2.2f;             // meters
    // v = sqrt(2 * -g * h)
    constexpr float JUMP_VELOCITY = 11.489125f;     // m/s sqrt(2 * 30 * 2.2)
    constexpr float COYOTE_TIME = 0.1f;             // seconds
    constexpr float JUMP_BUFFER_TIME = 0.1f;        // seconds
    constexpr float MODEL_TURN_SPEED = 18.0f;       // rad/s smooth model rotation

    // Camera
    constexpr float CAM_MIN_PITCH = -10.0f;         // degrees
    constexpr float CAM_MAX_PITCH = 70.0f;          // degrees
    constexpr float CAM_MIN_DISTANCE = 4.0f;        // meters
    constexpr float CAM_MAX_DISTANCE = 14.0f;       // meters
    constexpr float CAM_DEFAULT_DISTANCE = 8.0f;    // meters
    constexpr float CAM_DEFAULT_PITCH = 20.0f;      // degrees
    constexpr float CAM_DEFAULT_YAW = 0.0f;         // degrees
    constexpr float CAM_MOUSE_SENSITIVITY = 0.18f;  // mouse sensitivity factor
    constexpr float CAM_KEY_SPEED = 90.0f;          // deg/s with arrow keys
    constexpr float CAM_ZOOM_SPEED = 1.5f;          // m per scroll tick
    constexpr float CAM_SMOOTH_SPEED = 14.0f;       // smoothing factor
    constexpr float CAM_TARGET_HEIGHT = 1.2f;       // focus height above player base
    constexpr float CAM_OBSTACLE_MARGIN = 0.25f;    // pull-in clearance
}
