# Physics, Kinematics & Gameplay Model

**Branch:** `2d-version`  
**Document:** Movement, Collision, Combat & Scoring Systems  
**Constants Source:** [`src/constants.h`](../src/constants.h)

---

## 1. Kinodynamic Tuning Constants

All physical, visual, and timing metrics are defined as compile-time `constexpr` values in `namespace Constants`:

| Constant Identifier | Value | Units | Gameplay Meaning & Description |
|---|---|---|---|
| `PHYSICS_TICK_RATE` | `120.0f` | $\text{Hz}$ | Target frequency for the fixed-timestep physics update loop. |
| `PHYSICS_DT` | `1.0f / 120.0f` | $\text{s}$ | Fixed delta time per physics step ($\approx 0.008333\text{ s}$). |
| `MAX_FRAME_DELTA` | `0.1f` | $\text{s}$ | Maximum frame delta cap to prevent physics spiral of death during lag spikes. |
| `PLAYER_WIDTH` | `1.0f` | $\text{m}$ | Avatar physical width along $X$-axis. |
| `PLAYER_HEIGHT` | `2.0f` | $\text{m}$ | Avatar physical height along $Y$-axis. |
| `PLAYER_DEPTH` | `1.0f` | $\text{m}$ | Avatar physical depth along $Z$-axis. |
| `RUN_SPEED` | `8.0f` | $\text{m/s}$ | Maximum target horizontal running speed. |
| `GROUND_ACCEL` | `60.0f` | $\text{m/s}^2$ | Ground steering acceleration rate toward target input velocity. |
| `GROUND_FRICTION` | `50.0f` | $\text{m/s}^2$ | Ground braking deceleration when no directional movement is held. |
| `AIR_ACCEL` | `24.0f` | $\text{m/s}^2$ | Airborne steering acceleration rate ($40\%$ of ground acceleration). |
| `AIR_DRAG` | `2.0f` | $\text{m/s}^2$ | Airborne horizontal resistance. |
| `GRAVITY` | `-30.0f` | $\text{m/s}^2$ | Downward gravitational acceleration acting on airborne bodies. |
| `TERMINAL_VELOCITY` | `-40.0f` | $\text{m/s}$ | Maximum clamped downward falling speed. |
| `JUMP_HEIGHT` | `2.2f` | $\text{m}$ | Desired vertical jump apex above ground surface. |
| `JUMP_VELOCITY` | `11.489125f` | $\text{m/s}$ | Initial upward jump launch velocity: $\sqrt{2 \cdot \|g\| \cdot h} = \sqrt{2 \cdot 30 \cdot 2.2}$. |
| `COYOTE_TIME` | `0.1f` | $\text{s}$ | Ledge departure grace window allowing jumps after walking into air. |
| `JUMP_BUFFER_TIME` | `0.1f` | $\text{s}$ | Pre-landing input buffer window executing jumps upon touching ground. |
| `MODEL_TURN_SPEED` | `18.0f` | $\text{rad/s}$ | Smooth spherical turning speed of the player character model toward move direction. |
| `MAX_HEARTS` | `3` | count | Total maximum player health capacity. |
| `INVINCIBILITY_TIME`| `1.5f` | $\text{s}$ | Duration of damage immunity following a hit. |
| `KNOCKBACK_SPEED` | `6.0f` | $\text{m/s}$ | Horizontal knockback velocity applied away from hazard upon taking damage. |
| `KNOCKBACK_HOP` | `5.0f` | $\text{m/s}$ | Upward vertical hop velocity applied upon taking damage. |
| `HEART_SHAKE_DURATION`| `0.35f` | $\text{s}$ | Duration of HUD heart shake animation upon health loss. |
| `BLINK_INTERVAL` | `0.10f` | $\text{s}$ | Blink interval for drawing alternation during invincibility. |
| `WALKER_SPEED` | `2.0f` | $\text{m/s}$ | Horizontal patrol pacing speed of Walker enemies. |
| `FLYER_SPEED` | `2.5f` | $\text{m/s}$ | Horizontal travel speed of Flyer enemies. |
| `STOMP_BOUNCE` | `9.0f` | $\text{m/s}$ | Upward vertical impulse applied to player upon stomping an enemy. |
| `ENEMY_FLATTEN_TIME`| `0.20f` | $\text{s}$ | Duration of squash and fade animation when an enemy is defeated. |
| `BUMP_BLOCK_BOUNCE_TIME`| `0.15f` | $\text{s}$ | Duration of upward sinusoidal bounce when a BumpBlock is hit. |
| `BUMP_BLOCK_BOUNCE_HEIGHT`| `0.20f` | $\text{m}$ | Peak vertical displacement of BumpBlock bounce. |
| `COIN_POP_DURATION` | `0.40f` | $\text{s}$ | Duration of coin pop and rotation animation above a BumpBlock. |
| `COIN_POP_HEIGHT` | `0.80f` | $\text{m}$ | Peak elevation of coin pop animation. |
| `SAW_ROTATION_SPEED`| `14.0f` | $\text{rad/s}$ | Continuous angular spin speed of rotating saw blades ($\approx 2.23\text{ rev/s}$). |
| `SAW_DEFAULT_RADIUS`| `0.8f` | $\text{m}$ | Default radius of circular saw hazard. |
| `SAW_DEFAULT_SPEED` | `2.5f` | $\text{m/s}$ | Default linear translation speed of saw along its track. |

---

## 2. Jump Mechanics & Assist Systems

### 2.1 Physics Derivation
The jump impulse is derived from basic Newtonian kinematic formulas under constant gravitational acceleration $g = -30.0\text{ m/s}^2$:

$$v_{\text{jump}} = \sqrt{2 \cdot |g| \cdot h_{\text{jump}}} = \sqrt{2 \cdot 30.0 \cdot 2.2} \approx 11.4891\text{ m/s}$$

### 2.2 Coyote Time ($0.10\text{ s}$)
When `m_isGrounded` switches from `true` to `false` without jumping (e.g., walking off a ledge), `m_coyoteTimer` initializes to `Constants::COYOTE_TIME` ($0.10\text{ s}$) and counts down. A jump input received while `m_coyoteTimer > 0.0f` fires a full-height jump as if the player were still grounded.

### 2.3 Jump Buffering ($0.10\text{ s}$)
When the player presses the jump key (`Space`) while falling in the air, `m_jumpBufferTimer` initializes to `Constants::JUMP_BUFFER_TIME` ($0.10\text{ s}$) and counts down. If the player lands on a solid platform before this timer expires, the jump executes automatically on the very first grounded tick.

---

## 3. Substepped AABB Collision Resolution Algorithm

To guarantee that high-speed collisions never tunnel through thin platforms (e.g., $0.10\text{ m}$ thick) or clip through walls, `Player::updatePhysics` subdivides the frame step into discrete sub-steps:

### 3.1 Substep Count Calculation
$$\text{substeps} = \max\left(1, \left\lceil \frac{\max(|\Delta x|, |\Delta y|, |\Delta z|)}{0.04\text{ m}} \right\rceil\right)$$

This ensures that the player never moves more than $0.04\text{ m}$ per substep.

### 3.2 Axis-by-Axis Resolution Order
Within each substep, displacement is applied and resolved independently along each axis:

1. **$X$-Axis Resolution:**
   - Displace: $x \leftarrow x + v_x \cdot \Delta t_{\text{sub}}$.
   - Query all solid colliders $C$ where $Y$-bounds and $Z$-bounds overlap with a skin clearance $\epsilon = 0.01\text{ m}$:
     $$y_{\min} < C_{y,\max} - \epsilon \quad \text{and} \quad y_{\max} > C_{y,\min} + \epsilon$$
     $$z_{\min} < C_{z,\max} - \epsilon \quad \text{and} \quad z_{\max} > C_{z,\min} + \epsilon$$
   - If an overlap in $X$ occurs, snap $x$ to the collider boundary and zero $v_x$:
     $$x \leftarrow \begin{cases} C_{x,\min} - \text{halfWidth} & \text{if } v_x > 0 \\ C_{x,\max} + \text{halfWidth} & \text{if } v_x < 0 \end{cases}$$
2. **$Z$-Axis Resolution:**
   - Displace: $z \leftarrow z + v_z \cdot \Delta t_{\text{sub}}$.
   - Query overlapping colliders in $X$ and $Y$ with skin clearance $\epsilon$.
   - Snap $z$ to boundary and zero $v_z$.
3. **$Y$-Axis Resolution (Ceiling & Floor):**
   - Displace: $y \leftarrow y + v_y \cdot \Delta t_{\text{sub}}$.
   - Query overlapping colliders in $X$ and $Z$ with skin clearance $\epsilon$.
   - **Floor Impact ($v_y \le 0$):**
     $$y \leftarrow C_{y,\max}, \quad v_y \leftarrow 0, \quad \text{landedThisTick} \leftarrow \text{true}$$
   - **Ceiling Impact ($v_y > 0$):**
     $$y \leftarrow C_{y,\min} - \text{PLAYER\_HEIGHT}, \quad v_y \leftarrow 0$$

---

## 4. Moving Platform Player Transport

`MovingPlatform` inherits from `Platform` and calculates its displacement between consecutive ticks:

$$\vec{d}_{\text{frame}} = \vec{p}_{\text{next}} - \vec{p}_{\text{prev}}$$

During `MovingPlatform::update`, the platform checks whether the player is standing on its top surface:

$$\text{standingOnTop} = \text{player.isGrounded}() \land (|y_{\text{player}} - y_{\text{top}}| < 0.05\text{ m}) \land (\text{horizontal overlap in } X \text{ and } Z)$$

When `standingOnTop` is true, the player's position is displaced by the platform's exact movement:

$$\vec{p}_{\text{player}} \leftarrow \vec{p}_{\text{player}} + \vec{d}_{\text{frame}}$$

---

## 5. Combat, Health & Enemy Mechanics

### 5.1 Hearts & Invincibility
- The player begins with 3 hearts (`Constants::MAX_HEARTS`).
- Damage subtracts 1 heart, activates a $1.5\text{ s}$ invincibility timer, triggers horizontal knockback of $6.0\text{ m/s}$ away from the hazard source, and applies an upward hop of $5.0\text{ m/s}$.
- Damage requests received while `isInvincible()` is true are ignored.
- At 0 hearts, the player respawns at the latest checkpoint with hearts fully replenished. Checkpoints also fully heal the player on contact.

### 5.2 Stomp Detection vs. Body Damage
In `Enemy::update(dt, player)`:
1. Overlap is tested between `enemy.getBounds()` and `player.getBounds()`.
2. If overlapping:
   - **Stomp Condition:**
     $$y_{\text{feet}} \ge y_{\text{enemy,mid}} - 0.15\text{ m} \quad \text{and} \quad v_{y,\text{player}} \le 0.0\text{ m/s}$$
     If satisfied:
     - `enemy.onStomped()` marks the enemy defeated and starts a $0.20\text{ s}$ flattening animation.
     - Upward stomp impulse: $v_{y,\text{player}} \leftarrow 9.0\text{ m/s}$ (`Constants::STOMP_BOUNCE`).
     - Player receives zero damage.
   - **Body Contact:**
     - If stomp condition is not satisfied, `enemy.onHitPlayer(player)` is called, inflicting 1 heart of damage and knockback.

### 5.3 Autonomous Walker AI
`Walker` paces horizontally at $2.0\text{ m/s}$. Autonomous ray-probes run every tick:
1. **Wall Probe:** Probes a small AABB $0.08\text{ m}$ ahead of the front face. If it intersects any solid collider, the Walker reverses facing direction ($\text{facing} \leftarrow -\text{facing}$).
2. **Ledge Probe:** Probes a point $0.15\text{ m}$ ahead of the front foot and $0.20\text{ m}$ below feet height. If no solid collider contains this point, the Walker reverses facing direction before falling.
3. **Patrol Range:** If `m_range > 0`, reverses when $|x - x_{\text{start}}| > \text{range}$.

### 5.4 Bump Block Reaction
When the player jumps from below:
$$\text{touchingBottom} = (y_{\text{head}} \ge y_{\text{bottom}} - 0.15\text{ m}) \land (y_{\text{head}} \le y_{\text{bottom}} + 0.35\text{ m}) \land (y_{\text{feet}} < y_{\text{bottom}}) \land (\text{horizontal overlap})$$

If touched while $v_y \ge -0.2\text{ m/s}$:
- Triggers a $0.20\text{ m}$ vertical sinusoidal bounce over $0.15\text{ s}$.
- Ejects 1 golden coin into player inventory (`player.addCoins(1)`).
- Triggers chime sound and particle puff.
- Decrements coins remaining; once at 0, turns into an empty gray block. Solid collision bounds remain resting at all times.

### 5.5 Saw Blade Damage
`Saw` interpolates parametrically along a track between $\vec{p}_A$ and $\vec{p}_B$ while spinning at $14.0\text{ rad/s}$. Contact is evaluated by clamping the saw center to the player's bounding box and testing distance against `radius`:

$$\text{overlap} = \|\vec{p}_{\text{saw}} - \text{clamp}(\vec{p}_{\text{saw}}, \text{AABB}_{\min}, \text{AABB}_{\max})\|^2 \le r_{\text{saw}}^2$$

Inflicts 1 heart of damage and knockback on contact.

---

## 6. Scoring, Timing & Par Time

$$\text{Star Rating} = 
\begin{cases} 
3 \text{ Stars} & \text{if } t_{\text{elapsed}} \le t_{\text{par}} \text{ and } \text{deaths} = 0 \\
2 \text{ Stars} & \text{if } t_{\text{elapsed}} \le 1.4 \cdot t_{\text{par}} \\
1 \text{ Star} & \text{otherwise}
\end{cases}$$

Level progress (best time and star rating) is written to `SaveData` upon touching the `FinishZone`.
