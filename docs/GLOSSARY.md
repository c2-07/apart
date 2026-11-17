# Glossary of Technical & Game Engine Terms

**Branch:** `2d-version`  
**Document:** Definitions of Architecture, Math, Physics & Graphics Terminology  

---

### AABB (Axis-Aligned Bounding Box)
A rectangular prism volume oriented strictly parallel to the Cartesian coordinate axes ($X, Y, Z$). AABBs simplify collision detection because testing intersection between two AABBs requires only 6 scalar comparisons ($x_{\min} < x'_{\max}$ etc.), completely avoiding expensive trigonometric rotation calculations.

### Fixed Timestep / Accumulator Loop
A game simulation loop pattern where game logic and physics advance at a constant, deterministic frequency (here, $120\text{ Hz}$ or $8.33\text{ ms}$ per tick) regardless of the monitor's display refresh rate. Real-world elapsed frame time (`frameDelta`) is added into an `accumulator`, which is drained in discrete physics steps, preventing inconsistent game behavior caused by varying framerates.

### Coyote Time
A platformer gameplay assist mechanism that grants the player a brief grace period (here, $0.10\text{ s}$) to jump after stepping off a platform ledge into midair. Named after the cartoon character Wile E. Coyote, it prevents the game from feeling unfair when a player presses jump slightly after their avatar leaves a surface.

### Jump Buffering
An input tolerance assist where pressing the jump button briefly before landing on a surface (here, within $0.10\text{ s}$) records the input and executes the jump automatically on the very first grounded frame, eliminating missed inputs caused by micro-timing errors.

### Z-Fighting (Coplanar Flicker)
A visual artifact that occurs when two geometric surfaces occupy the exact same depth coordinates (coplanar), causing the GPU's depth buffer to flicker between them as floating-point precision fluctuates. Prevented in this codebase by raising checkpoints and decals slightly ($+0.02\text{ m}$ to $+0.05\text{ m}$) and tuning near/far clipping planes (`0.1` to `300.0`).

### Substepping (Tunneling Prevention)
Subdividing a single physics time step into multiple smaller micro-steps whenever an object's speed would cause it to travel further than a critical distance (here, $0.04\text{ m}$) in one tick. This ensures fast-moving bodies cannot pass completely through thin walls or platforms between consecutive collision queries.

### RAII (Resource Acquisition Is Initialization)
A foundational C++ programming idiom where resource allocation (such as GPU shaders or audio device initialization) is performed inside a class constructor, and resource deallocation is guaranteed inside the class destructor. When the wrapping object goes out of scope, resources are automatically freed, preventing memory and handle leaks.

### Polymorphism & Vtable (Virtual Method Table)
The capability of different classes derived from a common base to provide their own distinct implementations of virtual methods. The compiler constructs a hidden table of function pointers (vtable) for each polymorphic class, allowing callers to execute `entity->update()` through a base pointer while resolving the correct derived function at runtime.

### Factory Pattern
A creational design pattern that delegates object instantiation to a specialized method (`EntityFactory::createEntity`). The caller provides input parameters (such as string tokens from a `.lvl` file), and the factory returns an abstract base pointer (`std::unique_ptr<Entity>`) without the caller needing to know the concrete derived class.

### State Pattern
A behavioral design pattern that allows an object (`Game`) to alter its behavior when its internal state changes by delegating input, update, and drawing routines to a polymorphic `GameState` object (`MenuState`, `PlayState`, `PauseState`, etc.).

### Erase-Remove Idiom (`std::remove_if`)
An efficient C++ algorithm pattern for removing elements matching a predicate from an `std::vector`. `std::remove_if` shifts surviving elements to the front of the vector in $O(N)$ time, and `vector::erase` prunes the dead tail, avoiding iterator invalidation during iteration.

### Object Pooling
A memory management optimization that pre-allocates a fixed array of reusable objects (`ObjectPool<T>`). When an item (like a particle) is created, it is activated from the pool; when it dies, it is swapped back to the inactive list in $O(1)$ time, eliminating runtime heap allocations (`new`/`delete`).

### Skin Tolerance ($\epsilon$)
A tiny geometric offset (here, $0.01\text{ m}$) used during collision resolution to prevent mathematical edge cases where rounding errors cause an object resting flush on a surface to be falsely detected as intersecting adjacent lateral walls.

### Lambertian Diffuse Lighting
A 3D shading model where the perceived brightness of a surface is proportional to the cosine of the angle between the surface normal $\vec{N}$ and the light source direction $\vec{L}$ ($\vec{N} \cdot \vec{L}$). Provides clear visual depth and facet distinction without photorealistic specular glare.

### Par Time
The gold-standard target duration (in seconds) established by the level designer for completing a platforming course. Beating a level within its par time with 0 deaths awards a 3-star ranking.
