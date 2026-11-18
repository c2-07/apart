#include "level_select_state.h"
#include "game.h"
#include "ui.h"
#include "raylib.h"
#include <cstdio>

namespace obby {

LevelSelectState::LevelSelectState(Game& game)
    : GameState(game) {}

void LevelSelectState::handleInput() {}

void LevelSelectState::update(float dt) {
    (void)dt;
}

void LevelSelectState::draw() const {
    ClearBackground(Color{ 20, 25, 35, 255 });
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    const char* title = "LEVEL SELECT - 4 WORLDS";
    int tw = MeasureText(title, 26);
    DrawText(title, (sw - tw) / 2, 25, 26, Color{ 255, 225, 60, 255 });

    const char* worldNames[4] = { "World 1: Meadow", "World 2: Ocean", "World 3: Winter", "World 4: Factory" };
    Color worldColors[4] = { Color{ 85, 180, 75, 255 }, Color{ 50, 160, 220, 255 }, Color{ 140, 195, 235, 255 }, Color{ 230, 130, 40, 255 } };

    float startY = 75.0f;
    for (int w = 0; w < 4; ++w) {
        bool worldUnlocked = (w + 1) <= m_game.save.unlockedWorlds;

        float rowY = startY + w * 125.0f;
        DrawText(worldNames[w], 60, (int)rowY, 20, worldUnlocked ? worldColors[w] : Color{ 110, 115, 125, 255 });

        if (!worldUnlocked) {
            DrawText("[ LOCKED - Complete previous world to unlock ]", 250, (int)rowY + 2, 16, Color{ 140, 145, 155, 200 });
        }

        for (int l = 0; l < 5; ++l) {
            int lvlIdx = w * 5 + l;
            float btnX = 60.0f + l * 230.0f;
            float btnY = rowY + 30.0f;
            Rectangle tile = { btnX, btnY, 210, 75 };

            if (worldUnlocked) {
                char btnLabel[32];
                snprintf(btnLabel, sizeof(btnLabel), "Level %d-%d", w + 1, l + 1);

                int stars = m_game.save.levels[lvlIdx].stars;
                const char* starTxt = (stars >= 3) ? "***" : (stars == 2) ? "**-" : (stars == 1) ? "*--" : "---";

                Color bg = (m_game.currentLevelIdx == lvlIdx) ? Color{ 50, 120, 180, 255 } : Color{ 40, 50, 70, 240 };
                if (UI::Button(tile, btnLabel, bg, Color{ 60, 140, 210, 255 }, WHITE, 18)) {
                    m_game.LoadLevelByIndex(lvlIdx);
                    m_game.RequestStateChange(StateType::PLAYING);
                }

                // Stars and best time below button label
                char statBuf[48];
                if (m_game.save.levels[lvlIdx].bestTime > 0.0f) {
                    snprintf(statBuf, sizeof(statBuf), "[%s] %ds", starTxt, static_cast<int>(m_game.save.levels[lvlIdx].bestTime));
                } else {
                    snprintf(statBuf, sizeof(statBuf), "[%s] No Record", starTxt);
                }
                DrawText(statBuf, (int)(tile.x + 35), (int)(tile.y + 48), 14, Color{ 255, 215, 80, 240 });
            } else {
                DrawRectangleRounded(tile, 0.2f, 4, Color{ 35, 40, 50, 180 });
                DrawRectangleRoundedLines(tile, 0.2f, 4, Color{ 60, 65, 75, 200 });
                char lockTxt[32];
                snprintf(lockTxt, sizeof(lockTxt), "%d-%d (Locked)", w + 1, l + 1);
                DrawText(lockTxt, (int)(tile.x + 40), (int)(tile.y + 28), 16, Color{ 110, 115, 125, 200 });
            }
        }
    }

    // 2D Showcase Level buttons
    float bottomY = (float)(sh - 65);
    if (UI::Button(Rectangle{ 60.0f, bottomY, 220, 42 }, "2D LEVEL 1 (GATEWAY)", Color{ 35, 120, 180, 255 }, Color{ 55, 150, 220, 255 }, WHITE, 14)) {
        m_game.LoadLevelByPath("assets/levels2d/level_2d_1.lvl");
        m_game.Set2DMode(true);
        m_game.RequestStateChange(StateType::PLAYING);
    }
    if (UI::Button(Rectangle{ 300.0f, bottomY, 220, 42 }, "2D LEVEL 2 (CAVERN)", Color{ 45, 140, 150, 255 }, Color{ 65, 170, 180, 255 }, WHITE, 14)) {
        m_game.LoadLevelByPath("assets/levels2d/level_2d_2.lvl");
        m_game.Set2DMode(true);
        m_game.RequestStateChange(StateType::PLAYING);
    }

    // Back Button
    if (UI::Button(Rectangle{ (float)(sw - 200), bottomY, 180, 42 }, "BACK TO MENU", Color{ 70, 80, 95, 255 }, Color{ 95, 110, 130, 255 }, WHITE, 16)) {
        m_game.RequestStateChange(StateType::MAIN_MENU);
    }
}

} // namespace obby
