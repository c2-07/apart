# Development Notes - 3D Roblox-Style Single-Player Obby

## Overview
A single-player 3D platformer/obby built in C++17 from scratch using raylib 5.5 and raygui.

## Build and Run Instructions
```bash
# Configure and build
cmake -B build
cmake --build build

# Run game (windowed mode)
./build/obby

# Run headless automated physics & gameplay smoke test
./build/obby --smoke-test
```

## Milestone Status

### Milestone 1: Skeleton
- **Status:** COMPLETED
- **Features Implemented:**
  - CMake setup fetching raylib 5.5 and configuring raygui.
  - Strict compiler flags (`-Wall -Wextra -pedantic -Werror` on project sources) building with zero warnings.
  - Centralized tunable gameplay constants in `src/constants.h`.
  - Fixed-timestep physics accumulator at 120 Hz with `MAX_FRAME_DELTA = 0.1s`.
  - Controllable player cube with WASD camera-relative movement and jumping.
  - Third-person orbit camera with pitch clamping `[-10, 70]` degrees, scroll zooming `[4, 14]` meters, and ray-vs-AABB obstacle pull-in.
  - Headless `--smoke-test` verification mode checking landing, acceleration, peak jump height (~2.2m), and camera clamping.
  - Full Cocoa OpenGL windowed mode verified running smoothly at 60 FPS on macOS.
- **Architectural Decisions:**
  - Coordinates: +Y up, units in meters. Player AABB is 1.0m (W) x 2.0m (H) x 1.0m (D) centered at player feet position.
  - Camera-relative movement: W moves away from camera eye along ground plane, S towards camera, A/D strafe.
  - Ray-AABB intersection: implemented using Kay & Kajiya slab method with axis-aligned bounding boxes.
  - Per-axis collision: tested with a tolerance skin (1 cm) to prevent edge snagging when walking across adjacent surfaces.

### Milestone 2: Collision
- **Status:** COMPLETED
- **Features Implemented:**
  - Robust per-axis AABB resolution (X, then Z, then Y) with substepping (max 0.04m micro-steps).
  - Tolerance skin margin (0.01m) on orthogonal axes to completely eliminate edge snagging when crossing seams between adjacent boxes.
  - Tunneling prevention against high speeds (tested at terminal velocity -40 m/s against 0.05m thin platforms).
  - High-speed wall collision stopping (tested at 80 m/s without clipping through).
  - Coyote time window (0.1s) allowing jump inputs shortly after walking off ledges.
  - Jump buffering window (0.1s) triggering jumps immediately upon touching down.
  - Camera collision pull-in using ray-vs-AABB sweep against scene geometry with smooth pull-in and obstacle clearance margin (0.25m).
  - Comprehensive headless unit tests in `--smoke-test` asserting all collision and control criteria.

