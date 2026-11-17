# Data & Control Flow Pipelines

**Branch:** `2d-version`  
**Document:** System Execution & Data Flow Pipelines  

---

## 1. Startup Pipeline

The startup sequence initializes hardware abstractions, loads saved configuration, compiles custom shaders, and transitions into the primary game state:

```mermaid
sequenceDiagram
    autonumber
    participant Main as main()
    participant Platform as raylib Engine
    participant Audio as AudioManager / SoundBank
    participant Game as Game Context
    participant Save as SaveData
    participant Shader as ShaderHolder

    Main->>Main: Parse CLI flags (--smoke-test, --validate)
    alt Headless flag present
        Main->>Main: Run headless verification & exit(0)
    else Interactive GUI run
        Main->>Platform: SetConfigFlags(MSAA_4X, VSYNC, RESIZABLE)
        Main->>Platform: InitWindow(1280, 720, "Blocky Obby Adventure")
        Main->>Platform: rlSetClipPlanes(0.1, 300.0)
        Main->>Platform: SetTargetFPS(60)
        Main->>Audio: AudioManager::Init() (Synthesize waveforms)
        Main->>Game: Game::Init()
        Game->>Save: SaveData::Load("savegame.txt")
        Game->>Audio: AudioManager::SetVolume(save.volume)
        Game->>Shader: ShaderHolder("lighting.vs", "lighting.fs")
        Game->>Game: LoadLevelByIndex(0)
        Game->>Game: ChangeState(MenuState)
        Main->>Main: Enter Frame Loop
    end
```

---

## 2. Main Loop Pipeline

The game loop enforces a strict separation between rendering rate and physics simulation rate using a **fixed-timestep accumulator**:

```mermaid
flowchart TD
    StartFrame["1. Frame Start: GetFrameTime()"] --> CapDelta["2. Cap Delta: min(frameDelta, 0.1s)"]
    CapDelta --> GameUpdate["3. game.Update(frameDelta)"]
    GameUpdate --> CheckPendingState{"Pending State?"}
    CheckPendingState -- Yes --> ExecTransition["ChangeState(std::move(m_pendingState))"]
    CheckPendingState -- No --> UpdateActiveState["m_currentState->update(frameDelta)"]
    ExecTransition --> UpdateActiveState

    subgraph FixedStepLoop["4. Fixed-Timestep Physics (PlayState)"]
        Accumulate["accumulator += frameDelta"] --> CheckAccum{"accumulator >= 1/120s?"}
        CheckAccum -- Yes --> PollInput["Poll PlayerInput (WASD, Space, R)"]
        PollInput --> LevelTick["Level::Update(1/120s, player, particles)"]
        LevelTick --> ResolvePhysics["Player::updatePhysics(1/120s, input, colliders)"]
        ResolvePhysics --> DeductAccum["accumulator -= 1/120s"]
        DeductAccum --> CheckAccum
        CheckAccum -- No --> EndFixedStep["End Physics Ticks"]
    end

    UpdateActiveState --> FixedStepLoop
    EndFixedStep --> BeginRender["5. BeginDrawing() & ClearBackground(BLACK)"]
    BeginRender --> StateDraw["6. game.Draw() -> m_currentState->draw()"]
    StateDraw --> EndRender["7. EndDrawing()"]
```

---

## 3. Update Pipeline (One 120 Hz Physics Step)

Inside each $1/120\text{ s}$ tick, the game updates game entities, moves moving platforms, applies gravity, resolves collisions via substepping, arbitrates enemy combat, and checks level triggers:

```mermaid
flowchart TD
    StepStart["Start 120 Hz Tick"] --> Step0["0. Ghost Action Record / Playback Tick"]
    Step0 --> Step1["1. Expose Solid Colliders: Walker::setLevelColliders()"]
    Step1 --> Step2["2. Level Entity Updates: entity->update(dt, player)"]
    
    subgraph EntityInteractions["Entity & Combat Resolution"]
        Step2 --> MovPlat["MovingPlatform: compute displacement & transport player"]
        Step2 --> EnemyLogic["Enemy::think(dt): Walker ledge/wall turn, Flyer sine wave"]
        Step2 --> StompCheck{"Player landing on Enemy?"}
        StompCheck -- Yes: vel.y <= 0 & feet > midY --> Stomp["Stomp: Enemy flattens, Player bounces 9 m/s"]
        StompCheck -- No: Side contact --> Hurt["Damage: Player loses 1 heart, invincibility starts"]
        Step2 --> BumpCheck["BumpBlock: Hit from below -> bounce & pop coin"]
        Step2 --> SawCheck["Saw: Circle-vs-box overlap -> take damage"]
    end

    EntityInteractions --> Step3["3. Safe Pruning: remove_if(e->isExpired())"]
    Step3 --> Step4["4. Door/Button Sync: doors open if linked button is active"]
    Step4 --> Step5["5. Player Kinematics: Apply run accel & friction"]
    Step5 --> Step6["6. Jump Buffer & Coyote: trigger jump if buffered"]
    Step6 --> Step7["7. Gravity: vel.y += -30.0 * dt, clamp to -40 m/s"]
    
    subgraph SubsteppedCollision["8. Substepped AABB Collision Resolution"]
        Step7 --> SubCalc["Calculate substeps: maxMove / 0.04m"]
        SubCalc --> XAxis["Move X & resolve overlapping boxes"]
        XAxis --> ZAxis["Move Z & resolve overlapping boxes"]
        ZAxis --> YAxis["Move Y & resolve overlapping boxes (landing / ceiling)"]
    end

    SubsteppedCollision --> Step8["8. Objective Checks: FinishZone completion & KillY void respawn"]
```

---

## 4. Render Pipeline & Layering

The rendering pipeline coordinates 3D geometry, custom lighting shaders, transparent shadow/particle passes, and 2D immediate-mode UI overlays:

```mermaid
sequenceDiagram
    autonumber
    participant PlayState as PlayState::draw()
    participant Sky as Level::DrawSkybox()
    participant Shader as ShaderHolder / OpenGL
    participant World as Level::Draw()
    participant Avatar as Player::draw()
    participant FX as Shadow & Particles
    participant UI as UI::DrawHUD()

    PlayState->>Sky: Draw vertical background gradient (skyTop -> skyBottom)
    PlayState->>PlayState: BeginMode3D(camera)
    PlayState->>Shader: ApplyLightingShader(cameraEye, worldId)
    Note over Shader: Enables Lambertian diffuse lighting with ambient fill
    PlayState->>World: Draw all solid & decorative entities with edge caps
    PlayState->>Avatar: Draw blocky character limbs & head
    PlayState->>Shader: EndShaderMode()
    
    Note over FX: Disable depth writing (rlDisableDepthMask) for transparency
    PlayState->>FX: player.drawShadow(colliders) (Projected ground decal)
    PlayState->>FX: particles.Draw() (Puffs & Confetti)
    Note over FX: Re-enable depth writing (rlEnableDepthMask)
    
    PlayState->>PlayState: EndMode3D()
    
    Note over UI: 2D Screen Space Overlay
    PlayState->>UI: Draw screen fade rect (respawn / level load transitions)
    PlayState->>UI: Draw animated Level Title banner (first 2s)
    PlayState->>UI: DrawHUD (Heart shapes, shake, timer, par, checkpoints, coins, deaths)
    PlayState->>UI: Draw control hint bar (fades after 8s)
    PlayState->>UI: DrawFPS (if F3 toggled)
```

---

## 5. Level Loading & Parsing Pipeline

Levels are loaded via a robust parsing pipeline that turns line-based tokens into entity objects with detailed syntax diagnostics:

```mermaid
flowchart TD
    FileOpen["Open level file (.lvl)"] --> ReadLine["Read line & increment lineNum"]
    ReadLine --> FilterComment{"Empty or comment (#)?"}
    FilterComment -- Yes --> CheckEOF{"End of file?"}
    FilterComment -- No --> Tokenize["Extract command token & parameters"]
    
    Tokenize --> CheckGlobal{"Global token?"}
    CheckGlobal -- name / world / killY / par / spawn --> SetMetadata["Store level global metadata"]
    CheckGlobal -- No: Entity token --> CallFactory["EntityFactory::createEntity(token, ss, worldId)"]
    
    CallFactory --> Instantiation{"Known entity syntax?"}
    Instantiation -- box / mover / crumble / spikes / ... --> MakeObj["Instantiate std::unique_ptr<Entity>"]
    Instantiation -- walker / flyer / bump / saw --> Make2DObj["Instantiate 2D platformer entity"]
    Instantiation -- Corrupted syntax --> ThrowError["throw LevelParseError(description)"]
    
    MakeObj --> TrackObservers["Store in m_entities; track observer pointers (buttons, doors, checkpoints)"]
    Make2DObj --> TrackObservers
    TrackObservers --> CheckEOF
    
    CheckEOF -- No --> ReadLine
    CheckEOF -- Yes --> Success["Log success: loaded N entities"]
    
    ThrowError --> CatchError["Catch LevelParseError: log filename:lineNum & return false"]
```

---

## 6. Game State Transition Graph

State transitions are mediated by the `Game` context using the **State Pattern**:

```mermaid
stateDiagram-v2
    [*] --> MAIN_MENU : Startup

    MAIN_MENU --> PLAYING : Start Game / Continue
    MAIN_MENU --> LEVEL_SELECT : Select Level
    MAIN_MENU --> CHARACTER_SELECT : Select Character
    MAIN_MENU --> SETTINGS : Open Settings
    MAIN_MENU --> [*] : Quit Game

    LEVEL_SELECT --> PLAYING : Pick Level
    LEVEL_SELECT --> MAIN_MENU : Back

    CHARACTER_SELECT --> MAIN_MENU : Save & Back

    SETTINGS --> MAIN_MENU : Back (from Main Menu)
    SETTINGS --> PAUSED : Back (from In-Game Pause)

    PLAYING --> PAUSED : Press Escape
    PAUSED --> PLAYING : Resume Game
    PAUSED --> PLAYING : Restart Level
    PAUSED --> SETTINGS : Open Settings
    PAUSED --> LEVEL_SELECT : Level Select
    PAUSED --> MAIN_MENU : Exit to Menu

    PLAYING --> RESULTS : Step into FinishZone
    RESULTS --> PLAYING : Next Level / Retry
    RESULTS --> LEVEL_SELECT : Level Select
    RESULTS --> MAIN_MENU : Main Menu
```

---

## 7. Save and Persistence Pipeline

Game data is persisted in a simple plain-text key-value format in `savegame.txt`:

```mermaid
flowchart LR
    subgraph DataStructures["In-Memory State"]
        ShirtCol["chosenColor (int)"]
        Vol["volume (float)"]
        Sens["sensitivity (float)"]
        Invert["invertY (bool)"]
        Unlocks["unlockedWorlds (int)"]
        Scores["levels[20] (bestTime, stars)"]
    end

    subgraph SaveTrigger["Persistence Triggers"]
        Complete["Level Completed"]
        Quit["Game::Shutdown()"]
    end

    subgraph FileDisk["Disk Storage: savegame.txt"]
        TextFile["# Blocky Obby Adventure Save File\ncolor 0\nvolume 0.8\nsens 0.18\ninverty 0\nunlocked_worlds 2\nlevel 0 22.4 3\n..."]
    end

    SaveTrigger --> SaveData::save
    DataStructures --> SaveData::save
    SaveData::save --> TextFile
    TextFile --> SaveData::load
    SaveData::load --> DataStructures
```

---

## 8. Ghost-Clone Recording and Playback Pipeline

The ghost-clone system records player movement frames into an in-memory trajectory array and replays it as an interactive entity:

```mermaid
sequenceDiagram
    autonumber
    participant Player as Player Input
    participant Level as Level Context
    participant Ghost as GhostRecorder
    participant Button as Button Entity

    Note over Level: Player presses 'G' key
    Level->>Ghost: StartRecording()
    loop Every 120 Hz tick (up to 20s)
        Level->>Ghost: RecordTick(playerPos, yaw, isGrounded)
        Ghost->>Ghost: Store FrameData in std::vector
    end
    Note over Level: Player presses 'G' again
    Level->>Ghost: StopRecording() & Reset player to spawn
    Level->>Ghost: StartPlayback()
    loop Every 120 Hz tick
        Level->>Ghost: UpdatePlayback()
        Ghost->>Ghost: Lerp position & compute ghost AABB
        Level->>Button: Check ghostAABB.intersects(buttonBounds)
        alt Ghost touches button
            Button->>Button: setGhostTouching(true) -> Button activates!
        end
    end
```
