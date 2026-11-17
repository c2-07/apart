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

    // Rendering & Depth (Visual Pass Step 1)
    constexpr float CLIP_NEAR = 0.1f;               // near clipping plane for depth precision
    constexpr float CLIP_FAR = 300.0f;              // far clipping plane to prevent z-shimmer
    constexpr float CHECKPOINT_PAD_HEIGHT = 0.06f;  // raised thin box height
    constexpr float CHECKPOINT_PAD_OFFSET_Y = 0.03f;// resting on top of platform
    constexpr float SURFACE_DECAL_OFFSET_Y = 0.02f; // prevent coplanar z-fighting
    constexpr float SHADOW_OFFSET_Y = 0.02f;        // shadow elevation above surface
    constexpr float SHADOW_HEIGHT = 0.02f;          // shadow thickness

    // Lighting & Atmosphere (Visual Pass Step 2 & 4)
    constexpr float LIGHT_DIR_X = 0.45f;
    constexpr float LIGHT_DIR_Y = 0.85f;
    constexpr float LIGHT_DIR_Z = 0.28f;
    constexpr float SUN_LIGHT_INTENSITY = 0.65f;
    constexpr float AMBIENT_LIGHT_INTENSITY = 0.52f;
    constexpr float OUTLINE_DARKEN_FACTOR = 0.82f;
    constexpr float OUTLINE_EXPANSION = 0.015f;
    constexpr float DEFAULT_FOG_DENSITY = 0.005f;

    // Platform Detail & Variation (Visual Pass Step 3)
    constexpr float PLATFORM_CAP_HEIGHT = 0.10f;
    constexpr float PLATFORM_CAP_OVERHANG = 0.04f;
    constexpr float COLOR_VARIATION_INTENSITY = 0.06f;

    // Shadows (Visual Pass Step 5)
    constexpr float SHADOW_MAX_DISTANCE = 18.0f;
    constexpr float SHADOW_BASE_RADIUS = 0.95f;
    constexpr float SHADOW_MIN_RADIUS = 0.45f;
    constexpr unsigned char SHADOW_MAX_ALPHA = 175;

    // Checkpoint & Finish Polish (Visual Pass Step 6)
    constexpr float CHECKPOINT_FLAG_WAVE_SPEED = 6.0f;
    constexpr float CHECKPOINT_RING_PULSE_SPEED = 4.0f;
}
