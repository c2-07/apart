# Testing & Verification Guide

**Branch:** `2d-version`  
**Document:** Test Suites, Assertion Specifications & Expected Outputs  
**Test Harness:** Built-in Headless Test Runner (`--smoke-test` and `--validate`)

---

## 1. Testing Philosophy & Strategy

The project features a **two-tier automated testing strategy**:
1. **Headless Smoke Test Harness (`--smoke-test`):** 16 independent test suites exercising core physics invariants, jump assists, puzzle logic, ghost playback, save file serialization, hearts, autonomous enemy AI, stomp detection, bump blocks, and saw hitboxes without requiring an active graphical window or audio device.
2. **20-Level Reachability & Safety Validator (`--validate`):** A graph-based Breadth-First Search (BFS) pathfinding solver that computes horizontal and vertical jump distances between all level nodes, guaranteeing that every level is mathematically completable, that Walkers do not start inside solid walls, that Saws never cross spawn or checkpoints, and that BumpBlocks have clear head-space from below.

---

## 2. Headless Smoke Test Breakdown

The smoke test is located in `RunSmokeTest()` in [`src/main.cpp`](../src/main.cpp) and validates 16 critical milestone subsystems:

| Test Suite | Systems Tested | Assertions Verified |
|---|---|---|
| **1. Gravity & Landing** | Fixed-timestep gravity integration | Avatar falls from $y=5\text{ m}$, lands on ground at $y=0\text{ m}$, zeroing downward velocity and setting `isGrounded = true`. |
| **2. Flush Seam Crossing** | Ground seam collision resolution | Avatar runs across 5 adjacent flush boxes without snagging on seam edges, reaching $x > 16.0\text{ m}$. |
| **3. Wall Tunneling Prevention**| High-speed lateral collision | Avatar moving at $50\text{ m/s}$ impacts a solid wall; substepping stops avatar cleanly at $x = 7.5\text{ m}$ with zero penetration. |
| **4. Thin Platform Tunneling**| High-speed vertical collision | Avatar dropping from $y=20\text{ m}$ onto a $0.05\text{ m}$ thin box lands on the top face at $y=0\text{ m}$ without tunneling through. |
| **5. Coyote Time** | Ledge departure jump assist | Avatar walking off an edge into open air successfully fires a jump at $v_y = 11.239\text{ m/s}$ within the $0.10\text{ s}$ window. |
| **6. Jump Buffering** | Pre-landing input assist | Jump pressed $0.08\text{ s}$ prior to touching down automatically triggers on the first grounded tick. |
| **7. Camera Occlusion** | Spherical orbit raycast pull-in | Camera detects an obstructing wall between eye and player, pulling in distance from $10.0\text{ m}$ to $<4.0\text{ m}$. |
| **8. Timed Route Puzzle** | Button & Door state synchronization | Stepping on button 0 activates the switch and opens door 0 in `puzzle_timed.lvl`. |
| **9. Ghost-Clone Playback** | Trajectory recording & playback | 120-frame recorded ghost clone depresses button 0 during autonomous playback, opening door 0 for the player. |
| **10. 20-Level Campaign Validator** | Graph BFS reachability | All 20 campaign levels are loaded and verified mathematically reachable under par kinematic jump bounds. |
| **11. SaveData Serialization** | File I/O persistence | Serializes chosen color, volume, sensitivity, invert-Y, and level stars to `test_savegame.txt`, reloads, and asserts identical values. |
| **12. Hearts & Health System** | Damage, invincibility, respawn | Damage deducts 1 heart and applies knockback; invincibility prevents damage; 3rd hit respawns with 3 hearts; checkpoints heal; spikes kill instantly. |
| **13. Enemy Hierarchy & Stomping** | Walker, Flyer, Stomp, Pruning | Walker turns at walls, ledges, and range limits. Stomping an enemy from above bounces player at $9\text{ m/s}$ and flattens enemy; defeated enemies are pruned via `std::remove_if`. |
| **14. Bump Blocks** | Head impact, coins, emptiness | Jumping into block from below releases 1 coin and transitions into empty gray block. Hitting from side or above does nothing; player cannot jump through. |
| **15. Saw Blade Hazards** | Kinematics & circular hitboxes | Saw blade inflicts damage on circle-vs-box overlap, causes zero damage when clear, translates along track, and rotates continuously. |
| **16. 2D Showcase Levels** | Level format & 2D validation | Validates that `level_2d_1.lvl` and `level_2d_2.lvl` parse correctly and pass reachability. |

---

## 3. How to Run the Tests

```bash
# Run headless smoke test
./build/obby --smoke-test

# Run level validator
./build/obby --validate
```

### Expected Output (`--smoke-test`)
```
[SMOKE-TEST] Starting headless physics and logic verification...
WARNING: AUDIO: Failed to initialize playback device
[SoundBank] Audio device unavailable (running silent).
[SMOKE-TEST] Landed Y: 0, Grounded: 1
[SMOKE-TEST] Testing seam crossing across adjacent boxes...
[SMOKE-TEST] Seam test passed: reached X = 16.0268
[SMOKE-TEST] Testing high-speed wall collision...
[SMOKE-TEST] Wall impact stopped at X = 7.5 (expected 7.5)
[SMOKE-TEST] Testing thin platform tunneling prevention...
[SMOKE-TEST] Landed on thin platform at Y = 0, Grounded: 1
[SMOKE-TEST] Testing coyote time...
[SMOKE-TEST] Coyote jump velocity Y = 11.2391 m/s
[SMOKE-TEST] Testing jump buffering...
[SMOKE-TEST] Buffered jump fired: YES (vel Y = 11.2391 m/s)
[SMOKE-TEST] Testing camera obstacle pull-in...
[SMOKE-TEST] Camera pulled in to distance: 2.55003 m (target was 10.0)
[SMOKE-TEST] Testing Puzzle A: Timed Route (puzzle_timed.lvl)...
[SMOKE-TEST] Testing Puzzle B: Ghost-Clone Puzzle (puzzle_ghost.lvl)...
[SMOKE-TEST] Running Milestone 7 Reachability Validator on all 20 levels...
...
[SMOKE-TEST] All 20 levels passed reachability verification!
[SMOKE-TEST] Testing SaveData file serialization...
[SMOKE-TEST] SaveData file serialization passed cleanly!
[SMOKE-TEST] Testing Health & Hearts System...
[SMOKE-TEST] Hearts & Health System verified successfully!
[SMOKE-TEST] Testing Enemy Hierarchy (Walker & Flyer)...
[SMOKE-TEST] Enemy Hierarchy verified successfully!
[SMOKE-TEST] Testing Bump Blocks...
[SMOKE-TEST] Bump Blocks verified successfully!
[SMOKE-TEST] Testing Saw Blade Hazards...
[SMOKE-TEST] Saw Blade Hazards verified successfully!
[SMOKE-TEST] Testing 2D Showcase Levels Validation...
[SMOKE-TEST] 2D Showcase Levels verified successfully!
[SMOKE-TEST] ALL MILESTONES (M1 through M8) VERIFIED AND PASSED!
```

---

## 4. What is Not Covered by Automated Tests

While the automated suites verify all physical invariants and logic, the following subjective areas require interactive manual verification:
1. **Visual Aesthetic & Shaders:** Verifying subjective lighting contrast, platform cap color readability, and drop shadow blending on physical display panels.
2. **Audio Hardware Output:** Confirming audio clarity and volume balance through physical headphones or stereo speakers.
3. **Player Ergonomics:** Subjective evaluation of mouse look sensitivity, thumbstick deadzones, and input responsiveness.
