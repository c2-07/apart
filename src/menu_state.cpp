#include "menu_state.h"
#include "game.h"
#include "ui.h"
#include "raylib.h"
#include "rlgl.h"

namespace obby {

MenuState::MenuState(Game& game)
    : GameState(game) {}

void MenuState::enter() {}

void MenuState::handleInput() {}

void MenuState::update(float dt) {
    // Background slow camera orbit
    m_game.camera.setYaw(m_game.camera.getYaw() + 12.0f * dt);
    std::vector<AABB> colliders = m_game.level.GetSolidColliders();
    m_game.camera.update(m_game.player.getPosition(), dt, colliders);
}

void MenuState::draw() const {
    m_game.level.DrawSkybox();
    BeginMode3D(m_game.camera.getRaylibCamera());
    m_game.ApplyLightingShader(m_game.camera.getCurrentEye().toRaylib(), m_game.level.worldId);
    m_game.level.Draw();
    m_game.player.draw();
    if (m_game.shaderLoaded) EndShaderMode();

    rlDisableDepthMask();
    m_game.player.drawShadow(m_game.level.GetSolidColliders());
    rlEnableDepthMask();

    EndMode3D();

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    // Backdrop shadow panel for menu
    Rectangle panel = { (sw - 380) * 0.5f, (sh - 480) * 0.5f, 380, 480 };
    DrawRectangleRounded(panel, 0.12f, 4, Color{ 15, 20, 30, 215 });
    DrawRectangleRoundedLines(panel, 0.12f, 4, Color{ 70, 95, 140, 240 });

    const char* title = "BLOCKY OBBY";
    const char* sub = "3D SOLO ADVENTURE";
    int tw = MeasureText(title, 34);
    int sww = MeasureText(sub, 16);
    DrawText(title, (int)(panel.x + (panel.width - tw) * 0.5f), (int)(panel.y + 35), 34, Color{ 255, 220, 50, 255 });
    DrawText(sub, (int)(panel.x + (panel.width - sww) * 0.5f), (int)(panel.y + 75), 16, Color{ 180, 215, 255, 230 });

    float btnY = panel.y + 120;
    float btnW = 280;
    float btnH = 48;
    float btnX = panel.x + (panel.width - btnW) * 0.5f;

    if (UI::Button(Rectangle{ btnX, btnY, btnW, btnH }, "PLAY", Color{ 45, 150, 85, 255 }, Color{ 65, 185, 110, 255 }, WHITE, 22)) {
        m_game.LoadLevelByIndex(m_game.currentLevelIdx);
        m_game.RequestStateChange(StateType::PLAYING);
    }
    if (UI::Button(Rectangle{ btnX, btnY + 65, btnW, btnH }, "LEVEL SELECT", Color{ 55, 80, 120, 255 }, Color{ 75, 105, 155, 255 }, WHITE, 20)) {
        m_game.RequestStateChange(StateType::LEVEL_SELECT);
    }
    if (UI::Button(Rectangle{ btnX, btnY + 130, btnW, btnH }, "CHARACTER", Color{ 120, 70, 140, 255 }, Color{ 150, 95, 175, 255 }, WHITE, 20)) {
        m_game.RequestStateChange(StateType::CHARACTER_SELECT);
    }
    if (UI::Button(Rectangle{ btnX, btnY + 195, btnW, btnH }, "SETTINGS", Color{ 70, 80, 95, 255 }, Color{ 95, 105, 125, 255 }, WHITE, 20)) {
        m_game.previousState = AppState::MAIN_MENU;
        m_game.RequestStateChange(StateType::SETTINGS);
    }
    if (UI::Button(Rectangle{ btnX, btnY + 260, btnW, btnH }, "QUIT", Color{ 140, 50, 60, 255 }, Color{ 180, 70, 80, 255 }, WHITE, 20)) {
        CloseWindow();
    }
}

} // namespace obby
