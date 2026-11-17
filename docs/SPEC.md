# Software Specification: Blocky Obby Adventure

**Document:** System Requirements & Functional Specification  
**Branch:** `2d-version`  
**Game Title:** Blocky Obby Adventure  
**Revision:** 2.1 (Post-2D Platformer Feature Pass)

---

## 1. Purpose and Scope

### 1.1 Purpose
This document specifies the software requirements, behavioral contracts, technical constraints, and control schemes for **Blocky Obby Adventure**, an original single-player 3D and 2D platformer game built in C++17 using raylib 5.5. The project serves as an exemplary academic submission for a college Object-Oriented Programming (OOP) laboratory, demonstrating rigorous object-oriented architecture, custom kinematic physics, data-driven levels, procedural audio/visual shaders, and automated test validation.

### 1.2 Scope
The game is inspired by classic cooperative obstacle-course ("obby") platformers, re-engineered as a single-player puzzle and agility challenge. The player navigates a blocky avatar across 20 campaign levels divided into 4 thematic worlds, avoiding environmental hazards, stomping or evading autonomous enemies, collecting coins, solving single-player cooperative puzzles (timed runs and ghost clones), and reaching the finish portal under par time.

### 1.3 Tech Stack & Toolchain
- **Language Standard:** C++17 (ISO/IEC 14882:2017).
- **Build System:** CMake 3.20+ with out-of-source builds.
- **Compiler Flags:** Compiles cleanly with `-Wall -Wextra -pedantic -Werror` (GCC/Clang) and `/W4` (MSVC).
- **Core Engine & Windowing:** raylib 5.5 (pinned via CMake `FetchContent_Declare`).
- **UI System:** Immediate-mode custom UI routines backed by raygui headers bundled with raylib.
- **Physics Engine:** Custom AABB substepped spatial resolution (no third-party physics libraries like Box2D, Bullet, or PhysX).
- **Audio Backend:** raylib / miniaudio procedural audio synthesis with silent fallback for headless environments.
- **Graphics Pipeline:** OpenGL 3.3 Core Profile with custom Lambertian diffuse directional lighting shaders and procedural geometric primitives.
- **Level Serialization:** Custom plain-text line-based parser (no third-party JSON/XML libraries).

---

## 2. Functional Requirements

### 2.1 Player Kinematics & Movement
- **FR-1 [Horizontal Movement]:** The player avatar moves at a top run speed of $8.0\text{ m/s}$ (`Constants::RUN_SPEED`). On ground surfaces, horizontal acceleration is $60.0\text{ m/s}^2$ (`Constants::GROUND_ACCEL`), and ground deceleration/friction is $50.0\text{ m/s}^2$ (`Constants::GROUND_FRICTION`).
- **FR-2 [Air Control]:** While airborne, horizontal steering acceleration is reduced to $24.0\text{ m/s}^2$ (`Constants::AIR_ACCEL`, exactly 40% of ground acceleration), with minimal air drag of $2.0\text{ m/s}^2$ (`Constants::AIR_DRAG`).
- **FR-3 [Vertical Kinematics & Gravity]:** Gravity accelerates the player downward at $-30.0\text{ m/s}^2$ (`Constants::GRAVITY`), clamped to a terminal falling velocity of $-40.0\text{ m/s}$ (`Constants::TERMINAL_VELOCITY`).
- **FR-4 [Jumping]:** Jumping from the ground imparts an instantaneous upward velocity of $11.489\text{ m/s}$ (`Constants::JUMP_VELOCITY`), achieving a jump apex of exactly $2.2\text{ m}$ (`Constants::JUMP_HEIGHT`).
- **FR-5 [Coyote Time]:** If a player walks off a platform ledge without jumping, a grace window of $0.10\text{ s}$ (`Constants::COYOTE_TIME`) allows a full-height jump to fire.
- **FR-6 [Jump Buffering]:** Pressing the jump key up to $0.10\text{ s}$ (`Constants::JUMP_BUFFER_TIME`) prior to touching the ground buffers the input, firing the jump automatically on the first grounded tick.
- **FR-7 [Model Orientation]:** The character's visual model smoothly slerps toward its horizontal movement vector at an angular speed of $18.0\text{ rad/s}$ (`Constants::MODEL_TURN_SPEED`).

### 2.2 Collision & Environmental Interactions
- **FR-8 [AABB Collision Resolution]:** Player geometry is modeled as an Axis-Aligned Bounding Box ($1.0\text{ m}$ wide, $2.0\text{ m}$ tall, $1.0\text{ m}$ deep). Movement is resolved across sub-steps (max $0.04\text{ m}$ per substep) in strict order: $X$-axis, $Z$-axis, then $Y$-axis, with a $0.01\text{ m}$ skin tolerance.
- **FR-9 [Static Platforms]:** Platforms are solid obstacles with deterministic color variations based on spatial coordinate hashes and a raised top cap ($0.10\text{ m}$ tall, $0.04\text{ m}$ overhang).
- **FR-10 [Moving Platforms]:** Oscillate smoothly between a base position and an offset displacement vector over a configurable sinusoidal period. Moving platforms transport any grounded player standing on their top surface by their per-frame displacement vector.
- **FR-11 [Crumble Tiles]:** Solid platforms that shake and fall $0.8\text{ s}$ to $1.0\text{ s}$ after player contact, respawning after a $2.0\text{ s}$ to $2.5\text{ s}$ delay.
- **FR-12 [Bounce Pads]:** Launch the player upward with a custom velocity (e.g., $18\text{ m/s}$ to $22\text{ m/s}$), resetting coyote time and grounded flags.

### 2.3 Single-Player Puzzle Mechanics
- **FR-13 [Timed Route Puzzles]:** Stepping on a timed button (red/green) opens associated door entities for a specified duration before closing automatically.
- **FR-14 [Ghost-Clone Playback]:** Pressing `G` allows the player to record up to $20\text{ s}$ of movement. Pressing `G` again starts playback: an autonomous translucent clone replays the exact trajectory, capable of depressing buttons to solve dual-switch puzzles solo.

### 2.4 Health, Hazards & Enemies (2D Platformer Additions)
- **FR-15 [Hearts System]:** The player possesses a maximum of 3 hearts (`Constants::MAX_HEARTS`). Sustaining damage deducts 1 heart, grants $1.5\text{ s}$ of invincibility (`Constants::INVINCIBILITY_TIME`) with $0.10\text{ s}$ drawing blink alternation, and applies $6.0\text{ m/s}$ knockback with a $5.0\text{ m/s}$ vertical hop.
- **FR-16 [Defeat & Checkpoint Respawn]:** At 0 hearts or upon falling below `killY`, the player respawns at the latest active checkpoint with hearts fully replenished. Checkpoints also instantly restore hearts to 3 when activated. Spikes and kill planes remain instant-kill hazards.
- **FR-17 [Walker Enemies]:** Ground-patrolling enemies that pace at $2.0\text{ m/s}$ (`Constants::WALKER_SPEED`). Autonomous downward and forward ray probes detect walls and platform drop-offs, automatically turning around before walking off ledges.
- **FR-18 [Flyer Enemies]:** Airborne hazards moving horizontally across a patrol range while oscillating vertically according to a sine wave ($y = y_0 + \sin(t \cdot \text{waveSpeed}) \cdot \text{amplitude}$).
- **FR-19 [Stomp Attack]:** Landing on a Walker or Flyer with downward velocity and feet above the enemy's vertical midpoint defeats the enemy, triggering a $0.20\text{ s}$ flattening animation and bouncing the player upward at $9.0\text{ m/s}$ (`Constants::STOMP_BOUNCE`).
- **FR-20 [Bump Blocks]:** Solid question-mark blocks. Hitting a bump block from below while jumping upwards triggers a $0.20\text{ m}$ bounce animation over $0.15\text{ s}$, ejects a golden coin into the player's inventory with audio/particle feedback, and transitions into an empty gray block.
- **FR-21 [Saw Blades]:** Continuously rotating circular blades ($14.0\text{ rad/s}$) moving back and forth between two track endpoints. Contact damages the player via circle-vs-box distance checks.

### 2.5 Camera, Levels & Save System
- **FR-22 [Third-Person Orbit Camera]:** Spherical coordinate camera orbiting the player with smooth spherical interpolation, mouse pitch/yaw control, automatic zoom pull-in to avoid wall occlusion, and speed-based FOV expansion ($65^\circ \rightarrow 71^\circ$).
- **FR-23 [20-Level Campaign]:** 4 distinct worlds (Meadow, Lagoon, Ice Peak, Clockwork Factory) containing 5 levels each, totaling 20 levels. Progress through World $N$ unlocks World $N+1$.
- **FR-24 [Star Rating & Par Time]:**
  - **3 Stars:** Completed under par time with 0 deaths.
  - **2 Stars:** Completed under $140\%$ of par time.
  - **1 Star:** Completed above $140\%$ of par time or with deaths.
- **FR-25 [Save System]:** Plain-text persistence (`savegame.txt`) saving shirt color, master volume, mouse sensitivity, invert-Y axis, unlocked worlds, and per-level best times and star ratings.

---

## 3. Non-Functional Requirements

- **NFR-1 [Frame Rate Target]:** Game must maintain a sustained $60+\text{ FPS}$ on standard laptop integrated GPUs.
- **NFR-2 [Deterministic Fixed Timestep]:** Physics is strictly executed at $120\text{ Hz}$ ($dt = 1/120\text{ s}$) using a delta-time accumulator capped at $0.10\text{ s}$ to prevent the "spiral of death."
- **NFR-3 [Zero Compiler Warnings]:** Codebase must compile without any warnings under `-Wall -Wextra -pedantic -Werror` on Clang and GCC.
- **NFR-4 [Headless CI/Smoke Test]:** Supports non-graphical automated testing via `--smoke-test` and `--validate-levels`, validating all 20 levels and 16 functional subsystems without requiring an active display or audio device.
- **NFR-5 [Zero Dynamic Heap Allocs in Core Loop]:** Particle systems use pre-allocated `ObjectPool<T>`, and entity lists avoid mid-frame reallocations.

---

## 4. Controls Reference Table

| Context | Action | Primary Key / Binding | Secondary Key / Controller |
|---|---|---|---|
| **Movement** | Move Forward / Backward | `W` / `S` | Up / Down Arrows |
| **Movement** | Move Left / Right | `A` / `D` | Left / Right Arrows |
| **Movement** | Jump | `Space` | Buffered / Coyote enabled |
| **Movement** | Quick Respawn | `R` | Triggers fade transition |
| **Camera** | Orbit Yaw & Pitch | Mouse Move (Right Click drag or direct) | Left / Right / Up / Down |
| **Camera** | Zoom In / Out | Mouse Wheel Scroll | Clamped between 4m and 14m |
| **Puzzle** | Record / Stop / Play Ghost | `G` | Up to 20s recording |
| **Debug** | Noclip Flight Toggle | `F1` | Fly with WASD + Space/Shift |
| **Debug** | Teleport to Checkpoint | `T` (in Noclip) | Instant warp to latest checkpoint |
| **Diagnostics**| Toggle FPS Counter | `F3` | Top-right display |
| **System** | Pause Game / Return | `Escape` | Opens PauseState menu |

---

## 5. Scoring & Star Rules

$$\text{Star Rating} = 
\begin{cases} 
3 \text{ Stars} & \text{if } t \le t_{\text{par}} \text{ and } \text{deaths} = 0 \\
2 \text{ Stars} & \text{if } t \le 1.4 \cdot t_{\text{par}} \\
1 \text{ Star} & \text{otherwise (level completed)}
\end{cases}$$

Where $t$ is level elapsed time, and $t_{\text{par}}$ is defined in the level file. Completing all 5 levels of a world unlocks access to the subsequent world in `SaveData`.

---

## 6. Assumptions and Constraints

1. **No External Physics Libraries:** All raycasts, box-box intersections, sweep resolutions, and circle-box hitboxes are custom implemented in C++.
2. **Text-Based File Formats:** Levels (`.lvl`) and save files (`savegame.txt`) use line-oriented ASCII formats parsed with standard C++ streams.
3. **Hardware Independence:** When no audio hardware or graphics display server is found, the game gracefully falls back to silent audio generation and terminal test diagnostics.
