# Object-Oriented Programming (OOP) Design & Viva Defense

**Branch:** `2d-version`  
**Document:** OOP Principles, Code Snippets & Defense Rationale  
**Target Audience:** Academic Evaluators, Students, Lab Examiners

---

## 1. Overview of OOP Architecture

The project is structured according to strict Object-Oriented Software Engineering (OOD/OOP) principles in C++17. The codebase completely avoids procedural global state or monolithic switch statements in the game loop, replacing them with polymorphic interfaces, clean inheritance hierarchies, RAII resource wrappers, template memory pools, and value classes with operator overloading.

---

## 2. Comprehensive Demonstration of OOP Concepts

### 2.1 Abstraction
- **Definition in 2 Sentences:** Abstraction hides complex internal implementation details behind high-level, intuitive interfaces that model real-world concepts. This allows consumer systems to interact with objects by intent (e.g. updating an entity or rendering a state) without needing to understand underlying geometric math, shader pipelines, or countdown timers.
- **File & Class:** `src/game_state.h`, `GameState`
- **Real Code Snippet:**
  ```cpp
  // File: src/game_state.h
  class GameState {
  protected:
      Game& m_game;
  public:
      explicit GameState(Game& game) : m_game(game) {}
      virtual ~GameState() = default;
      virtual void enter() {}
      virtual void exit() {}
      virtual void handleInput() = 0;
      virtual void update(float dt) = 0;
      virtual void draw() const = 0;
      virtual StateType getType() const = 0;
  };
  ```
- **Why Chosen:** Abstracting game states decouples `Game::Update()` from menu layouts, gameplay physics, and pause screens. The main loop merely calls `m_currentState->update(dt)`, eliminating monolithic state switches.

---

### 2.2 Encapsulation
- **Definition in 2 Sentences:** Encapsulation binds an object's internal data with the methods that operate on it, hiding state variables behind access specifiers (`private`/`protected`). External code cannot mutate class invariants directly, ensuring consistency and preventing corrupt intermediate states.
- **File & Class:** `src/player.h`, `Player`
- **Real Code Snippet:**
  ```cpp
  // File: src/player.h
  class Player {
  private:
      Vec3 m_position;
      Vec3 m_velocity;
      int m_hearts;
      int m_maxHearts;
      float m_invincibleTimer;
      int m_coins;
  public:
      int getHearts() const { return m_hearts; }
      int getCoins() const { return m_coins; }
      void takeDamage(const Vector2& knockbackDirection);
      void heal(int amount);
      void addCoins(int amount = 1) { m_coins += amount; }
  };
  ```
- **Why Chosen:** Encapsulating player physics variables (`m_position`, `m_velocity`, `m_hearts`) prevents external entities from arbitrarily teleporting the player or corrupting heart limits without triggering knockback or invincibility timers.

---

### 2.3 Inheritance
- **Definition in 2 Sentences:** Inheritance allows a derived class to acquire the data members and behavioral methods of a base class, establishing an "is-a" relationship. This maximizes code reuse by defining shared attributes and contracts in a common base class.
- **File & Class:** `src/spikes.h`, `Spikes` (inheriting from `Entity`)
- **Real Code Snippet:**
  ```cpp
  // File: src/spikes.h
  class Spikes : public Entity {
  public:
      Spikes(const Vec3& position, const Vec3& size);
      virtual ~Spikes() override = default;
      virtual void update(float dt, Player& player) override;
      virtual void draw() const override;
      virtual void onPlayerTouch(Player& player) override;
      virtual const char* getTypeName() const override { return "Spikes"; }
  };
  ```
- **Why Chosen:** `Spikes`, `Platform`, `Spinner`, and `BouncePad` all share spatial attributes (`m_position`, `m_size`, `m_color`) and lifecycle operations. Inheriting from `Entity` eliminates code duplication across all 15 derived world classes.

---

### 2.4 Multi-Level Inheritance
- **Definition in 2 Sentences:** Multi-level inheritance forms a linear hierarchy where a class derives from a parent class that itself derives from an ancestor class. This allows specialization to occur incrementally at each tier of the inheritance chain.
- **File & Class:** `src/moving_platform.h`, `MovingPlatform` (`MovingPlatform` $\rightarrow$ `Platform` $\rightarrow$ `Entity`)
- **Real Code Snippet:**
  ```cpp
  // File: src/moving_platform.h
  class MovingPlatform : public Platform {
  private:
      Vec3 m_basePos;
      Vec3 m_delta;
      float m_period;
      Vec3 m_frameDisplacement;
  public:
      MovingPlatform(const Vec3& basePos, const Vec3& size, const Vec3& delta,
                     float period, int colorIndex = 0, int worldId = 1);
      virtual void update(float dt, Player& player) override;
      virtual void draw() const override;
  };
  ```
- **Why Chosen:** `MovingPlatform` is a specialized platform. It inherits solid static AABB collision and top cap rendering from `Platform`, which in turn inherits core coordinates and entity properties from `Entity`.

---

### 2.5 Polymorphism & Virtual Function Overriding
- **Definition in 2 Sentences:** Polymorphism enables objects of different derived classes to be manipulated through base class pointers or references, with the runtime calling the correct derived method via a virtual method table (vtable). This allows game loops to process heterogeneous collections uniformly without knowing specific concrete types.
- **File & Class:** `src/level.cpp`, `Level::Update()` & `Level::Draw()`
- **Real Code Snippet:**
  ```cpp
  // File: src/level.cpp
  for (const auto& entity : m_entities) {
      entity->update(dt, player); // Polymorphic dispatch via Entity* vtable
  }
  for (const auto& entity : m_entities) {
      entity->draw();             // Polymorphic rendering dispatch
  }
  ```
- **Why Chosen:** The level update loop requires zero `switch` statements or downcasts (`dynamic_cast`). Whether an entity is a `Platform`, `Saw`, `Walker`, or `CrumbleTile`, the appropriate derived `update()` and `draw()` executes automatically.

---

### 2.6 Pure Virtual Functions & Abstract Classes
- **Definition in 2 Sentences:** An abstract class contains at least one pure virtual function (declared with `= 0`) and cannot be instantiated directly. It serves as an enforced template contract, mandating that every concrete subclass provide an implementation.
- **File & Class:** `src/entity.h`, `Entity`
- **Real Code Snippet:**
  ```cpp
  // File: src/entity.h
  class Entity : public IDrawable, public ITouchable {
  public:
      virtual ~Entity() override;
      virtual void update(float dt, Player& player) = 0; // Pure virtual
      virtual void draw() const = 0;                     // Pure virtual
      virtual const char* getTypeName() const = 0;       // Pure virtual
  };
  ```
- **Why Chosen:** A generic "Entity" has no concrete visual shape or physical behavior on its own. Marking `update` and `draw` pure virtual prevents instantiating meaningless raw entities while guaranteeing compile-time enforcement on derived classes.

---

### 2.7 Operator Overloading
- **Definition in 2 Sentences:** Operator overloading redefines the behavior of standard C++ operators (`+`, `-`, `*`, `==`) for user-defined types. This enables domain objects like vectors and bounding boxes to be manipulated using intuitive, natural mathematical expressions.
- **File & Class:** `src/vec3.h` / `src/vec3.cpp`, `Vec3`
- **Real Code Snippet:**
  ```cpp
  // File: src/vec3.h
  Vec3 operator+(const Vec3& rhs) const { return Vec3{ x + rhs.x, y + rhs.y, z + rhs.z }; }
  Vec3 operator-(const Vec3& rhs) const { return Vec3{ x - rhs.x, y - rhs.y, z - rhs.z }; }
  Vec3 operator*(float scalar) const    { return Vec3{ x * scalar, y * scalar, z * scalar }; }
  Vec3& operator+=(const Vec3& rhs)     { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
  ```
- **Why Chosen:** Writing `pos += vel * dt;` is mathematically transparent, safe, and readable compared to clumsy C function calls like `Vector3Add(pos, Vector3Scale(vel, dt));`.

---

### 2.8 Member Initializer Lists & Virtual Destructors
- **Definition in 2 Sentences:** Member initializer lists initialize class member variables and parent sub-objects directly before the constructor body executes, optimizing performance. Virtual destructors ensure that when a derived object is deleted through a base class pointer, the complete destructor chain executes in reverse order, preventing memory leaks.
- **File & Class:** `src/enemy.cpp` and `src/entity.h`
- **Real Code Snippet:**
  ```cpp
  // File: src/enemy.cpp
  Enemy::Enemy(const Vec3& position, const Vec3& size, Color color)
      : Entity(position, size, color),
        m_facingDirection(1),
        m_isAlive(true),
        m_isFlattened(false),
        m_flattenTimer(0.0f),
        m_startPos(position) {} // Direct member initialization

  // File: src/entity.h
  virtual ~Entity() override; // Virtual destructor guarantees derived cleanup
  ```
- **Why Chosen:** Without a virtual destructor in `Entity`, executing `m_entities.clear()` on `std::vector<std::unique_ptr<Entity>>` would only call `~Entity()`, leaving derived resources (such as timers and colliders in `MovingPlatform` or `Enemy`) leaked.

---

### 2.9 RAII (Resource Acquisition Is Initialization)
- **Definition in 2 Sentences:** RAII binds the lifecycle of unmanaged operating system resources (GPU shader programs and audio device handles) to the lifetime of stack or heap C++ objects. Acquisition occurs in the constructor, and deallocation is guaranteed in the destructor, even in the presence of runtime exceptions.
- **File & Class:** `src/shader_holder.h` / `src/shader_holder.cpp`, `ShaderHolder`
- **Real Code Snippet:**
  ```cpp
  // File: src/shader_holder.cpp
  ShaderHolder::ShaderHolder(const std::string& vsPath, const std::string& fsPath) {
      m_shader = LoadShader(vsPath.c_str(), fsPath.c_str());
      m_loaded = (m_shader.id > 0);
  }

  ShaderHolder::~ShaderHolder() {
      if (m_loaded) {
          UnloadShader(m_shader); // Guaranteed GPU memory deallocation
          m_loaded = false;
      }
  }
  ```
- **Why Chosen:** Prevents GPU memory leaks when reloading levels or exiting the application without requiring error-prone manual `UnloadShader()` calls scattered across game shutdown routines.

---

### 2.10 Generic Class Templates
- **Definition in 2 Sentences:** Class templates allow classes to be defined with generic type parameters, enabling a single codebase to operate safely across multiple data types with zero runtime overhead. The compiler generates specialized, type-safe implementations during compilation.
- **File & Class:** `src/object_pool.h`, `ObjectPool<T>`
- **Real Code Snippet:**
  ```cpp
  // File: src/object_pool.h
  template <typename T>
  class ObjectPool {
  private:
      std::vector<T> m_pool;
      size_t m_activeCount;
  public:
      explicit ObjectPool(size_t initialCapacity = 256) : m_activeCount(0) {
          m_pool.resize(initialCapacity);
      }
      T* acquire() {
          if (m_activeCount >= m_pool.size()) m_pool.resize(m_pool.size() * 2);
          return &m_pool[m_activeCount++];
      }
      void release(size_t index) {
          if (index < m_activeCount) {
              std::swap(m_pool[index], m_pool[--m_activeCount]); // O(1) swap-and-pop
          }
      }
  };
  ```
- **Why Chosen:** Calling `new` or `std::vector::push_back` at 120 Hz during particle explosions causes heap fragmentation and CPU frame spikes. `ObjectPool<T>` provides pre-allocated memory with $O(1)$ release.

---

### 2.11 Exception Handling
- **Definition in 2 Sentences:** Exception handling provides a structured mechanism to detect, propagate, and resolve runtime anomalies without crashing the application. Exceptions decouple the point of error detection from the point of error recovery.
- **File & Class:** `src/level_error.h`, `LevelParseError`
- **Real Code Snippet:**
  ```cpp
  // File: src/level_error.h
  class LevelParseError : public std::runtime_error {
  public:
      explicit LevelParseError(const std::string& message) : std::runtime_error(message) {}
  };

  // File: src/level.cpp
  try {
      while (std::getline(file, line)) { ... }
  } catch (const LevelParseError& err) {
      std::cerr << "[Level Parser Exception] " << filepath << ":" << lineNum << ": " << err.what() << std::endl;
      return false; // Graceful recovery without process crash
  }
  ```
- **Why Chosen:** If a level file contains corrupted float coordinates or unknown entity tokens, `EntityFactory` throws a descriptive `LevelParseError`. `Level::LoadFromFile` catches it, logs the exact line number, and returns `false`, preventing silent errors or segmentation faults.

---

### 2.12 Static Members & Methods
- **Definition in 2 Sentences:** Static class members belong to the class type itself rather than any specific object instance. Static methods provide utility operations that execute without requiring an allocated instance, while static variables track class-wide state across all instances.
- **File & Class:** `src/entity.h` / `src/entity.cpp`, `Entity::s_entityCount`
- **Real Code Snippet:**
  ```cpp
  // File: src/entity.h
  class Entity : public IDrawable, public ITouchable {
  protected:
      static int s_entityCount; // Class-wide tracking variable
  public:
      static int getEntityCount() { return s_entityCount; }
  };

  // File: src/entity.cpp
  int Entity::s_entityCount = 0;
  Entity::Entity(...) { ++s_entityCount; }
  Entity::~Entity()   { --s_entityCount; }
  ```
- **Why Chosen:** Allows diagnostic tools and test harnesses to verify that entities are constructed and destructed symmetrically without any heap leaks.

---

### 2.13 Multiple Interface Inheritance
- **Definition in 2 Sentences:** Multiple interface inheritance occurs when a derived class realizes two or more pure abstract classes (interfaces) simultaneously. Because interfaces declare only pure virtual functions without member variables, this pattern avoids the ambiguity and overhead of the Diamond Problem.
- **File & Class:** `src/entity.h`, `Entity` (`IDrawable` and `ITouchable`)
- **Real Code Snippet:**
  ```cpp
  // File: src/entity.h
  class Entity : public IDrawable, public ITouchable {
      // Realizes draw() from IDrawable
      virtual void draw() const override = 0;

      // Realizes onPlayerTouch() from ITouchable
      virtual void onPlayerTouch(Player& player) override;
  };
  ```
- **Why Chosen:** Separates rendering concerns (`IDrawable`) from collision reaction concerns (`ITouchable`), satisfying the Interface Segregation Principle (ISP) while allowing entities to be passed to specialized sub-systems.

---

### 2.14 STL Container Utilization & Safe Deletion Idiom
- **Definition in 2 Sentences:** Standard Template Library (STL) containers provide safe, standardized data structures with guaranteed algorithmic complexity. The erase-remove idiom (`std::remove_if`) prunes elements meeting a predicate in a single linear pass without iterator invalidation.
- **File & Class:** `src/level.cpp`, `Level::Update()`
- **Real Code Snippet:**
  ```cpp
  // File: src/level.cpp
  m_entities.erase(
      std::remove_if(m_entities.begin(), m_entities.end(),
          [](const std::unique_ptr<Entity>& e) {
              return e->isExpired(); // Prune defeated enemies whose flatten timer ended
          }),
      m_entities.end());
  ```
- **Why Chosen:** Deleting an entity directly during the update loop invalidates iterators and skips elements. Marking entities `isExpired() == true` and running `std::remove_if` afterward guarantees $O(N)$ iterator-safe cleanup.
