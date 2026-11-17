#pragma once
#include "game_state.h"

namespace obby {

/**
 * @brief Pause overlay state displaying resume, restart, settings, and quit modal.
 */
class PauseState : public GameState {
public:
    explicit PauseState(Game& game);
    virtual ~PauseState() override = default;

    virtual void handleInput() override;
    virtual void update(float dt) override;
    virtual void draw() const override;
    virtual StateType getType() const override { return StateType::PAUSED; }
};

} // namespace obby

using PauseState = obby::PauseState;
