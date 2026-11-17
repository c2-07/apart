#include "character_select_state.h"
#include "game.h"
#include "ui.h"
#include "raylib.h"

namespace obby {

CharacterSelectState::CharacterSelectState(Game& game)
    : GameState(game) {}

void CharacterSelectState::handleInput() {}

void CharacterSelectState::update(float dt) {
    m_game.player.setModelYaw(m_game.player.getModelYaw() + 1.5f * dt);
}

void CharacterSelectState::draw() const {
    ClearBackground(Color{ 25, 30, 42, 255 });
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    const char* title = "CHARACTER CUSTOMIZATION";
    int tw = MeasureText(title, 26);
    DrawText(title, (sw - tw) / 2, 35, 26, Color{ 255, 220, 60, 255 });

    // 3D Preview of character
    Camera3D previewCam;
    previewCam.position = Vector3{ 0.0f, 1.4f, 4.0f };
    previewCam.target = Vector3{ 0.0f, 1.2f, 0.0f };
    previewCam.up = Vector3{ 0.0f, 1.0f, 0.0f };
    previewCam.fovy = 45.0f;
    previewCam.projection = CAMERA_PERSPECTIVE;

    BeginMode3D(previewCam);
    Vec3 oldPos = m_game.player.getPosition();
    m_game.player.setPosition(Vec3{ 0.0f, 0.0f, 0.0f });
    m_game.ApplyLightingShader(previewCam.position, 1);
    m_game.player.draw();
    if (m_game.shaderLoaded) EndShaderMode();
    m_game.player.setPosition(oldPos);
    EndMode3D();

    // Color Swatches
    const char* colorNames[6] = { "Classic Blue", "Crimson Red", "Emerald Green", "Royal Purple", "Sunset Orange", "Sunshine Yellow" };
    Color shirtColors[6] = {
        Color{ 55, 130, 235, 255 }, Color{ 225, 45, 55, 255 }, Color{ 45, 185, 95, 255 },
        Color{ 145, 65, 215, 255 }, Color{ 245, 125, 35, 255 }, Color{ 245, 215, 45, 255 }
    };

    float startX = (sw - (6 * 140)) * 0.5f;
    for (int i = 0; i < 6; ++i) {
        Rectangle swatch = { startX + i * 140.0f, (float)(sh - 180), 120, 50 };
        bool selected = (m_game.player.getColorIndex() == i);

        if (UI::Button(swatch, colorNames[i], shirtColors[i], shirtColors[i], WHITE, 14)) {
            m_game.player.setColorIndex(i);
            m_game.save.chosenColor = i;
            m_game.save.Save();
        }
        if (selected) {
            DrawRectangleLinesEx(Rectangle{ swatch.x - 3, swatch.y - 3, swatch.width + 6, swatch.height + 6 }, 3.0f, WHITE);
        }
    }

    if (UI::Button(Rectangle{ (sw - 180) * 0.5f, (float)(sh - 80), 180, 42 }, "CONFIRM & BACK", Color{ 45, 140, 85, 255 }, Color{ 65, 175, 110, 255 }, WHITE, 16)) {
        m_game.RequestStateChange(StateType::MAIN_MENU);
    }
}

} // namespace obby
