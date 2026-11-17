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

### Milestone 5: Character & Polish
- **Status:** COMPLETED
- **Features Implemented:**
  - Full cube-built character assembled from 6 primary body segments (head, torso, 2 arms, 2 legs) with face eyes and smile.
  - Procedural animations: running walk cycle with swinging limbs and torso bobbing, idle breathing, and jumping/falling airborne poses.
  - Character customization with 6 selectable shirt colors (Classic Blue, Crimson Red, Emerald Green, Royal Purple, Sunset Orange, Sunshine Yellow).
  - Soft fake drop shadow projected dynamically onto the nearest surface below via downward raycast.
  - 3D cube particle puff system emitting on jumps and hard landings.
  - Screen shake feedback and dark red respawn fade on death.
  - Procedural sound effect synthesizer using raylib `Wave` API (jump chirp, landing thud, checkpoint chime, death buzz, victory fanfare) with no external audio assets needed.
  - Interactive master volume slider control on the HUD.

### Milestone 6: Single-Player Solo Cooperative Puzzles
- **Status:** COMPLETED
- **Features Implemented:**
  - Interactive pressure buttons (`button id x y z duration`): activated when stepped on by either player or ghost clone; stays active for `duration` seconds after release.
  - Visible 3D countdown progress ring rendered around active buttons.
  - Linked timed doors (`door id x y z sx sy sz`): solid barrier when closed; passes player cleanly when active.
  - Ghost-clone recording system (`GhostRecorder`): records position, yaw, and grounded state up to 20 seconds at 120 Hz, replaying as a translucent holographic clone capable of stepping on pressure plates.
  - Authored two complete standalone puzzle courses:
    - `assets/levels/puzzle_timed.lvl`: Timed route puzzle requiring sprint and precision platforming across a chasm before the door locks.
    - `assets/levels/puzzle_ghost.lvl`: Asymmetric dual-path co-op puzzle requiring ghost recording to hold Button 2 while the real player runs through Door 2.
  - Automated smoke tests validating both puzzle mechanics and completions.

### Milestone 7: Worlds, Content & Validator
- **Status:** COMPLETED
- **Features Implemented:**
  - 4 unique themed worlds with distinct visual palettes and core mechanics:
    1. **Meadow (World 1, Levels 1-5):** Fundamental jumps, slope courses, elevated spirals, and spike crossings leading to the Meadow Apex boss summit.
    2. **Ocean (World 2, Levels 1-5):** Moving platforms across water lagoons, high-arc bounce pads, vertical lifts, and the Sunken Citadel multi-mover gauntlet.
    3. **Winter (World 3, Levels 1-5):** Fragile crumbling ice tiles, rotating frost hazards, blizzard steps, and the towering Summit of the Frost Titan.
    4. **Factory (World 4, Levels 1-5):** Conveyor movers, industrial spikes, timed switches, twin-path ghost puzzles, and the grand Master Generator finale combining all game mechanics.
  - `tools/level_generator.py`: authored and generated all 20 level files with exact kinematic tolerances.
  - Built-in Level Validator (`--validate-levels` CLI): breadth-first search path reachability solver verifying jump distance bounds ($D_{max} \le 6.13$m flat, plus launch pad and mover displacements) with zero unreachable gaps across all 20 levels.
  - Debug cheats: F1 toggles 6DOF noclip flight (WASD + Space/Shift), and 'T' teleports the player directly to the next checkpoint or finish for rapid level playtesting.

### Milestone 8: Menus, Saving, and Release
- **Status:** COMPLETED
- **Features Implemented:**
  - **Full Game State Machine:** `MAIN_MENU`, `LEVEL_SELECT`, `CHARACTER_SELECT`, `SETTINGS`, `PLAYING`, `PAUSED`, `RESULTS`.
  - **Main Menu:** Attractive 3D rotating background camera orbit with custom rounded buttons: Play, Level Select, Character, Settings, and Quit.
  - **Pause Menu:** Activated via `ESC` key while playing; semi-transparent dark backdrop overlay with Resume, Restart Level, Settings, and Quit to Menu.
  - **Settings Menu:** Sliders for Master Volume and Camera Sensitivity (0.05x to 0.50x), toggles for Invert Pitch Y and Fullscreen mode, with Save & Back returning cleanly to the previous state (Main Menu or Pause).
  - **Character Customization:** Interactive screen with a rotating 3D model of the cube character, live selection swatches for 6 shirt colors, persistent selection saved immediately.
  - **Level Select Grid:** 4-row grid for Worlds 1-4 with 5 levels each (20 levels total). Displays star rating indicators (`***`, `**-`, `*--`, `---`) and recorded best times. Automatically enforces progression: completing the final boss level of a world unlocks the next world.
  - **Save System:** Human-readable line-based `savegame.txt` next to the executable, storing shirt color, audio volume, camera sensitivity, invert Y, fullscreen preference, unlocked worlds, and per-level best times and star ratings. Automatically loads on startup and saves on changes/completion/quit.
  - **Verification:**
    - Headless automated smoke test (`--smoke-test`) validates save file creation, round-trip serialization/deserialization, level reachability, ghost puzzle logic, and high-speed collision with zero crashes.
    - Headless level validator (`--validate-levels`) validates reachability across all 20 authored courses.
    - Windowed Cocoa OpenGL display mode runs at a smooth 60 FPS on macOS with interactive raygui-styled UI controls.
    - Zero compiler warnings under `-Wall -Wextra -pedantic -Werror`.







