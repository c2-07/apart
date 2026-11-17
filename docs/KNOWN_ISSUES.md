# Known Issues, Limitations & Roadmap

**Branch:** `2d-version`  
**Document:** Technical Limitations & Future Engineering Roadmap  

---

## 1. Known Technical Limitations

### 1.1 Audio Playback in Headless Environments
- **Observation:** When running `--smoke-test` or `--validate` on a headless server, remote SSH terminal, or container without physical audio hardware, miniaudio outputs:
  ```
  WARNING: AUDIO: Failed to initialize playback device
  [SoundBank] Audio device unavailable (running silent).
  ```
- **Analysis:** This is an expected hardware constraint, not a software bug. `SoundBank` intercepts audio device failures gracefully, setting `m_initialized = false` and muting procedural waveforms so that tests and headless builds continue uninterrupted.

### 1.2 Fixed Jump Velocity (No Variable Cutoff)
- **Observation:** Pressing the jump key always imparts full launch velocity ($11.489\text{ m/s}$), achieving the maximum $2.2\text{ m}$ apex regardless of whether `Space` is tapped or held.
- **Analysis:** This matches classic fixed-impulse 3D obby physics (ensuring predictable par jump distances), but lacks the variable jump height common in precision 2D platformers.

### 1.3 Moving Platform Horizontal Crush Resolution
- **Observation:** If a moving platform travels horizontally into a standing player who is backed against an immovable wall, the platform passes through while the wall collision solver halts the player, causing the player to be pushed off the edge of the moving platform.
- **Analysis:** The collision system uses kinematic platform displacement rather than a continuous swept-volume solver with squish-death physics.

### 1.4 Coordinate Alignment in Hybrid 2D/3D Levels
- **Observation:** The circle-vs-box overlap check in `Saw::checkCircleOverlap` evaluates 3D Euclidean distance:
  $$(\Delta x)^2 + (\Delta y)^2 + (\Delta z)^2 \le r^2$$
- **Analysis:** In 2D levels where all objects reside at $z = 0$, $\Delta z = 0$ and the check is an exact circle-rectangle intersection. In 3D levels, if a saw is placed at a different $Z$ depth than the player's path, it will not hit the player. Level authors must align track $Z$-coordinates with platform $Z$-coordinates.

---

## 2. Unverified Subjective Areas

Due to testing in automated command-line environments without physical human peripherals, the following subjective factors require manual playtesting:
1. **Mouse Orbit Sensitivity on Non-Standard Displays:** The default sensitivity factor (`0.18f`) may feel fast or slow on ultra-high-DPI mouse sensors (e.g., 16,000 DPI).
2. **Perceived Audio Loudness:** Procedural waveforms (square wave jumps, sine wave chimes, noise bursts) have not been normalized across professional studio monitors.

---

## 3. Future Engineering Roadmap

- [ ] **Variable Jump Cutoff:** Terminate upward vertical velocity early if `Space` is released while $v_y > 0$.
- [ ] **Native Gamepad Support:** Add analog thumbstick camera control and XInput/DualShock rumble haptics.
- [ ] **Multi-Layer Parallax Backgrounds:** Render distant scrolling background planes in 2D side-scrolling levels.
- [ ] **In-Game Level Editor:** Allow placing platforms, hazard saws, bump blocks, and checkpoints directly through a 3D gizmo interface.
- [ ] **Time Trial Leaderboards:** Add SQLite or web REST API integration for global time-trial speedrunning leaderboards.
