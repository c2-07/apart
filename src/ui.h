#pragma once
#include "raylib.h"
#include <string>

enum class ResultsAction {
    NONE,
    RETRY,
    NEXT_LEVEL
};

namespace UI {
    // Draws in-game HUD: Level name, timer, checkpoints, deaths
    void DrawHUD(const std::string& levelName, float timer, float parTime, int checkpoints, int totalCheckpoints, int deaths);

    // Draws Master Volume slider widget
    void DrawVolumeControl(int posX, int posY, float width = 160.0f);

    // Draws Level Complete results modal
    ResultsAction DrawResultsScreen(int screenWidth, int screenHeight, float finalTime, float parTime, int deaths, int stars);
}
