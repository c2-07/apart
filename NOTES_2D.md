# Blocky Obby Adventure - 2D Gameplay Elements Architecture & Design

## 1. Overview & Architecture Plan

This document describes the design and integration plan for the new gameplay elements added to the `2d-version` branch of **Blocky Obby Adventure**:
1. **Hearts System (Health & Invincibility)**
2. **Enemy Hierarchy (`Enemy` base class, `Walker`, `Flyer`)**
3. **Interactive Bump Blocks (`BumpBlock`)**
4. **Hazard Saw Blades (`Saw`)**

Every new element adheres strictly to the existing Object-Oriented Programming (OOP) architecture:
- Abstract base class `Entity` is extended with clean virtual contracts.
- Concrete entities are instantiated through the static factory `EntityFactory::createEntity`.
- Game loops update and draw all elements polymorphically via base pointers (`std::unique_ptr<Entity>`) without dynamic casts or type switches.
- Safe deferred removal of defeated enemies is achieved using the `std::remove_if` idiom after the update tick.

---

## 2. Planned File Modifications & Placements

### New Classes and Files
- `src/enemy.h`, `src/enemy.cpp`: Abstract base class `Enemy : public Entity` with pure virtual `think(float dt)`, virtual `onStomped()`, and virtual `onHitPlayer(Player&)`.
- `src/walker.h`, `src/walker.cpp`: Derived concrete class `Walker : public Enemy` implementing horizontal patrol, wall detection, ledge detection, and range boundaries.
- `src/flyer.h`, `src/flyer.cpp`: Derived concrete class `Flyer : public Enemy` implementing sinusoidal wave flight and range turnaround.
- `src/bump_block.h`, `src/bump_block.cpp`: Derived concrete class `BumpBlock : public Entity` implementing bottom-impact detection, bounce animation, coin pop release, and empty block state.
- `src/saw.h`, `src/saw.cpp`: Derived concrete class `Saw : public Entity` implementing linear track interpolation, circular teeth rotation, and circle-vs-AABB damage contact.

### Existing Files Modified
- `src/constants.h`: Add constants for hearts, invincibility duration, knockback velocities, enemy speeds, stomp bounce velocity, bump block animation times, and saw metrics.
- `src/player.h`, `src/player.cpp`: Add private heart counters, invincibility timers, blink toggle, and public methods `takeDamage(const Vector2&)`, `heal(int)`, `isInvincible()`, `getHearts()`, `resetHearts()`.
- `src/checkpoint.cpp`: Trigger `player.resetHearts()` upon checkpoint activation.
- `src/entity.h`: Add `virtual bool isActive() const { return true; }` for polymorphic deferred lifecycle management.
- `src/level.h`, `src/level.cpp`: Add safe deferred removal of inactive entities via `std::remove_if`, and expose level colliders for enemy wall/ledge queries.
- `src/entity_factory.h`, `src/entity_factory.cpp`: Parse `walker`, `flyer`, `bump`, and `saw` level tokens and instantiate corresponding entities.
- `src/ui.h`, `src/ui.cpp`: Update `DrawHUD` to render heart shapes (full red, empty dark outline) with a damage shake animation.
- `src/game.h`, `src/play_state.cpp`: Route player heart loss events to trigger HUD heart shake animation.
- `src/validator.h`, `src/validator.cpp`: Add validation rules for Walker ranges/solid interpenetration, Saw track clearings, and BumpBlock bottom clearance.
- `src/main.cpp`: Extend headless `--smoke-test` to comprehensively verify hearts, invincibility, 3rd-hit respawn, Walker wall/ledge turning, stomping bounce, BumpBlock hit from below, and Saw damage contact.
- `CMakeLists.txt`: Include new source files in build target.
- `OOP_CONCEPTS.md`: Add Enemy hierarchy, safe-removal idiom, and updated UML diagrams.
- `README_2D.md`: User-facing documentation for 2D gameplay additions.

---

## 3. Implementation Steps & Verification Strategy

1. **Step 1:** Implement Player hearts, invincibility, blink rendering, knockback, and HUD heart icons with shake. Add smoke test assertions and verify.
2. **Step 2:** Implement abstract `Enemy`, `Walker`, `Flyer`, stomp logic, and `std::remove_if` cleanup in `Level`. Add smoke test assertions and verify.
3. **Step 3:** Implement `BumpBlock`, upward impact trigger, coin release, and empty state. Add smoke test assertions and verify.
4. **Step 4:** Implement `Saw`, continuous rotation, track movement, and circle overlap damage. Add smoke test assertions and verify.
5. **Step 5:** Populate elements across at least 4 level files, expand `--validate-levels`, and verify reachability and syntax compliance.
6. **Step 6:** Update `OOP_CONCEPTS.md`, Mermaid diagrams, and `README_2D.md`.

---

## 4. Design Decisions & Implementation Records

1. **Encapsulated Entity Lifecycle & Safe Deletion:**
   - Introduced `virtual bool isExpired() const` on `Entity` (default `false`) and overridden in `Enemy` (`return !m_isAlive && (!m_isFlattened || m_flattenTimer <= 0.0f);`).
   - `Level::Update()` executes `m_entities.erase(std::remove_if(..., [](const auto& e){ return e->isExpired(); }), m_entities.end())` strictly after iterating through entities.
   - This prevents iterator invalidation while allowing the 0.2 s flattening and fading animation to run to completion before memory is freed.

2. **BumpBlock Physics vs. Render Decoupling:**
   - `BumpBlock::getBounds()` strictly returns the resting AABB `AABB::fromCenterSize(m_position, m_size)`.
   - The upward 0.2 m bounce is applied only as a visual offset in `BumpBlock::draw()`. This ensures that collision resolution against the player remains rock-solid without clipping or tunneling.

3. **Autonomous Navigation Probing:**
   - `Walker` autonomous patrolling uses `Walker::setLevelColliders()` to query level solids without tight coupling to `Level`.
   - Wall turnaround checks a forward probe AABB `(size.x * 0.5 + 0.08)`.
   - Ledge turnaround checks a downward probe point `feetY - 0.20f` situated `0.15 m` ahead of the front foot; if no supporting solid is detected, the walker reverses direction.

4. **Parametric Saw Hitbox:**
   - `Saw::checkCircleOverlap()` clamps the saw center to the player's bounding box and computes the squared Euclidean distance against `radius * radius`.
   - In 2D plane ($Z$ aligned), this is exact circle-vs-box collision; in 3D, it evaluates sphere-vs-box, ensuring consistent and fair hitboxes in all perspectives.

5. **Verification Results:**
   - Built cleanly with zero warnings under `-Wall -Wextra -pedantic -Werror`.
   - Headless smoke test (`./build/obby --smoke-test`) passed all 16 milestone and 2D test suites:
     - Section 12: Hearts, invincibility, 3rd-hit respawn, checkpoint full heal, spike instant kill.
     - Section 13: Walker wall turn, ledge turn, patrol range turn, stomp bounce & defeat, side contact damage, and `std::remove_if` deferred removal.
     - Section 14: BumpBlock upward hit coin pop, empty block state, side/top inaction, and solid ceiling blocking.
     - Section 15: Saw blade damage on overlap, no damage when clear, track interpolation, and continuous rotation.
     - Section 16: Validation and reachability of 2D showcase levels (`level_2d_1.lvl`, `level_2d_2.lvl`).
   - Full 20-level validator suite (`./build/obby --validate-levels` and `--validate`) passed 20/20 levels with zero warnings.
