# Level File Format Specification

**Branch:** `2d-version`  
**Document:** Grammar, Tokens, Coordinate System & Level Catalog  
**Parser Source:** [`src/level.cpp`](../src/level.cpp), [`src/entity_factory.cpp`](../src/entity_factory.cpp)

---

## 1. Syntax Overview & Coordinate System

### 1.1 Format Fundamentals
- **File Extension:** `.lvl`
- **Encoding:** ASCII / UTF-8 plain text.
- **Line Structure:** Token-based, whitespace-delimited (`space` or `tab`).
- **Comments:** Any line beginning with `#` (with optional leading whitespace) is treated as a comment and ignored.
- **Blank Lines:** Empty or whitespace-only lines are ignored.
- **Coordinate Conventions:**
  - **$+Y$:** Upwards (altitude/height in meters).
  - **$+X$:** Rightwards (horizontal lateral axis).
  - **$-Z$:** Primary forward gameplay progression axis in 3D campaign levels.
  - **Origin $(0, 0, 0)$:** World center.
  - **Units:** Metric meters ($1.0\text{ unit} = 1.0\text{ meter}$).

---

## 2. Command Grammar & Entity Tokens

### 2.1 Level Metadata Directives

```
name <Level Title>
world <worldId: 1..4>
killY <bottomY: float>
par <targetSeconds: float>
spawn <x: float> <y: float> <z: float>
```

- **`name`:** Display title shown on the HUD and in menus (e.g. `name Winding Canopy`).
- **`world`:** Theme identifier selecting skybox gradients, fog, and platform color palettes:
  - `1`: Meadow (lush green, blue skies).
  - `2`: Ocean Lagoon (sandy beige, cyan waters, coral accents).
  - `3`: Ice Peak (frost white, cold blue, glacier accents).
  - `4`: Clockwork Factory (industrial steel, hazard orange, dark iron).
- **`killY`:** Altitude plane triggering immediate respawn if the player falls below it (e.g. `killY -15.0`).
- **`par`:** Target completion time in seconds for earning a 3-star rating (e.g. `par 42.0`).
- **`spawn`:** Initial coordinates of the player avatar upon loading the level (e.g. `spawn 0 1 0`).

---

### 2.2 Entity Tokens

Every entity line is parsed by the factory method `EntityFactory::createEntity(token, ss, worldId)`:

| Token | Parameters | Description |
|---|---|---|
| **`box`** | `x y z sx sy sz [colorIndex]` | Solid static platform centered at `(x, y, z)` with size `(sx, sy, sz)`. `colorIndex` (0–3) picks from world palette. |
| **`mover`** | `x y z sx sy sz dx dy dz period [colorIndex]` | Kinematic moving platform oscillating along vector `(dx, dy, dz)` over `period` seconds. |
| **`crumble`**| `x y z sx sy sz [delay] [respawn]` | Collapsing platform that shakes for `delay` s (default 0.8s) upon touch, falls, and respawns after `respawn` s. |
| **`spikes`** | `x y z sx sy sz` | Instant-kill static hazard pad. Touching it triggers immediate death and respawn. |
| **`spinner`**| `x y z length speed rotationSpeed` | Lethal rotating obstacle bar revolving around `(x, y, z)` at `rotationSpeed` rad/s. |
| **`bounce`** | `x y z sx sy sz strength` | Elastic launch pad propelling the player upward with launch velocity `strength` m/s. |
| **`checkpoint`**| `x y z` | Waypoint pad with waving flag; updates respawn point and fully heals player hearts. |
| **`finish`** | `x y z sx sy sz` | Level completion zone; entering triggers completion, star calculation, and confetti. |
| **`button`** | `id x y z [duration]` | Pressure button linked to door `id`. Activates for `duration` seconds when stepped on. |
| **`door`** | `id x y z sx sy sz` | Retracting barrier obstacle that opens while linked button `id` is active. |
| **`walker`** | `x y range` *(2D)*<br>`x y z range [speed]` *(3D)* | Ground patrol enemy pacing at 2.0 m/s with autonomous wall and ledge detection. |
| **`flyer`** | `x y amplitude speed` *(2D)*<br>`x y z amplitude waveSpeed [speed] [range]` *(3D)* | Aerial enemy oscillating along sinusoidal vertical wave curves. |
| **`bump`** | `x y [coins]` *(2D)*<br>`x y z [coins] [worldId]` *(3D)* | Solid question block popping a coin when hit from underneath. |
| **`saw`** | `x1 y1 x2 y2 speed radius` *(2D)*<br>`x1 y1 z1 x2 y2 z2 speed radius` *(3D)* | Rotating circular saw hazard translating along a track between two endpoints. |

---

## 3. Real Level File Examples

### Example 1: 3D Platformer Level (`assets/levels/w1_l3.lvl`)
```
# World 1 - Level 3: Winding Canopy
name Winding Canopy
world 1
killY -15.0
par 42.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0

box 0 0 -6 3 1 3 1
box 4 0.5 -10 3 1 3 0
box 8 1.0 -13 3 1 3 2
bump 8 4.2 -13 1
box 12 1.5 -15 3 1 3 1
box 16 2.0 -15 3 1 3 0
box 20 2.5 -13 3 1 3 2
walker 20 3.4 -13 1.0
box 24 3.0 -10 3 1 3 1

box 29 3.0 -10 6 1 6 0
checkpoint 29 3.5 -10
...
finish 2 11.5 -67 4 3 4
```

### Example 2: 2D Side-Scroller Showcase (`assets/levels2d/level_2d_1.lvl`)
```
# 2D Side-Scroller Showcase Level 1
name 2D Adventure Gateway
world 1
killY -10.0
par 30.0

spawn 0 1 0
box 0 -0.5 0 8 1 4 0

# Bump block with coin above resting platform
box 8 0.5 0 6 1 4 1
bump 8 3.8 1

# Walker patrolling along a ledge
box 18 1.5 0 8 1 4 2
walker 18 2.4 2.5

# Checkpoint 1
box 28 2.0 0 6 1 4 0
checkpoint 28 2.5 0

# Flyer hovering over hazard gap
box 36 2.5 0 4 1 4 1
flyer 40 4.5 1.0 2.0
box 43 3.0 0 6 1 4 2

# Rotating saw blade along track
saw 46 5.0 52 5.0 2.5 0.8

# Finish Zone
box 52 3.5 0 8 1 4 3
finish 52 5.0 0 4 3 4
```

---

## 4. Error Handling & Parsing Diagnostics

If any parameter is malformed, missing, or of incorrect type, `EntityFactory` throws a `LevelParseError`:

```cpp
// Example from src/entity_factory.cpp
} else if (type == "saw") {
    std::vector<float> args;
    float val;
    while (ss >> val) args.push_back(val);
    if (args.size() == 6) {
        return std::make_unique<Saw>(Vec3{ args[0], args[1], 0.0f }, Vec3{ args[2], args[3], 0.0f }, args[4], args[5]);
    } else if (args.size() >= 8) {
        return std::make_unique<Saw>(Vec3{ args[0], args[1], args[2] }, Vec3{ args[3], args[4], args[5] }, args[6], args[7]);
    }
    throw LevelParseError("Invalid saw entity parameters");
}
```

In `Level::LoadFromFile`, the exception is caught and logged with the exact file name and line number, preventing application crashes:

```
[Level Parser Exception] assets/levels/w1_l3.lvl:14: Invalid saw entity parameters
```

---

## 5. Complete Campaign Level Catalog

### World 1: Meadow (Spring / Green Valley)
1. **`w1_l1.lvl` ("Meadow Steps"):** Gentle introduction to straight and diagonal jumping across static platforms.
2. **`w1_l2.lvl` ("Gentle Slopes"):** Elevation changes, ascending steps, and basic camera panning.
3. **`w1_l3.lvl` ("Winding Canopy"):** S-curved canopy route featuring the first Walker patrol and BumpBlock.
4. **`w1_l4.lvl` ("Spiky Crossings"):** Introduction to ground spike hazards and narrow platform landings.
5. **`w1_l5.lvl` ("Meadow Apex"):** Final ascent to the peak, combining narrow pillars and long jump gaps.

### World 2: Ocean Lagoon (Tidal / Springs)
6. **`w2_l1.lvl` ("Tidal Planks"):** Introduction to moving platforms with linear lateral oscillation.
7. **`w2_l2.lvl` ("Coral Springs"):** Bounce pads launching players across coral pillars; features an aerial Flyer, 3-coin BumpBlock, and Saw hazard.
8. **`w2_l3.lvl` ("Shifting Sands"):** Multi-axis moving platforms over deep water.
9. **`w2_l4.lvl` ("High Tide Launch"):** Chained bounce pad launches combined with moving platforms.
10. **`w2_l5.lvl` ("Sunken Citadel"):** Citadel ruins with fast-moving platforms, narrow bridges, and vertical climbs.

### World 3: Ice Peak (Winter / Glacial)
11. **`w3_l1.lvl` ("Thin Ice"):** Introduction to crumble tiles that shake and collapse upon contact.
12. **`w3_l2.lvl` ("Frost Whirl"):** Introduction to lethal rotating spinner hazard arms.
13. **`w3_l3.lvl` ("Blizzard Steps"):** Combination of crumble tiles, rotating spinners, Walker patrol, and Saw blade gauntlet.
14. **`w3_l4.lvl` ("Glacial Gauntlet"):** Fast-collapsing ice platforms with alternating spinner timing.
15. **`w3_l5.lvl` ("Summit of the Frost Titan"):** Extended mountain ascent demanding precise jump pacing and spinner avoidance.

### World 4: Clockwork Factory (Industrial / Automation)
16. **`w4_l1.lvl` ("Assembly Line"):** Timed button switch opening factory security doors across conveyor platforms.
17. **`w4_l2.lvl` ("Clockwork Switch"):** Multi-stage timed door run featuring an aerial Flyer wave, Saw blade, and BumpBlock.
18. **`w4_l3.lvl` ("Hazard Core"):** Industrial hazard gauntlet combining spinners, spikes, and tight par times.
19. **`w4_l4.lvl` ("Twin Synchrony"):** Dual-door puzzle requiring high-speed sequential button triggering.
20. **`w4_l5.lvl` ("Master Generator"):** Climax level combining timed buttons, doors, moving platforms, and spinners.

### Standalone Puzzle Showcases
- **`puzzle_timed.lvl` ("Timed Dash"):** Milestone 6 single-player timed button-door obstacle run.
- **`puzzle_ghost.lvl` ("Ghost Duo"):** Milestone 6 cooperative puzzle solved via the `GhostRecorder` clone system.

### 2D Side-Scroller Showcases
- **`level_2d_1.lvl` ("2D Adventure Gateway"):** Pure 2D side-scrolling platformer introducing Walkers, BumpBlocks, Flyers, and Saws.
- **`level_2d_2.lvl` ("Clockwork Cavern 2D"):** Multi-coin bump block, dual walker pacing, flyer waves, and high-speed saw gauntlet.
