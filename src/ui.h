#pragma once
#include "raylib.h"
#include <string>

enum class ResultsAction {
    NONE,
    RETRY,
    NEXT_LEVEL,
    MENU
};

namespace UI {
    // Basic immediate-mode controls
    bool Button(Rectangle bounds, const char* text, Color baseColor, Color hoverColor, Color textColor = WHITE, int fontSize = 20);
    bool Slider(Rectangle bounds, const char* label, float* value, float minVal, float maxVal, const char* format = "%.2f");
    bool Toggle(Rectangle bounds, const char* label, bool* value);

    // In-game HUD
    void DrawHUD(const std::string& levelName, float timer, float parTime,
                 int checkpoints, int totalCheckpoints, int deaths,
                 int hearts = 3, int maxHearts = 3, float heartShake = 0.0f,
                 float hintAlpha = 1.0f, float titleAlpha = 0.0f);
    void DrawVolumeControl(int posX, int posY, float width = 150.0f);

    // Results modal
    ResultsAction DrawResultsScreen(int screenWidth, int screenHeight, float finalTime, float parTime, int deaths, int stars);
}
