#include "pause_state.h"
#include "game.h"
#include "ui.h"
#include "raylib.h"

namespace obby {

PauseState::PauseState(Game& game)
    : GameState(game) {}

void PauseState::handleInput() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        m_game.RequestStateChange(StateType::PLAYING);
    }
}

void PauseState::update(float dt) {
    (void)dt;
    handleInput();
}

void PauseState::draw() const {
    m_game.DrawPlaying(); // Draw frozen game scene behind pause modal

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    DrawRectangle(0, 0, sw, sh, Color{ 10, 15, 25, 175 });

    float cardW = 360.0f;
    float cardH = 340.0f;
    Rectangle card = { (sw - cardW) * 0.5f, (sh - cardH) * 0.5f, cardW, cardH };

    DrawRectangleRounded(card, 0.15f, 6, Color{ 25, 32, 45, 250 });
    DrawRectangleRoundedLines(card, 0.15f, 6, Color{ 90, 120, 170, 255 });

    const char* title = "PAUSED";
    int tw = MeasureText(title, 28);
    DrawText(title, (int)(card.x + (cardW - tw) * 0.5f), (int)(card.y + 30), 28, Color{ 255, 220, 50, 255 });

    float btnY = card.y + 85;
    float btnW = 260;
    float btnH = 44;
    float btnX = card.x + (cardW - btnW) * 0.5f;

    if (UI::Button(Rectangle{ btnX, btnY, btnW, btnH }, "RESUME", Color{ 45, 140, 85, 255 }, Color{ 65, 175, 110, 255 }, WHITE, 18)) {
        m_game.RequestStateChange(StateType::PLAYING);
    }
    if (UI::Button(Rectangle{ btnX, btnY + 55, btnW, btnH }, "RESTART LEVEL", Color{ 55, 80, 120, 255 }, Color{ 75, 105, 155, 255 }, WHITE, 18)) {
        m_game.LoadLevelByIndex(m_game.currentLevelIdx);
        m_game.RequestStateChange(StateType::PLAYING);
    }
    if (UI::Button(Rectangle{ btnX, btnY + 110, btnW, btnH }, "SETTINGS", Color{ 70, 80, 95, 255 }, Color{ 95, 105, 125, 255 }, WHITE, 18)) {
        m_game.previousState = AppState::PAUSED;
        m_game.RequestStateChange(StateType::SETTINGS);
    }
    if (UI::Button(Rectangle{ btnX, btnY + 165, btnW, btnH }, "QUIT TO MENU", Color{ 140, 50, 60, 255 }, Color{ 180, 70, 80, 255 }, WHITE, 18)) {
        m_game.RequestStateChange(StateType::MAIN_MENU);
    }
}

} // namespace obby
