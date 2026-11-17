#include "ui.h"
#include <cstdio>

namespace UI {

void DrawHUD(const std::string& levelName, float timer, float parTime, int checkpoints, int totalCheckpoints, int deaths) {
    // Top-left HUD panel
    Rectangle hudPanel = { 15, 15, 340, 115 };
    DrawRectangleRounded(hudPanel, 0.2f, 4, Color{ 20, 25, 35, 200 });
    DrawRectangleRoundedLines(hudPanel, 0.2f, 4, Color{ 70, 90, 120, 255 });

    // Level Title
    DrawText(levelName.c_str(), 30, 25, 20, Color{ 255, 255, 255, 255 });

    // Timer & Par
    char timeBuf[64];
    snprintf(timeBuf, sizeof(timeBuf), "Time: %.2fs (Par: %.1fs)", timer, parTime);
    Color timeColor = (timer <= parTime) ? Color{ 100, 240, 120, 255 } : Color{ 240, 190, 80, 255 };
    DrawText(timeBuf, 30, 52, 17, timeColor);

    // Checkpoints & Deaths
    char cpBuf[64];
    snprintf(cpBuf, sizeof(cpBuf), "Checkpoints: %d / %d", checkpoints, totalCheckpoints);
    DrawText(cpBuf, 30, 75, 16, Color{ 210, 220, 230, 255 });

    char deathBuf[32];
    snprintf(deathBuf, sizeof(deathBuf), "Deaths: %d", deaths);
    DrawText(deathBuf, 30, 97, 16, (deaths == 0) ? Color{ 150, 230, 150, 255 } : Color{ 245, 110, 100, 255 });

    // Bottom-center controls hint
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    const char* hint = "WASD: Move | Space: Jump | R: Respawn | Right-Mouse/Arrows: Camera Orbit";
    int hintWidth = MeasureText(hint, 16);
    DrawRectangleRounded(Rectangle{ (float)(screenWidth / 2 - hintWidth / 2 - 15), (float)(screenHeight - 40), (float)(hintWidth + 30), 30 },
                         0.3f, 4, Color{ 15, 20, 30, 180 });
    DrawText(hint, screenWidth / 2 - hintWidth / 2, screenHeight - 33, 16, Color{ 220, 230, 240, 220 });
}

static bool DrawButton(Rectangle bounds, const char* text, Color baseColor, Color hoverColor, Color textColor) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, bounds);
    Color col = hovered ? hoverColor : baseColor;

    DrawRectangleRounded(bounds, 0.25f, 4, col);
    DrawRectangleRoundedLines(bounds, 0.25f, 4, Color{ 255, 255, 255, 60 });

    int textWidth = MeasureText(text, 20);
    DrawText(text, (int)(bounds.x + (bounds.width - textWidth) * 0.5f),
                   (int)(bounds.y + (bounds.height - 20) * 0.5f), 20, textColor);

    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

ResultsAction DrawResultsScreen(int screenWidth, int screenHeight, float finalTime, float parTime, int deaths, int stars) {
    // Dim background overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, Color{ 10, 15, 25, 175 });

    // Dialog card
    float cardW = 460.0f;
    float cardH = 340.0f;
    Rectangle card = {
        (screenWidth - cardW) * 0.5f,
        (screenHeight - cardH) * 0.5f,
        cardW,
        cardH
    };

    DrawRectangleRounded(card, 0.15f, 6, Color{ 25, 32, 45, 250 });
    DrawRectangleRoundedLines(card, 0.15f, 6, Color{ 100, 130, 180, 255 });

    // Header banner
    const char* title = "LEVEL COMPLETE!";
    int titleW = MeasureText(title, 28);
    DrawText(title, (int)(card.x + (cardW - titleW) * 0.5f), (int)(card.y + 25), 28, Color{ 255, 220, 60, 255 });

    // Star rating display
    const char* starStr = (stars >= 3) ? "[ * * * ]" : (stars == 2) ? "[ * * - ]" : "[ * - - ]";
    if (stars == 3) {
        starStr = "STAR RATING: 3 / 3 (GOLD)";
    } else if (stars == 2) {
        starStr = "STAR RATING: 2 / 3 (SILVER)";
    } else {
        starStr = "STAR RATING: 1 / 3 (BRONZE)";
    }
    int starW = MeasureText(starStr, 20);
    DrawText(starStr, (int)(card.x + (cardW - starW) * 0.5f), (int)(card.y + 65), 20,
             (stars == 3) ? Color{ 255, 215, 0, 255 } : (stars == 2) ? Color{ 200, 220, 240, 255 } : Color{ 205, 135, 80, 255 });

    // Stats
    char timeStat[64];
    snprintf(timeStat, sizeof(timeStat), "Final Time: %.2f seconds (Par: %.1fs)", finalTime, parTime);
    DrawText(timeStat, (int)(card.x + 40), (int)(card.y + 115), 18, Color{ 230, 240, 255, 255 });

    char deathStat[48];
    snprintf(deathStat, sizeof(deathStat), "Total Deaths: %d", deaths);
    DrawText(deathStat, (int)(card.x + 40), (int)(card.y + 145), 18, (deaths == 0) ? Color{ 120, 240, 140, 255 } : Color{ 245, 120, 110, 255 });

    const char* rankMsg = (stars == 3) ? "Outstanding! Flawless speedrun!"
                        : (stars == 2) ? "Great job! Try cutting time for 3 stars!"
                                       : "Course finished! Keep practicing!";
    DrawText(rankMsg, (int)(card.x + 40), (int)(card.y + 185), 16, Color{ 170, 190, 210, 255 });

    // Buttons
    Rectangle retryBtn = { card.x + 40, card.y + 245, 175, 48 };
    Rectangle nextBtn  = { card.x + cardW - 40 - 175, card.y + 245, 175, 48 };

    bool retryClicked = DrawButton(retryBtn, "Retry", Color{ 60, 75, 100, 255 }, Color{ 85, 105, 135, 255 }, WHITE);
    bool nextClicked  = DrawButton(nextBtn, "Next Level", Color{ 45, 140, 85, 255 }, Color{ 65, 175, 110, 255 }, WHITE);

    if (retryClicked) return ResultsAction::RETRY;
    if (nextClicked) return ResultsAction::NEXT_LEVEL;

    return ResultsAction::NONE;
}

} // namespace UI
