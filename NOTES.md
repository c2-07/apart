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

### Milestone 3: Level Loading & Flow
- **Status:** COMPLETED
- **Features Implemented:**
  - Robust line-based `.lvl` file parser skipping comments, empty lines, and gracefully reporting line errors.
  - Parsed entities: `name`, `world`, `killY`, `par`, `spawn`, `box`, `checkpoint`, `finish`.
  - Authored `assets/levels/w1_l1.lvl`: tutorial course with 28 platforms, 2 checkpoints, and finish zone.
  - Checkpoint system: activating checkpoint sets player's active spawn point, activates flag/pad visual indicator, and increments checkpoint HUD counter.
  - Fall-to-death & R-key respawns player at active checkpoint.
  - Clean HUD panel showing Level Name, Level Timer, Par Time, Checkpoint count, and Death counter.
  - Results screen dialog modal displaying final time, deaths, 1-3 star rating calculation based on par time, with interactive Retry and Next buttons.
  - Automated smoke test verifying parsing, checkpoint triggers, respawn points, and finish trigger.

### Milestone 4: Moving Things & Hazards
- **Status:** COMPLETED
- **Features Implemented:**
  - Moving platforms (`mover`): smooth sine-wave ping-pong trajectories with carrier physics that displace the player by frame displacement with zero jitter.
  - Rotating spinner hazards (`spinner`): analytical XZ segment-to-player collision with Y-height overlap; touches are fatal hazards that trigger respawns.
  - Crumble tiles (`crumble`): 3-state state machine (`STABLE` -> `CRUMBLING` with visual rumble shake -> `FALLEN` non-solid -> `STABLE` respawn).
  - Hazard spikes (`spikes`): drawn as 3D pyramids; fatal on contact.
  - Bounce pads (`bounce`): launch pads with customizable impulse strength propelling the player upward into high arcs.
  - Automated smoke tests verifying mover carrying with zero relative drift, spinner sweep detection, crumble state lifecycle, spike collision, and bounce pad velocity.



