#pragma once
#include "game_state.h"

namespace obby {

/**
 * @brief Character Customization state with 3D avatar preview and 6 color themes.
 */
class CharacterSelectState : public GameState {
public:
    explicit CharacterSelectState(Game& game);
    virtual ~CharacterSelectState() override = default;

    virtual void handleInput() override;
    virtual void update(float dt) override;
    virtual void draw() const override;
    virtual StateType getType() const override { return StateType::CHARACTER_SELECT; }
};

} // namespace obby

using CharacterSelectState = obby::CharacterSelectState;
