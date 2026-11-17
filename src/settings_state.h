#pragma once
#include "game_state.h"

namespace obby {

/**
 * @brief Settings state configuring audio volume, camera sensitivity, pitch inversion, and fullscreen.
 */
class SettingsState : public GameState {
public:
    explicit SettingsState(Game& game);
    virtual ~SettingsState() override = default;

    virtual void handleInput() override;
    virtual void update(float dt) override;
    virtual void draw() const override;
    virtual StateType getType() const override { return StateType::SETTINGS; }
};

} // namespace obby

using SettingsState = obby::SettingsState;
