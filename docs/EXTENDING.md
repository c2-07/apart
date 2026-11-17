# Developer Extension Guide

**Branch:** `2d-version`  
**Document:** Adding Entities, Enemies, Levels, Palettes & States  
**Toolchain:** CMake 3.20+, C++17

---

## 1. How to Add a New Entity Subclass

To illustrate adding an entity, we present a minimal worked example: `SpeedPad`, an interactive boost pad that accelerates the player forward when stepped upon.

### Step 1: Create Header (`src/speed_pad.h`)
```cpp
#pragma once
#include "entity.h"

namespace obby {

class SpeedPad : public Entity {
private:
    float m_boostForce;

public:
    SpeedPad(const Vec3& position, const Vec3& size, float boostForce = 16.0f);
    virtual ~SpeedPad() override = default;

    virtual void update(float dt, Player& player) override;
    virtual void draw() const override;
    virtual void onPlayerTouch(Player& player) override;
    virtual const char* getTypeName() const override { return "SpeedPad"; }
    virtual bool isSolid() const override { return true; }
};

} // namespace obby
```

### Step 2: Implement Logic (`src/speed_pad.cpp`)
```cpp
#include "speed_pad.h"
#include "player.h"
#include "audio.h"

namespace obby {

SpeedPad::SpeedPad(const Vec3& position, const Vec3& size, float boostForce)
    : Entity(position, size, Color{ 255, 140, 0, 255 }),
      m_boostForce(boostForce) {}

void SpeedPad::update(float dt, Player& player) {
    (void)dt;
    (void)player;
}

void SpeedPad::onPlayerTouch(Player& player) {
    // Apply forward horizontal impulse when player lands on top
    Vec3 vel = player.getVelocity();
    vel.x *= 1.5f;
    vel.z *= 1.5f;
    player.setVelocity(vel);
    AudioManager::Play(SoundId::JUMP);
}

void SpeedPad::draw() const {
    Vector3 pos = m_position.toRaylib();
    Vector3 sz = m_size.toRaylib();
    DrawCube(pos, sz.x, sz.y, sz.z, m_color);
    DrawCubeWires(pos, sz.x + 0.02f, sz.y + 0.02f, sz.z + 0.02f, DarkenColor(m_color, 0.7f));
}

} // namespace obby
```

### Step 3: Register in Factory (`src/entity_factory.cpp`)
```cpp
#include "speed_pad.h"

// Inside EntityFactory::createEntity():
} else if (type == "speedpad") {
    float x, y, z, sx, sy, sz, force;
    if (ss >> x >> y >> z >> sx >> sy >> sz >> force) {
        return std::make_unique<SpeedPad>(Vec3{ x, y, z }, Vec3{ sx, sy, sz }, force);
    }
    throw LevelParseError("Invalid speedpad parameters");
}
```

### Step 4: Add to `CMakeLists.txt`
```cmake
add_executable(obby
    ...
    src/speed_pad.cpp
)
```

---

## 2. How to Add a New Enemy Type

Every enemy derives from the abstract base class `Enemy` (`src/enemy.h`), inheriting stomp detection, flatten timers, and health damage arbitration.

### Step 1: Create Class Header (`src/hopper.h`)
```cpp
#pragma once
#include "enemy.h"

namespace obby {

class Hopper : public Enemy {
private:
    float m_hopTimer;
    float m_jumpInterval;

public:
    Hopper(const Vec3& position, float interval = 1.5f);
    virtual ~Hopper() override = default;

    virtual void think(float dt) override;
    virtual void draw() const override;
    virtual const char* getTypeName() const override { return "hopper"; }
};

} // namespace obby
```

### Step 2: Implement Autonomous AI (`src/hopper.cpp`)
```cpp
#include "hopper.h"
#include "constants.h"

namespace obby {

Hopper::Hopper(const Vec3& position, float interval)
    : Enemy(position, Vec3{ 0.8f, 0.8f, 0.8f }, GREEN),
      m_hopTimer(0.0f),
      m_jumpInterval(interval) {}

void Hopper::think(float dt) {
    m_hopTimer += dt;
    if (m_hopTimer >= m_jumpInterval) {
        m_hopTimer = 0.0f;
        // Execute autonomous hop
        m_position.y += 1.0f;
    }
}

void Hopper::draw() const {
    if (!isActive()) return;
    DrawCube(m_position.toRaylib(), m_size.x, m_size.y, m_size.z, m_color);
}

} // namespace obby
```

### Step 3: Add to `EntityFactory` & `CMakeLists.txt`
Add token parsing block `else if (type == "hopper")` in `src/entity_factory.cpp` and register `src/hopper.cpp` in `CMakeLists.txt`.

---

## 3. How to Add a New Level File

### Step 1: Create `.lvl` File
Create `assets/levels/w1_l6.lvl`:
```
# World 1 - Level 6: Meadow Bonus
name Meadow Bonus
world 1
killY -15.0
par 30.0

spawn 0 1 0
box 0 -0.5 0 8 1 8 0
box 0 0 -8 4 1 4 1
walker 0 1.4 -8 1.5
bump 0 3.8 -8 1
box 0 1 -16 6 1 6 2
finish 0 2.5 -16 4 3 4
```

### Step 2: Register in File Arrays
1. In `src/game.cpp`: Add path to `LEVEL_PATHS`:
   ```cpp
   static const std::vector<std::string> LEVEL_PATHS = {
       ...
       "assets/levels/w1_l6.lvl"
   };
   ```
2. In `src/main.cpp`: Add path to `ALL_LEVEL_FILES`:
   ```cpp
   static const std::vector<std::string> ALL_LEVEL_FILES = {
       ...
       "assets/levels/w1_l6.lvl"
   };
   ```

### Step 3: Validate Reachability
Run the automated validator to ensure the new level is 100% completable:
```bash
./build/obby --validate-levels
```

---

## 4. How to Add a New World Palette

### Step 1: Define Palette Colors in `src/render_utils.cpp`
Open `src/render_utils.cpp` and extend `GetWorldPalette(int worldId)`:
```cpp
WorldPalette GetWorldPalette(int worldId) {
    WorldPalette pal;
    switch (worldId) {
        ...
        case 5: // World 5: Volcanic Core
            pal.skyTop     = Color{ 60, 20, 20, 255 };   // Dark crimson smoke
            pal.skyBottom  = Color{ 220, 75, 30, 255 };  // Fiery magma orange
            pal.fogColor   = Color{ 80, 25, 20, 255 };
            pal.fogDensity = 0.006f;
            pal.colors[0]  = Color{ 45, 45, 50, 255 };   // Obsidian basalt
            pal.colors[1]  = Color{ 70, 65, 75, 255 };   // Ash rock
            pal.colors[2]  = Color{ 235, 95, 35, 255 };  // Molten accent
            pal.colors[3]  = Color{ 140, 40, 30, 255 };  // Dark lava brick
            break;
    }
    return pal;
}
```

Any `.lvl` file with directive `world 5` will automatically render with these volcanic environmental colors.

---

## 5. How to Add a New Game State

### Step 1: Declare Enum & Class
1. In `src/game_state.h`: Add `CREDITS` to `enum class StateType`.
2. In `src/game.h`: Add `CREDITS` to `enum class AppState`.
3. Create `src/credits_state.h`:
   ```cpp
   #pragma once
   #include "game_state.h"

   namespace obby {

   class CreditsState : public GameState {
   public:
       explicit CreditsState(Game& game) : GameState(game) {}
       virtual void handleInput() override;
       virtual void update(float dt) override;
       virtual void draw() const override;
       virtual StateType getType() const override { return StateType::CREDITS; }
   };

   } // namespace obby
   ```

### Step 2: Implement UI & Transitions (`src/credits_state.cpp`)
```cpp
#include "credits_state.h"
#include "game.h"
#include "ui.h"

namespace obby {

void CreditsState::handleInput() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        m_game.RequestStateChange(StateType::MAIN_MENU);
    }
}

void CreditsState::update(float dt) {
    (void)dt;
    handleInput();
}

void CreditsState::draw() const {
    DrawText("Blocky Obby Adventure - Credits", 100, 100, 30, WHITE);
    if (UI::Button(Rectangle{ 100, 400, 200, 50 }, "Back", GRAY, LIGHTGRAY)) {
        m_game.RequestStateChange(StateType::MAIN_MENU);
    }
}

} // namespace obby
```

### Step 3: Register in State Factory (`src/game.cpp`)
In `Game::CreateState(StateType type)`:
```cpp
case StateType::CREDITS: return std::make_unique<CreditsState>(*this);
```

Add `src/credits_state.cpp` to `CMakeLists.txt` and compile.
