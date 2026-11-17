#pragma once
#include "game_state.h"

namespace obby {

/**
 * @brief Results screen state displaying completion time, par comparison, death count, and star evaluation.
 */
class ResultsState : public GameState {
public:
    explicit ResultsState(Game& game);
    virtual ~ResultsState() override = default;

    virtual void handleInput() override;
    virtual void update(float dt) override;
    virtual void draw() const override;
    virtual StateType getType() const override { return StateType::RESULTS; }
};

} // namespace obby

using ResultsState = obby::ResultsState;
