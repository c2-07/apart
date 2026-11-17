# Build & Run Guide

**Branch:** `2d-version`  
**Document:** Toolchain Setup, Compilation & Execution  
**Platforms:** macOS, Linux, Windows

---

## 1. System Prerequisites

Before building, verify that the following toolchains are installed on your host system:

- **CMake:** Version 3.20 or newer (`cmake --version`).
- **C++ Compiler:** Supporting full C++17 language standards:
  - **macOS:** Apple Clang 12.0+ or LLVM Clang.
  - **Linux:** GCC 9.0+ or Clang 10.0+.
  - **Windows:** MSVC v142+ (Visual Studio 2019 / 2022) or MinGW-w64.
- **Git:** Required by CMake `FetchContent` to clone and pin raylib 5.5.
- **Graphics Drivers:** OpenGL 3.3 Core Profile compatible GPU driver.

### Linux Package Dependencies
On Debian, Ubuntu, or Linux Mint, install the required X11 and OpenGL development headers:
```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential git \
    libgl1-mesa-dev libglu1-mesa-dev xorg-dev libx11-dev \
    libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

---

## 2. Multi-Platform Build Instructions

### macOS (Terminal)
```bash
# 1. Configure the build directory
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Compile with all CPU cores
cmake --build build -j$(sysctl -n hw.ncpu)
```

### Linux (Bash)
```bash
# 1. Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Build
cmake --build build -j$(nproc)
```

### Windows (Visual Studio / PowerShell)
```powershell
# 1. Configure using Visual Studio 2022 Generator
cmake -B build -G "Visual Studio 17 2022" -A x64

# 2. Build the Release target
cmake --build build --config Release
```

---

## 3. How to Run the Game

### Interactive GUI Mode
Launch the compiled binary directly from the project root so it locates the `assets/` directory:

```bash
# macOS / Linux
./build/obby

# Windows
.\build\Release\obby.exe
```

---

## 4. Command-Line Arguments & Automation Modes

The executable includes built-in headless testing and validation flags that execute without launching a graphical window or requiring an audio device:

### 1. `--smoke-test`
Executes the comprehensive 16-suite headless verification pipeline verifying physics substepping, coyote time, jump buffering, timed puzzles, ghost playback, hearts, knockback, walker AI, bump block impacts, and saw hitboxes:
```bash
./build/obby --smoke-test
```
**Expected Output:**
```
[SMOKE-TEST] Starting headless physics and logic verification...
...
[SMOKE-TEST] ALL MILESTONES (M1 through M8) VERIFIED AND PASSED!
```
*Process exits with return code `0` on success.*

### 2. `--validate` or `--validate-levels`
Runs the automated Breadth-First Search (BFS) reachability validator across all 20 campaign levels plus 2D showcase levels. It verifies maximum jump gap feasibility, confirms Walker ranges are non-zero and start outside solid walls, verifies that Saw paths do not intersect spawn or checkpoints, and ensures BumpBlock undersides are reachable:
```bash
./build/obby --validate
```
**Expected Output:**
```
================================================================================
                 LEVEL VALIDATOR: 20-LEVEL REACHABILITY SUITE                  
================================================================================
File        Level Name                  World   Boxes   Movers  Max Gap   Status  
--------------------------------------------------------------------------------
w1_l1.lvl   Meadow Steps                1       28      0       4.20m     PASS    
w1_l2.lvl   Gentle Slopes               1       23      0       5.50m     PASS    
...
w4_l5.lvl   Master Generator            4       17      3       10.0m     PASS    
================================================================================
```
*Process exits with return code `0` if all levels pass.*

---

## 5. Troubleshooting & FAQ

### Issue: "WARNING: AUDIO: Failed to initialize playback device"
- **Cause:** Running in a headless server environment, container, or VM without a physical audio output device.
- **Resolution:** No action needed. `SoundBank` detects uninitialized audio hardware and automatically enters a silent fallback mode, allowing gameplay and tests to continue normally.

### Issue: "Window could not be initialized. Running smoke test instead."
- **Cause:** Running on a machine without an active `$DISPLAY` or Wayland server.
- **Resolution:** `main()` automatically intercepts window creation failures and falls back to running `--smoke-test`, returning exit code 0 if all tests pass.

### Issue: FetchContent git clone failure
- **Cause:** Firewalls blocking HTTPS access to GitHub.
- **Resolution:** Ensure outgoing connections to `https://github.com/raysan5/raylib.git` on port 443 are allowed.
