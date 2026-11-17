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

## Visual Polish Pass: Architecture & Initial Assessment

### How Drawing Currently Works
1. **Window, Projection & Depth Buffer:**
   - Window initialized via `InitWindow(1280, 720, ...)`. Default flags lack MSAA anti-aliasing hints.
   - Depth clip planes currently default to standard raylib values (~0.01 to 1000.0m), causing significant z-buffer compression and precision loss on distant platforms and coplanar surfaces.
   - 3D scene uses `BeginMode3D(camera.GetRaylibCamera())` with a fixed 65° vertical FOV perspective projection.

2. **Sky & Background:**
   - `Level::DrawSkybox()` currently clears the frame buffer to a single solid flat color (`pal.skyTop`) with `ClearBackground()`.
   - 8 procedural cube clouds drift along X using `DrawCube` and `DrawCubeWires` with semi-transparent tint.
   - No vertical sky gradient, horizon color blending, or atmospheric depth fog exists.

3. **3D Entity Shading & Lighting:**
   - All geometries (boxes, movers, hazards, checkpoints, player limbs) are drawn using raylib's immediate-mode `DrawCube(center, size, color)` and `DrawCubeWires(center, size, wireColor)`.
   - No shaders or lighting uniforms are currently used; all faces receive 100% flat unlit color, making 3D shapes lack depth perception when camera angles align with cube normals.
   - Outlines are drawn via dark wire cubes overlaid with a crude +0.02m expansion.

4. **Coplanar Geometries & Z-Fighting:**
   - Checkpoint base pads, finish zones, and button rings sit exactly coplanar with underlying platforms (e.g. at $Y = 0.0$), causing visible z-fighting and polygon flicker.
   - Player fake drop shadow is projected as a 0.01m tall box at $+0.015$m offset, which shimmers when the camera rotates at grazing angles.

5. **Character & Particle Rendering:**
   - Avatar assembled from 6 primitive cubes (head, torso, 2 arms, 2 legs) plus eye/smile cubes.
   - Walk animation drives leg/arm pitch via a single sinusoidal walk cycle without velocity-proportional limb scaling, landing squash-and-stretch, or directional lean.
   - Particles are simple unlit falling cubes.

6. **2D Interface (HUD & Menus):**
   - Rendered in screen space after `EndMode3D()` using rounded rectangles and standard bitmap text rendering.
   - HUD displays timer, checkpoint counter, and death counter with fixed padding, without smooth number interpolation, dynamic countdown coloring, or title fade-ins.

### Step 1: Flicker (Z-Fighting) Elimination & Depth Precision
- **Status:** COMPLETED
- **Features Implemented:**
  - Configured 4x MSAA (`FLAG_MSAA_4X_HINT`) and VSync (`FLAG_VSYNC_HINT`) prior to `InitWindow`.
  - Called `rlSetClipPlanes(0.1, 300.0)` immediately after window initialization to narrow the depth range and dramatically increase 24-bit z-buffer precision across distant platforms.
  - Checkpoint pads elevated to thin 0.06m boxes resting on platform top surfaces (`CHECKPOINT_PAD_OFFSET_Y = 0.03m`), eliminating coplanar z-fighting with underlying blocks.
  - Finish zones, mover accent plates, bounce pad spring plates, and button countdown ring dots elevated with `SURFACE_DECAL_OFFSET_Y = 0.02m`.
  - Player drop shadow raised with `SHADOW_OFFSET_Y = 0.02m` and given 0.02m depth thickness to prevent camera grazing angle shimmer.
  - Tunables centralized in `Constants::CLIP_NEAR`, `Constants::CLIP_FAR`, `Constants::CHECKPOINT_PAD_HEIGHT`, etc.

### Step 2: Directional Cartoon Lighting & Ambient Uniforms
- **Status:** COMPLETED
- **Features Implemented:**
  - Authored custom GLSL 330 vertex and fragment shaders in `assets/shaders/lighting.vs` and `assets/shaders/lighting.fs`.
  - Implemented single directional sunlight term with clamped Lambertian diffuse lighting ($N \cdot L$).
  - Added ambient environmental fill light uniform ensuring unlit cube faces stay vibrant, saturated, and legible without muddy black shadows.
  - Uniforms documented with explanation: `lightDir`, `lightColor`, `ambientColor`, `viewPos`, `fogColor`, `fogDensity`.
  - Refined platform edge wire outlines with `OUTLINE_DARKEN_FACTOR = 0.82f` and `OUTLINE_EXPANSION = 0.015m` for crisp, subtle cartoon border definition.
  - Applied consistent shader mode across platforms, hazards, avatar, and character customization preview.

### Step 3: Platform Detailing, Hash Variation & Hazard Styling
- **Status:** COMPLETED
- **Features Implemented:**
  - Added slim top caps (`PLATFORM_CAP_HEIGHT = 0.10m`) with subtle overhang (`PLATFORM_CAP_OVERHANG = 0.04m`) in a lighter tint (`LightenColor(variedColor, 1.25f)`) so platform jump landing edges read clearly.
  - Implemented deterministic 3D spatial hashing (`PositionHash(center)`) to vary base platform face brightness by $\pm 6\%$, removing visual monotony from repeated tiles.
  - **Movers:** Directional chevron indicator stripes aligned with movement axis and yellow hazard bumper trims along the bottom side rims.
  - **Spinners:** Center pivot shaft with a flashing red hazard warning beacon (`GetTime()` pulse) and alternating dual-tone hazard segments across the rotating arm.
  - **Crumble Tiles:** 4 fractured surface insets showing fragile structural cracks; state-driven color shifts to flashing amber-red when crumbling.
  - **Spikes:** Dark hazard-trimmed base block with metallic silver tips atop hazard-red teeth.
  - **Bounce Pads:** High-contrast cyan perimeter ring with hot-pink launch bullseye pad.

### Step 4: Sky Gradients, Atmospheric Fog & World Palettes
- **Status:** COMPLETED
- **Features Implemented:**
  - Fullscreen vertical sky gradient quad (`DrawRectangleGradientV`) rendered per world from `pal.skyTop` to `pal.skyBottom` prior to 3D scene projection.
  - Per-world palette expansion: `skyTop`, `skyBottom`, `fogColor`, `lightColor`, `ambientColor`, and `fogDensity` customized across all 4 worlds (Meadow, Ocean, Winter, Factory).
  - Distance depth fog computed in fragment shader: smoothly blends distant platforms into the horizon fog color according to world fog density ($1 - e^{-\text{dist} \times \text{fogDensity}}$).
  - Low-elevation atmospheric ground haze slab (`killY - 2.0m`, $360\text{m} \times 3\text{m} \times 360\text{m}$) rendered below the course to eliminate empty void.
  - Procedural 3D drifting cube clouds rendered high above play area ($Y \approx 48\text{m}$ to $53\text{m}$) with soft outlines.

### Step 5: Advanced Projected Drop Shadows & Dynamic Scaling
- **Status:** COMPLETED
- **Features Implemented:**
  - Enhanced player drop shadow with two concentric layers: an inner dense core (`SHADOW_MAX_ALPHA = 175`) and an outer soft penumbra fringe.
  - Implemented quadratic distance falloff ($(1 - d/d_{max})^2$) and smooth radius scaling ($0.95\text{m} \to 0.45\text{m}$) up to 18m elevation.
  - Added projected downward raycasted blob shadows for moving platforms (`movers`) and rotating hazard bars (`spinners`) projected onto solid surfaces below with height-dependent scaling and fading.
  - Elevated all projected shadows with `SHADOW_OFFSET_Y = 0.025m` to guarantee zero surface flicker or grazing-angle z-fighting.

### Step 6: Checkpoint & Finish Polish, Waving Flags & Confetti Bursts
- **Status:** COMPLETED
- **Features Implemented:**
  - Checkpoint flags animated with 3 connected waving cloth segments driven by sinusoidal wave equations (`sin(time * 6.0 - s * 0.85)`).
  - Inactive checkpoints render in neutral gray; activated checkpoints switch to bright gold with a soft pulsing green perimeter ring on the pad (`CHECKPOINT_RING_PULSE_SPEED = 4.0`).
  - Added celebratory burst of 24 emerald, gold, and cyan cube particles on checkpoint activation (`SpawnCheckpointBurst`).
  - Finish zone upgraded to a chunky golden victory arch with left/right pillars, cross lintel, pulsing inner portal jewels, and a rotating victory star trophy.
  - Added 60-particle multi-colored celebratory confetti explosion (`SpawnConfettiBurst`) upon course completion.

### Step 7: Character Animation Polish, Squash-and-Stretch & Dust Puffs
- **Status:** COMPLETED
- **Features Implemented:**
  - **Landing Squash-and-Stretch:** On hard landing (`previousFallSpeed < -3.5m/s`), the character model smoothly compresses vertically (`scaleY = 0.88`) and expands horizontally (`scaleXZ = 1.06`) over 0.10s (`LANDING_SQUASH_DURATION`), scaled upwards from feet level.
  - **Directional Lean:** The character leans forward into the direction of motion proportionally to horizontal speed (`LEAN_MAX_ANGLE = 8.5°`), providing dynamic physical weight and velocity cues.
  - **Speed-Proportional Limb Swing:** Arm and leg swing amplitudes scale dynamically with running velocity ratio (`34° * (speed / maxSpeed)`), eliminating artificial robotic pacing at slow speeds.
  - **Idle Breathing & Sway:** When stationary, character displays subtle sinusoidal vertical breathing bob (`0.015m` amplitude at 2.5 rad/s) and gentle resting arm sway.
  - **Expressive Airborne Poses:** Distinct jump pose raises both arms overhead (+135°) with outward splay (+12°) and knees drawn up (-18°); falling pose splays arms outward (+100°) with dangling legs.
  - **Skid Dust Puffs:** Rapid direction changes at high speed (`dot(vel, wishDir) < -0.2` while moving > 4.5 m/s) spawn backward-ejected dust puff particles (`SpawnSkidPuff`) under the player's feet.

### Step 8: Camera and Feel, Lookahead, Dynamic FOV & Transitions
- **Status:** COMPLETED
- **Features Implemented:**
  - **Movement Lookahead:** Smooth focus point lead in velocity direction (`CAM_LOOKAHEAD_DIST = 1.25m`) using exponential interpolation (`CAM_SMOOTH_SPEED`), giving intuitive spatial visibility in the direction of sprint.
  - **Dynamic Speed-Widened FOV:** Camera perspective field of view dynamically expands from 65.0° (`CAM_FOV_BASE`) to 71.0° (`CAM_FOV_MAX`) proportionally to horizontal sprint speed, emphasizing speed and dynamism.
  - **Retuned Death Screen Shake:** Calibrated short, punchy impulse screen shake (`SCREEN_SHAKE_DURATION = 0.28s`, `SCREEN_SHAKE_MAGNITUDE = 0.20m`) with quadratic falloff.
  - **Cinematic Fade Transitions:** Integrated smooth fullscreen fade-in (`LEVEL_LOAD_FADE_SPEED = 2.2`, `RESPAWN_FADE_SPEED = 2.8`) on level start, restart, and player death/respawn.
















