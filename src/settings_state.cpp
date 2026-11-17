#include "settings_state.h"
#include "game.h"
#include "ui.h"
#include "audio.h"
#include "raylib.h"

namespace obby {

SettingsState::SettingsState(Game& game)
    : GameState(game) {}

void SettingsState::handleInput() {}

void SettingsState::update(float dt) {
    (void)dt;
}

void SettingsState::draw() const {
    ClearBackground(Color{ 20, 25, 35, 255 });
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    float panelW = 460.0f;
    float panelH = 420.0f;
    Rectangle panel = { (sw - panelW) * 0.5f, (sh - panelH) * 0.5f, panelW, panelH };

    DrawRectangleRounded(panel, 0.15f, 6, Color{ 28, 35, 48, 250 });
    DrawRectangleRoundedLines(panel, 0.15f, 6, Color{ 90, 115, 160, 255 });

    const char* title = "GAME SETTINGS";
    int tw = MeasureText(title, 26);
    DrawText(title, (int)(panel.x + (panelW - tw) * 0.5f), (int)(panel.y + 30), 26, Color{ 255, 220, 50, 255 });

    float curY = panel.y + 90;
    float fieldW = 260.0f;
    float fieldX = panel.x + 50.0f;

    // Master Volume
    float curVol = AudioManager::GetVolume();
    if (UI::Slider(Rectangle{ fieldX, curY, fieldW, 18 }, "Master Volume", &curVol, 0.0f, 1.0f, "%.0f%%")) {
        AudioManager::SetVolume(curVol);
        m_game.save.volume = curVol;
    }

    // Mouse Sensitivity
    curY += 65;
    if (UI::Slider(Rectangle{ fieldX, curY, fieldW, 18 }, "Camera Sensitivity", &m_game.save.sensitivity, 0.05f, 0.50f, "%.2fx")) {
        // Sensitivity updated
    }

    // Invert Y
    curY += 65;
    UI::Toggle(Rectangle{ fieldX, curY, fieldW + 80, 32 }, "Invert Pitch Y", &m_game.save.invertY);

    // Fullscreen Toggle
    curY += 50;
    if (UI::Toggle(Rectangle{ fieldX, curY, fieldW + 80, 32 }, "Fullscreen Mode", &m_game.save.isFullscreen)) {
        ToggleFullscreen();
    }

    // Back Button
    if (UI::Button(Rectangle{ (sw - 180) * 0.5f, panel.y + panelH - 65.0f, 180, 42 }, "SAVE & BACK", Color{ 45, 140, 85, 255 }, Color{ 65, 175, 110, 255 }, WHITE, 16)) {
        m_game.save.Save();
        m_game.RequestStateChange(m_game.previousState == AppState::PAUSED ? StateType::PAUSED : StateType::MAIN_MENU);
    }
}

} // namespace obby
