#include "results_state.h"
#include "game.h"
#include "ui.h"
#include "raylib.h"

namespace obby {

ResultsState::ResultsState(Game& game)
    : GameState(game) {}

void ResultsState::handleInput() {}

void ResultsState::update(float dt) {
    (void)dt;
}

void ResultsState::draw() const {
    m_game.DrawPlaying();

    ResultsAction action = UI::DrawResultsScreen(
        GetScreenWidth(), GetScreenHeight(),
        m_game.level.elapsedTime, m_game.level.parTime, m_game.level.deaths, m_game.level.starRating
    );

    if (action == ResultsAction::RETRY) {
        m_game.LoadLevelByIndex(m_game.currentLevelIdx);
        m_game.RequestStateChange(StateType::PLAYING);
    } else if (action == ResultsAction::NEXT_LEVEL) {
        int nextIdx = (m_game.currentLevelIdx + 1) % m_game.GetTotalLevels();
        m_game.LoadLevelByIndex(nextIdx);
        m_game.RequestStateChange(StateType::PLAYING);
    } else if (action == ResultsAction::MENU) {
        m_game.RequestStateChange(StateType::MAIN_MENU);
    }
}

} // namespace obby
