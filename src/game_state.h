#pragma once

namespace obby {

class Game;

enum class StateType {
    MAIN_MENU,
    LEVEL_SELECT,
    CHARACTER_SELECT,
    SETTINGS,
    PLAYING,
    PAUSED,
    RESULTS
};

/**
 * @brief Abstract base class for the polymorphic Game State hierarchy.
 * Demonstrates:
 * - Pure virtual functions: handleInput(), update(float dt), draw()
 * - Virtual lifecycle methods: enter(), exit()
 * - Decoupled state transition requests via Game context
 * - Virtual destructor for proper polymorphic destruction
 */
class GameState {
protected:
    Game& m_game;

public:
    explicit GameState(Game& game) : m_game(game) {}
    virtual ~GameState() = default;

    GameState(const GameState&) = delete;
    GameState& operator=(const GameState&) = delete;

    virtual void enter() {}
    virtual void exit() {}
    virtual void handleInput() = 0;
    virtual void update(float dt) = 0;
    virtual void draw() const = 0;
    virtual StateType getType() const = 0;
};

} // namespace obby

using GameState = obby::GameState;
using StateType = obby::StateType;
