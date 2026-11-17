#pragma once
#include "game_state.h"

namespace obby {

/**
 * @brief Main Menu state with orbiting background camera and navigation buttons.
 */
class MenuState : public GameState {
public:
    explicit MenuState(Game& game);
    virtual ~MenuState() override = default;

    virtual void enter() override;
    virtual void handleInput() override;
    virtual void update(float dt) override;
    virtual void draw() const override;
    virtual StateType getType() const override { return StateType::MAIN_MENU; }
};

} // namespace obby

using MenuState = obby::MenuState;
