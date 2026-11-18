---
marp: true
theme: default
paginate: true
header: "Blocky Obby Adventure | OOP Game Engine Defense"
footer: "B.Tech CSE — Object-Oriented Programming Lab"
---

# Slide 1: Introduction to Blocky Obby Adventure
## An Object-Oriented 3D/2D Platformer Engine in Modern C++

* **Student:** Gourav Meena (BT25CSE138) | **Programme:** B.Tech Computer Science & Engineering
* **Course:** Object-Oriented Programming Lab | **Version:** `2d-version`

### What is the Game?
* A custom, deterministic **3D third-person and 2D side-scrolling obstacle course (obby) platformer** engineered from scratch in ISO C++17.
* Built **without** commercial game engines (Unity/Unreal) to demonstrate low-level computer science concepts: manual memory control, vtable polymorphism, fixed-step numerical integration, and clean software architecture.
* Features **4 thematic worlds (20 levels)**, moving platforms, crumbling tiles, bounce pads, pressure buttons, locked doors, hazards (spikes, saws), mobile ground/aerial enemies, and a 3-heart combat system.

> **Speaker Notes:** Welcome the evaluators. State that this project is not merely a game, but a complete educational game engine built in native C++17 to showcase foundational OOP paradigms, mathematical simulation stability, and zero-warning software engineering.

---

# Slide 2: Technology Stack & Technical Justification

| Technology | Purpose in Project | Why Chosen? (Technical Justification) |
| :--- | :--- | :--- |
| **ISO C++17** | Core Engine Language | Provides deterministic RAII memory management (`std::unique_ptr`), zero-overhead abstractions, strong compile-time typing, structured bindings, and high performance. |
| **raylib 5.5** | Graphics, Window & Audio | Lightweight, unopinionated C library offering direct OpenGL 3.3 bindings without imposing proprietary object hierarchies or hidden garbage collection runtimes. |
| **raymath** | Linear Algebra | Header-only 3D vector, matrix, and quaternion math optimized for real-time physics and spatial queries. |
| **miniaudio** | Sound Synthesis Engine | Multi-platform, low-latency audio engine embedded in raylib for procedural waveform generation and sound effect playback. |
| **CMake 3.20+** | Build Automation | Cross-platform build configuration enforcing strict `-Wall -Wextra -pedantic -Werror` zero-warning standards across Linux, macOS, and Windows. |
| **GLSL 330** | Programmable Shaders | Custom Lambertian diffuse and ambient shaders for crisp, saturated cartoon lighting. |

> **Speaker Notes:** Emphasize that raylib was selected because it is a minimal C library rather than an opinionated game engine. This required us to architect all game entities, states, physics, and memory management ourselves in modern C++.

---

# Slide 3: High-Level Game Architecture

The engine is organized into **5 decoupled architectural layers** enforcing strict separation of concerns:

```
+----------------------------------------------------------------------------------+
|  1. CORE LOOP (main.cpp)                                                         |
|     120 Hz Fixed Timestep Accumulator  |  Variable Rendering Refresh Dispatch    |
+----------------------------------------------------------------------------------+
                                          |
+----------------------------------------------------------------------------------+
|  2. STATE MACHINE (StateManager, IGameState)                                     |
|     SplashState -> MainMenuState -> LevelSelectState -> PlayingState (Pause/Win) |
+----------------------------------------------------------------------------------+
                                          |
+----------------------------------------------------------------------------------+
|  3. WORLD & SERIALIZATION (Level, EntityFactory)                                 |
|     Tokenized .lvl File Parsing  |  Polymorphic Entity Garbage Collection        |
+----------------------------------------------------------------------------------+
                  |                                                  |
+---------------------------------------+  +---------------------------------------+
|  4. ACTORS & PHYSICS                  |  |  5. SUPPORT SUBSYSTEMS                |
|     Player (PlayerAnimator, Camera)   |  |     SaveSystem (JSON/Disk Storage)    |
|     Entity Hierarchy (12 Subtypes)    |  |     SoundManager (SFX Audio Bank)     |
|     PhysicsEngine & AABB Substepping  |  |     ShaderScope (RAII GLSL Pipeline)  |
+---------------------------------------+  +---------------------------------------+
```

> **Speaker Notes:** Walk the evaluators through the 5 layers. The main loop knows nothing of gameplay rules; it updates the StateManager. The StateManager runs the current state, which orchestrates the Level, Player, Physics, and Audio subsystems.

---

# Slide 4: Core Execution Pipelines & Deterministic Physics

### 1. 120 Hz Fixed-Step Physics Accumulator
* Frame delta time is accumulated and consumed in exact fixed slices ($\Delta t = 1/120\text{ s} \approx 8.33\text{ ms}$).
* Accumulator is clamped at $0.1\text{ s}$ to prevent the "spiral of death" during lag spikes.
* **Why?** Guarantees identical jump trajectories and platform timing on any monitor refresh rate (60 Hz vs 144 Hz).

### 2. Substepped Collision Resolution ($X \rightarrow Z \rightarrow Y$)
* Per-frame displacement partitioned into substeps: $N = \lceil \|\vec{v} \cdot \Delta t\| / 0.04\text{ m} \rceil$.
* Prevents tunneling through thin platforms at high falling speeds ($v_y = -35\text{ m/s}$).
* Resolves horizontal axes ($X, Z$) first for smooth wall-sliding, then vertical ($Y$) for floor landings.

### 3. Platformer Responsiveness ("Juice")
* **Coyote Time ($0.10\text{ s}$):** Allows players to jump within 100 ms of walking off a platform edge.
* **Jump Buffering ($0.10\text{ s}$):** Caches early jump keypresses and triggers them immediately upon landing.
* **Moving Platform Riding:** Seamlessly transfers moving platform velocity and frame displacement to the player.

> **Speaker Notes:** Explain that collision substepping eliminates the classic "tunneling" bug where fast-moving characters pass right through thin geometry. The $X \rightarrow Z \rightarrow Y$ resolution order also eliminates sticky corners.

---

# Slide 5: Where OOP is Used: Entity Polymorphism & Hierarchy

```
                             +-------------------+
                             |   <<abstract>>    |
                             |      Entity       |
                             +-------------------+
                               /       |       \
                 +------------+        |        +------------+
                 |                     |                     |
         +---------------+     +---------------+     +---------------+
         |   Platform    |     |     Enemy     |     |  Interactive  |
         +---------------+     +---------------+     +---------------+
            /         \            /       \         | Spikes, Saws  |
           |           |          |         |        | Spinners      |
         +-------+ +---------+ +------+ +-------+    | BouncePads    |
         |Moving | |Crumble  | |Walker| | Flyer |    | Checkpoints   |
         |Plat.  | |Tile     | +------+ +-------+    | Button / Door |
         +-------+ +---------+                       | BumpBlocks    |
                                                     +---------------+
```

* **Abstract Base Class (`Entity`):** Defines pure virtual contracts: `update(float dt, Player&)`, `draw() const`.
* **Multi-Level Inheritance (`Entity` $\rightarrow$ `Platform` $\rightarrow$ `MovingPlatform`):** Reuses geometry rendering while adding parametric trajectory motion and per-frame displacement export.
* **Hierarchical Inheritance (`Entity` $\rightarrow$ `Enemy` $\rightarrow$ `Walker` / `Flyer`):** Shares squash animation and stomp combat while customizing ground ledge-detection vs sinusoidal flight AI.
* **Vtable Dynamic Dispatch:** The `Level` stores `std::vector<std::unique_ptr<Entity>>`. Updates and draws all entities in a clean loop without `dynamic_cast` or `switch` statements.

> **Speaker Notes:** Emphasize that polymorphism allows the Level class to manage completely different behaviors (moving platforms, enemies, doors) uniformly through base `Entity` pointers via virtual method tables.

---

# Slide 6: Where OOP is Used: Core Design Patterns

### 1. State Pattern (`IGameState` & `StateManager`)
* **Problem:** Complex nested `switch/case` statements handling menus, gameplay, pausing, and victory.
* **Solution:** Abstract `IGameState` interface (`enter`, `update`, `draw`, `exit`). `StateManager` maintains a polymorphic stack, allowing modal states (e.g., `PauseState`) to push over `PlayingState` without resetting game memory.

### 2. Factory Pattern (`EntityFactory`)
* **Problem:** Hardcoding entity constructors directly inside file parsers couples disk formats to concrete classes.
* **Solution:** Static parameterized factory: `EntityFactory::createEntity(type, stream)` returns `std::unique_ptr<Entity>` based on text tokens (`box`, `mover`, `enemy_walker`). Makes level authoring 100% data-driven.

### 3. RAII (Resource Acquisition Is Initialization)
* **Problem:** Memory leaks, double frees, and dangling pointers from manual `new`/`delete`.
* **Solution:** Exclusive ownership via `std::unique_ptr`. Custom RAII wrappers (`ShaderScope`, audio banks) guarantee resource cleanup upon scope exit or level change.

### 4. Composition Over Inheritance
* The `Player` class *has-a* `PlayerAnimator` and *has-a* `CameraController` rather than inheriting them. Decouples procedural squash-and-stretch rendering from physics logic.

> **Speaker Notes:** Point out that the Factory Pattern allows designers to create completely new levels in plain text without recompiling the executable. RAII ensures the game maintains zero memory leaks across level transitions.

---

# Slide 7: Where OOP is Used: Encapsulation & Safe Lifecycles

### 1. Robust Encapsulation & Data Hiding
* All class member data is `private` or `protected` (e.g., `Player::m_position`, `m_velocity`, `m_hearts`).
* State mutations are strictly constrained through public member functions maintaining invariants:
  ```cpp
  player.takeDamage(knockbackDir); // Decrements heart, begins 1.5s invincibility blink, applies impulse
  player.heal(1);                  // Clamped safely to maxHearts (3)
  player.applyImpulse(Vector3{...});// Validated velocity application
  ```

### 2. Safe Deferred Removal Pattern
* **Problem:** Deleting an entity immediately when stomped during a physics loop invalidates active iterators, causing undefined behavior or crashes.
* **Solution:** Virtual `isExpired() const` query. The `Level::update()` loop terminates with an atomic erase-remove idiom:
  ```cpp
  m_entities.erase(
      std::remove_if(m_entities.begin(), m_entities.end(),
                     [](const auto& e) { return e->isExpired(); }),
      m_entities.end());
  ```

> **Speaker Notes:** Highlight iterator invalidation as a classic systems programming challenge. The deferred removal pattern elegantly decouples entity death logic from the container's iteration lifecycle.

---

# Slide 8: Value Classes & Operator Overloading

### 1. The `AABB` (Axis-Aligned Bounding Box) Class
* Encapsulates 3D/2D bounding volumes using `Vector3 min` and `Vector3 max`.
* **Spatial Query Methods:**
  ```cpp
  bool intersects(const AABB& other) const; // Overlap test
  bool contains(const Vector3& point) const;  // Point inclusion
  ```
* **Overloaded `operator==`:** Compares spatial equivalence within floating-point tolerance ($\epsilon = 10^{-4}$).

### 2. Expressive Mathematical Operator Overloading
* Custom operators on vectors and bounding volumes allow natural algebraic notation:
  ```cpp
  Vector3 displacement = velocity * dt;
  Vector3 newPosition  = currentPosition + displacement;
  boundingBox += frameDisplacement; // Translates entire spatial volume
  ```
* **Why?** Replaces awkward procedural helper calls (`Vector3Add(a, Vector3Scale(b, dt))`) with clear, self-documenting mathematical equations that mirror physics formulas.

> **Speaker Notes:** Clarify that operator overloading was used strictly where domain conventions dictate mathematical arithmetic (vectors and bounding boxes), enhancing clarity without introducing confusing side effects.

---

# Slide 9: Automated Verification & Testing Harnesses

The engine includes two dedicated command-line verification suites requiring **no GPU or display server**:

### 1. 16-Suite Headless Smoke Test (`./obby --smoke-test`)
* Runs in CI pipelines in $<1.0\text{ s}$ to guarantee zero regressions.
* **Key Test Suites:**
  * Vector3 arithmetic & AABB spatial operator overloading
  * Substepped collision & seam crossing across adjacent boxes
  * High-speed barrier tunneling prevention
  * Coyote time (100 ms) and jump buffering (100 ms) triggers
  * Health loss, invincibility blink timing, and knockback impulses
  * Enemy stomping rebounds & deferred expiration garbage collection
  * SaveSystem file serialization and progress recovery

### 2. Analytical Level Reachability Validator (`./obby --validate`)
* Automatically parses all 20 campaign levels and verifies:
  * Spawn/finish validity and checkpoint ordering.
  * Mathematical jump gap reachability (all horizontal platform gaps satisfy $d \le 5.60\text{ m}$).

> **Speaker Notes:** Emphasize that automated testing is rare in student game projects. Our 16 headless test suites prove that the core physics, math, and OOP hierarchies function correctly independently of the visual renderer.

---

# Slide 10: Summary & Anticipated Viva Q&A

### Summary of Project Outcomes
* **Comprehensive OOP:** Demonstrated Abstraction, Encapsulation, Multi-Level & Hierarchical Inheritance, Polymorphism, Factory & State Patterns, RAII, and Operator Overloading.
* **Deterministic Simulation:** 120 Hz fixed-step physics with substepping, coyote time, and platform carrying.
* **Dual-Mode Engine:** Seamless orbital 3D platforming and locked-axis 2D side-scrolling in one codebase.
* **Code Cleanliness:** Compiles with zero warnings under `-Wall -Wextra -pedantic -Werror`.

### Top 3 Anticipated Viva Questions & Answers:
1. **Q: Why use `std::unique_ptr<Entity>` in the Level vector instead of raw pointers?**  
   *Ans:* Enforces exclusive ownership and automatic RAII destruction. When a level unloads, all heap entities are freed automatically with zero risk of memory leaks.
2. **Q: Why use a fixed timestep (120 Hz) instead of variable `GetFrameTime()`?**  
   *Ans:* Variable timesteps produce non-deterministic physics, causing platform clipping and inconsistent jump trajectories on monitors with different refresh rates.
3. **Q: How does the State Pattern improve menu/gameplay flow?**  
   *Ans:* It replaces monolithic conditional logic with modular state objects (`PlayingState`, `PauseState`), enabling modal stacking and isolated lifecycle management.
