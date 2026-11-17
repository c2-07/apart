#pragma once
#include "game_state.h"

namespace obby {

/**
 * @brief Active gameplay state running 120Hz physics substepping, HUD overlay, and completion triggers.
 */
class PlayState : public GameState {
private:
    mutable float m_heartShakeTimer;
    mutable int m_previousHearts;

public:
    explicit PlayState(Game& game);
    virtual ~PlayState() override = default;

    virtual void handleInput() override;
    virtual void update(float dt) override;
    virtual void draw() const override;
    virtual StateType getType() const override { return StateType::PLAYING; }
};

} // namespace obby

using PlayState = obby::PlayState;
