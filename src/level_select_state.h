#pragma once
#include "game_state.h"

namespace obby {

/**
 * @brief Level Select state showing 4 worlds with 5 courses each, unlock criteria, and star records.
 */
class LevelSelectState : public GameState {
public:
    explicit LevelSelectState(Game& game);
    virtual ~LevelSelectState() override = default;

    virtual void handleInput() override;
    virtual void update(float dt) override;
    virtual void draw() const override;
    virtual StateType getType() const override { return StateType::LEVEL_SELECT; }
};

} // namespace obby

using LevelSelectState = obby::LevelSelectState;
