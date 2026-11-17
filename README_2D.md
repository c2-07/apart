# Blocky Obby Adventure - 2D Gameplay Pass

A classic platformer enhancement pass built on top of the object-oriented C++/raylib engine, adding classic side-scrolling gameplay elements while strictly maintaining OOP design principles, zero warnings under `-Wall -Wextra -pedantic -Werror`, and 100% backward compatibility with all 20 levels.

---

## 1. New Gameplay Elements

### 1. Hearts & Damage System
- **Player Health:** The player has a maximum of 3 hearts (`Constants::MAX_HEARTS`).
- **Damage & Invincibility:** Taking damage subtracts 1 heart, grants 1.5 seconds of invincibility with visual blinking (rendered every 0.1 s), and inflicts horizontal knockback away from the hazard with an upward hop.
- **Respawn & Healing:** At 0 hearts, the player respawns at the last activated checkpoint with hearts fully restored. Activating any checkpoint also instantly restores all hearts to full. Instant-kill hazards (spikes, deep pits) continue to trigger immediate respawn.
- **HUD Indicator:** Real-time heart icons (full red hearts, empty dark-gray outlines) are displayed at the top of the HUD panel, animating with a subtle physical screen shake whenever damage is sustained.

### 2. Autonomous Enemy Hierarchy
- **Abstract `Enemy` Base Class:** Inherits from `Entity`. Encapsulates shared facing direction, alive flag, defeat animation timer (0.2 s), and standard stomp vs body damage arbitration via virtual functions (`think`, `onStomped`, `onHitPlayer`).
- **`Walker` (Ground Patrol):** Paces horizontally at 2.0 m/s. Autonomous ray-probes detect walls ahead and platform ledges beneath feet, automatically reversing direction before falling. Also supports an optional patrol range boundary.
- **`Flyer` (Aerial Hazard):** Moves horizontally while oscillating vertically in a continuous sine wave (`sin(t * waveSpeed) * amplitude`). Unaffected by gravity and solid geometry.
- **Stomping & Safe Deferred Removal:** Stomping an enemy from above (falling downward with feet above the enemy's vertical midpoint) defeats the enemy, flattens and fades its model over 0.2 s, and bounces the player upward at 9.0 m/s (`Constants::STOMP_BOUNCE`). Defeated enemies are safely purged from the level vector after the update tick using the `std::remove_if` erase-remove idiom.

### 3. Interactive Bump Blocks (`BumpBlock`)
- **Impact Mechanism:** Derived from `Entity` as a solid block. When hit from underneath by the player's head while moving upward, it executes a sinusoidal upward bounce (~0.2 m over 0.15 s) and pops a golden coin into the air.
- **Coin Collection & Visual Transition:** Adds to the player's coin counter, triggers a chime sound and particle burst. Once depleted of coins (default 1, configurable up to $N$), the block transforms into an empty neutral-gray block with no further bounces.
- **Physics Stability:** Solid collision bounds (`getBounds()`) remain fixed at resting coordinates while only the visual mesh is displaced during the bounce animation, preventing tunneling or player clipping.

### 4. Hazard Saw Blades (`Saw`)
- **Kinematics & Blade Design:** A non-solid circular blade drawn as a metallic disc with 12 rotating triangular teeth, moving back and forth between two endpoints at constant speed along a visible dark-gray track line.
- **Spatial Overlap Hitbox:** Evaluates circular contact against the player's bounding box using distance clamping, inflicting 1 heart of damage and knockback on contact.

---

## 2. Level Format Additions

New elements can be authored in `.lvl` level files (supporting both 2D side-scrolling and 3D coordinates):

```
walker x y range
flyer x y amplitude speed
bump x y [coins]
saw x1 y1 x2 y2 speed radius
```

*3D syntax is also fully supported:*
```
walker x y z range [speed]
flyer x y z amplitude waveSpeed [flySpeed] [range]
bump x y z [coins] [worldId]
saw x1 y1 z1 x2 y2 z2 speed radius
```

Dedicated showcase levels using the 2D format are located in:
- `assets/levels2d/level_2d_1.lvl`: Showcase of Walkers, Bump Blocks, Flyers, and Saws.
- `assets/levels2d/level_2d_2.lvl`: Clockwork Cavern 2D multi-coin and hazard gauntlet.

New gameplay elements have also been integrated into existing levels across all 4 worlds without redesigning them:
- `w1_l3.lvl` (Winding Canopy): Walker patrol + Bump block.
- `w2_l2.lvl` (Coral Springs): Aerial Flyer + 3-coin Bump block + Saw blade.
- `w3_l3.lvl` (Blizzard Steps): Walker patrol + Saw blade timing window.
- `w4_l2.lvl` (Clockwork Switch): Flyer aerial wave + Saw blade + Bump block.

---

## 3. Build & Verification

```bash
# Compile cleanly with -Wall -Wextra -pedantic -Werror
cmake -B build
cmake --build build

# Run headless smoke test (verifies all 16 milestone & 2D test suites)
./build/obby --smoke-test

# Run level validator (reachability, walker ranges, saw track clearings, bump block bottom clearance)
./build/obby --validate-levels
```
