# Blocky Obby Adventure

A complete, single-player 3D platformer/obby built in C++17 from scratch using **raylib 5.5** and **raygui**.

Inspired by cooperative Roblox obbies (such as *Together [Party Game]*), **Blocky Obby Adventure** replaces multi-player dependency with clever single-player solvable puzzle mechanics—such as sprint dash routes and **Ghost-Clone Recording**—where you record your own trajectory to step on pressure plates and hold doors open for yourself.

https://github.com/c2-07/apart/raw/main/gameplay.mp4

---

## Key Features

- **4 Unique Themed Worlds & 20 Authored Levels:**
  1. 🌲 **Meadow (World 1):** Precision jumping, ascending spirals, ramp courses, and spike crossings.
  2. 🌊 **Ocean (World 2):** Moving platforms across water lagoons, high-arc bounce pads, and vertical lifts.
  3. ❄️ **Winter (World 3):** Fragile crumbling ice tiles, spinning frost bars, and blizzard steps.
  4. 🏭 **Factory (World 4):** Conveyor platforms, industrial spikes, timed switches, twin-path ghost puzzles, and the grand Master Generator finale.
- **Custom In-House Physics & Collision:**
  - 120 Hz fixed-timestep physics accumulator with substepping (0.04m micro-steps).
  - Axis-by-axis AABB resolution (X, Z, Y) with orthogonal skin margins eliminating seam snagging.
  - Tunneling prevention against high fall speeds (-40 m/s against 0.05m thin platforms).
  - Coyote time (0.1s) and jump buffering (0.1s) for responsive platforming feel.
  - Carrier physics: riding moving platforms adds platform velocity with zero jitter or drift.
- **Solo Cooperative Puzzle Mechanics:**
  - **Timed Route Puzzles:** Step on pressure plates with visible 3D countdown rings and race to doors before they shut.
  - **Ghost-Clone Puzzles:** Record up to 20 seconds of your movements at 120 Hz, then replay yourself as a translucent holographic clone to hold down buttons while your real player navigates newly opened paths.
- **Chunky Blocky Art Direction:**
  - Distinct per-world 5-color palettes with edge outlines and face shading.
  - Dynamic procedural skybox with drifting cube clouds.
  - Cube-built humanoid avatar with expressive face, walk/run animations, idle breathing, and jump/fall poses.
  - Dynamic fake drop shadow projected dynamically onto ground geometry via downward raycasts.
  - 3D particle puffs emitted on jumps and hard landings.
- **Procedural Audio Synthesizer:**
  - Fully procedural sound effects synthesized in code via raylib's `Wave` API (jump chirp, landing thud, checkpoint chime, death buzz, victory fanfare). No external audio files needed.
- **Menus, Customization & Saving:**
  - Title menu with 3D orbiting camera background.
  - Interactive Character Customizer with 6 selectable shirt colors and a rotating 3D preview.
  - Level Select grid showing unlocked worlds, 1-3 star ratings based on par times, and personal best records.
  - Pause menu and settings (Master Volume, Camera Sensitivity, Invert Pitch Y, Fullscreen).
  - Human-readable save system (`savegame.txt`) persisting all progress.
- **Verification & Testing Suite:**
  - Headless `--smoke-test` mode validating all physics, collision, and save logic without a display.
  - Headless `--validate-levels` mode running a BFS jump arc reachability solver across all 20 levels.

---

## Controls

| Action | Control |
|---|---|
| **Move** | `W`, `A`, `S`, `D` (Camera-relative) |
| **Jump** | `Spacebar` |
| **Orbit Camera** | `Right Mouse Drag` or Arrow Keys (`Left`, `Right`, `Up`, `Down`) |
| **Zoom Camera** | `Mouse Wheel` (4m to 14m) |
| **Respawn** | `R` (Respawns at last checkpoint or spawn) |
| **Ghost Clone** | `G` (1st press: start recording; 2nd press: rewind & play; 3rd press: clear) |
| **Pause / Resume** | `Escape` |
| **Toggle Noclip (Cheat)** | `F1` (Fly with `WASD` + `Space` / `Shift`) |
| **Teleport (Cheat)** | `T` (Instantly teleports to next checkpoint or finish) |

---

## Building and Running

### Prerequisites
- **CMake 3.20+**
- **C++17 compliant compiler** (Clang, GCC, or MSVC)
- Git (for CMake FetchContent to download raylib 5.5)

### Build Instructions
```bash
# Clone repository
git clone https://github.com/<your-username>/apart.git
cd apart

# Configure with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Compile
cmake --build build
```

### Running the Game
```bash
# Launch game
./build/obby
```

### Running Headless Tests
The executable includes built-in automated test modes that run completely headless without requiring a display:

```bash
# Run comprehensive automated physics, hazard, and savegame smoke tests:
./build/obby --smoke-test

# Run analytical BFS level reachability validator on all 20 courses:
./build/obby --validate-levels
```

---

## Project Structure

```
├── CMakeLists.txt              # CMake build configuration (raylib 5.5 FetchContent)
├── NOTES.md                    # Technical decisions, physics specs, milestone notes
├── README.md                   # Project overview, build instructions, controls
├── assets/
│   └── levels/                 # 20 authored .lvl files + puzzle levels
│       ├── w1_l1.lvl ... w1_l5.lvl   (World 1: Meadow)
│       ├── w2_l1.lvl ... w2_l5.lvl   (World 2: Ocean)
│       ├── w3_l1.lvl ... w3_l5.lvl   (World 3: Winter)
│       ├── w4_l1.lvl ... w4_l5.lvl   (World 4: Factory)
│       ├── puzzle_timed.lvl          (Milestone 6 Timed Route demo)
│       └── puzzle_ghost.lvl          (Milestone 6 Ghost Duo demo)
├── src/
│   ├── main.cpp                # Program entrypoint, CLI argument dispatch, main loop
│   ├── game.h / .cpp           # Game state machine, menus, level transitions, savegame
│   ├── constants.h             # All tunable gameplay, physics, and camera constants
│   ├── player.h / .cpp         # Player physics, cube character rendering, animations
│   ├── camera.h / .cpp         # Orbit camera, raycast obstacle pull-in, mouse/pad input
│   ├── collision.h / .cpp      # AABB intersection, slab raycasts, per-axis resolution
│   ├── level.h / .cpp          # Line-based .lvl parser, movers, spinners, crumbles, buttons
│   ├── ui.h / .cpp             # Custom raygui-styled UI widgets, buttons, sliders, modals
│   ├── audio.h / .cpp          # Procedural wave audio synthesizer and sound playback
│   ├── particle.h / .cpp       # 3D cube puff particle system
│   └── validator.h / .cpp      # Analytical BFS level validator and jump solver
└── tools/
    └── level_generator.py      # Level generator script for procedural course layouts
```

---

## Level Format (`.lvl`)

Levels are plain human-readable text files parsed line-by-line:

```text
# Header
name <Course Name>
world <1..4>
killY <float>
par <seconds>

# Entities (whitespace separated)
spawn       x y z
checkpoint  x y z
finish      x y z  sx sy sz
box         x y z  sx sy sz  colorIndex
mover       x y z  sx sy sz  dx dy dz  period  colorIndex
spinner     x y z  length height speed
crumble     x y z  sx sy sz  delay respawnTime
spikes      x y z  sx sy sz
button      id x y z  duration
door        id x y z  sx sy sz
bounce      x y z  sx sy sz  strength
```

---

## Architecture & Design Notes

1. **Clean Dependencies:** Only depends on raylib 5.5 and raygui. No external physics engine, no ECS framework, no third-party JSON/YAML parser.
2. **Tunable Constants:** Every single gameplay variable (gravity, jump velocity, run speed, coyote window, camera limits, particle lifespans) is centralized in [src/constants.h](file:///Users/gourav/dev/apart/src/constants.h).
3. **Robust Seam Traversal:** When walking over adjacent blocks, traditional collision engines often catch on internal seams. Our per-axis solver applies a `SKIN = 0.01m` tolerance margin on non-tested axes to ensure completely snag-free walking.
4. **Substepping Tunneling Prevention:** High fall velocities can cause tunneling through thin platforms. The physics loop substeps large displacements into $\le 0.04$m increments, guaranteeing thin floors (down to 0.05m) are always caught.
5. **Zero Compiler Warnings:** Built with `-Wall -Wextra -pedantic -Werror` enabled across all project code.
