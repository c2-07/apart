#include "ui.h"
#include "audio.h"
#include <cstdio>
#include <algorithm>

namespace UI {

bool Button(Rectangle bounds, const char* text, Color baseColor, Color hoverColor, Color textColor, int fontSize) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, bounds);
    Color col = hovered ? hoverColor : baseColor;

    DrawRectangleRounded(bounds, 0.22f, 4, col);
    DrawRectangleRoundedLines(bounds, 0.22f, 4, Color{ 255, 255, 255, 70 });

    int textWidth = MeasureText(text, fontSize);
    DrawText(text, (int)(bounds.x + (bounds.width - textWidth) * 0.5f),
                   (int)(bounds.y + (bounds.height - fontSize) * 0.5f), fontSize, textColor);

    return hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool Slider(Rectangle bounds, const char* label, float* value, float minVal, float maxVal, const char* format) {
    Vector2 mouse = GetMousePosition();
    bool hovering = CheckCollisionPointRec(mouse, bounds);
    static const void* activeSlider = nullptr;

    if (hovering && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        activeSlider = value;
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        activeSlider = nullptr;
    }

    bool changed = false;
    if (activeSlider == value) {
        float ratio = (mouse.x - bounds.x) / bounds.width;
        ratio = std::clamp(ratio, 0.0f, 1.0f);
        float newVal = minVal + ratio * (maxVal - minVal);
        if (std::abs(newVal - *value) > 1e-4f) {
            *value = newVal;
            changed = true;
        }
    }

    float norm = (*value - minVal) / (maxVal - minVal);
    norm = std::clamp(norm, 0.0f, 1.0f);

    // Label on left
    DrawText(label, (int)bounds.x, (int)(bounds.y - 20), 16, Color{ 220, 230, 245, 240 });

    // Track
    Rectangle track = { bounds.x, bounds.y + bounds.height * 0.3f, bounds.width, bounds.height * 0.4f };
    DrawRectangleRounded(track, 0.5f, 4, Color{ 40, 50, 65, 220 });

    Rectangle filled = { bounds.x, track.y, bounds.width * norm, track.height };
    DrawRectangleRounded(filled, 0.5f, 4, Color{ 70, 170, 240, 255 });

    // Thumb
    Rectangle thumb = { bounds.x + bounds.width * norm - 7.0f, bounds.y, 14.0f, bounds.height };
    DrawRectangleRounded(thumb, 0.4f, 4, (activeSlider == value || hovering) ? WHITE : Color{ 210, 225, 245, 255 });

    // Formatted value on right
    char valBuf[32];
    snprintf(valBuf, sizeof(valBuf), format, *value);
    DrawText(valBuf, (int)(bounds.x + bounds.width + 12), (int)(bounds.y + 2), 16, Color{ 200, 220, 240, 230 });

    return changed;
}

bool Toggle(Rectangle bounds, const char* label, bool* value) {
    Vector2 mouse = GetMousePosition();
    bool hovering = CheckCollisionPointRec(mouse, bounds);

    if (hovering && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        *value = !(*value);
        return true;
    }

    DrawText(label, (int)bounds.x, (int)(bounds.y + bounds.height * 0.2f), 18, Color{ 220, 230, 245, 240 });

    Rectangle toggleBox = { bounds.x + bounds.width - 70, bounds.y, 70, bounds.height };
    Color bg = *value ? Color{ 50, 160, 90, 240 } : Color{ 65, 75, 90, 220 };
    DrawRectangleRounded(toggleBox, 0.4f, 4, bg);
    DrawRectangleRoundedLines(toggleBox, 0.4f, 4, Color{ 255, 255, 255, 60 });

    const char* txt = *value ? "ON" : "OFF";
    int tw = MeasureText(txt, 16);
    DrawText(txt, (int)(toggleBox.x + (toggleBox.width - tw) * 0.5f),
                   (int)(toggleBox.y + (toggleBox.height - 16) * 0.5f), 16, WHITE);

    return false;
}

void DrawVolumeControl(int posX, int posY, float width) {
    float curVol = AudioManager::GetVolume();
    Rectangle bounds = { (float)posX, (float)posY, width, 18.0f };
    if (Slider(bounds, "Master Volume", &curVol, 0.0f, 1.0f, "%.0f%%")) {
        AudioManager::SetVolume(curVol);
    }
}

void DrawHUD(const std::string& levelName, float timer, float parTime, int checkpoints, int totalCheckpoints, int deaths) {
    Rectangle hudPanel = { 15, 15, 340, 115 };
    DrawRectangleRounded(hudPanel, 0.2f, 4, Color{ 20, 25, 35, 200 });
    DrawRectangleRoundedLines(hudPanel, 0.2f, 4, Color{ 70, 90, 120, 255 });

    DrawText(levelName.c_str(), 30, 25, 20, Color{ 255, 255, 255, 255 });

    char timeBuf[64];
    snprintf(timeBuf, sizeof(timeBuf), "Time: %.2fs (Par: %.1fs)", timer, parTime);
    Color timeColor = (timer <= parTime) ? Color{ 100, 240, 120, 255 } : Color{ 240, 190, 80, 255 };
    DrawText(timeBuf, 30, 52, 17, timeColor);

    char cpBuf[64];
    snprintf(cpBuf, sizeof(cpBuf), "Checkpoints: %d / %d", checkpoints, totalCheckpoints);
    DrawText(cpBuf, 30, 75, 16, Color{ 210, 220, 230, 255 });

    char deathBuf[32];
    snprintf(deathBuf, sizeof(deathBuf), "Deaths: %d", deaths);
    DrawText(deathBuf, 30, 97, 16, (deaths == 0) ? Color{ 150, 230, 150, 255 } : Color{ 245, 110, 100, 255 });

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    const char* hint = "WASD: Move | Space: Jump | R: Respawn | ESC: Pause | G: Ghost";
    int hintWidth = MeasureText(hint, 15);
    DrawRectangleRounded(Rectangle{ (float)(screenWidth / 2 - hintWidth / 2 - 15), (float)(screenHeight - 38), (float)(hintWidth + 30), 28 },
                         0.3f, 4, Color{ 15, 20, 30, 180 });
    DrawText(hint, screenWidth / 2 - hintWidth / 2, screenHeight - 32, 15, Color{ 220, 230, 240, 220 });
}

ResultsAction DrawResultsScreen(int screenWidth, int screenHeight, float finalTime, float parTime, int deaths, int stars) {
    DrawRectangle(0, 0, screenWidth, screenHeight, Color{ 10, 15, 25, 185 });

    float cardW = 480.0f;
    float cardH = 360.0f;
    Rectangle card = { (screenWidth - cardW) * 0.5f, (screenHeight - cardH) * 0.5f, cardW, cardH };

    DrawRectangleRounded(card, 0.15f, 6, Color{ 25, 32, 45, 250 });
    DrawRectangleRoundedLines(card, 0.15f, 6, Color{ 100, 130, 180, 255 });

    const char* title = "LEVEL COMPLETE!";
    int titleW = MeasureText(title, 28);
    DrawText(title, (int)(card.x + (cardW - titleW) * 0.5f), (int)(card.y + 25), 28, Color{ 255, 220, 60, 255 });

    const char* starStr = (stars == 3) ? "[ * * * ]  GOLD RANK!"
                        : (stars == 2) ? "[ * * - ]  SILVER RANK!"
                                       : "[ * - - ]  BRONZE RANK!";
    int starW = MeasureText(starStr, 22);
    Color starCol = (stars == 3) ? Color{ 255, 215, 0, 255 } : (stars == 2) ? Color{ 200, 220, 245, 255 } : Color{ 210, 140, 80, 255 };
    DrawText(starStr, (int)(card.x + (cardW - starW) * 0.5f), (int)(card.y + 70), 22, starCol);

    char timeStat[64];
    snprintf(timeStat, sizeof(timeStat), "Final Time: %.2f seconds (Par: %.1fs)", finalTime, parTime);
    DrawText(timeStat, (int)(card.x + 40), (int)(card.y + 120), 18, Color{ 230, 240, 255, 255 });

    char deathStat[48];
    snprintf(deathStat, sizeof(deathStat), "Total Deaths: %d", deaths);
    DrawText(deathStat, (int)(card.x + 40), (int)(card.y + 150), 18, (deaths == 0) ? Color{ 120, 240, 140, 255 } : Color{ 245, 120, 110, 255 });

    const char* rankMsg = (stars == 3) ? "Incredible speedrun mastery!"
                        : (stars == 2) ? "Great run! Shave off a few seconds for 3 stars!"
                                       : "Good job! Try again for a faster time!";
    DrawText(rankMsg, (int)(card.x + 40), (int)(card.y + 190), 16, Color{ 170, 190, 210, 255 });

    Rectangle retryBtn = { card.x + 30, card.y + 260, 125, 45 };
    Rectangle nextBtn  = { card.x + 170, card.y + 260, 140, 45 };
    Rectangle menuBtn  = { card.x + 325, card.y + 260, 125, 45 };

    if (Button(retryBtn, "Retry", Color{ 60, 75, 100, 255 }, Color{ 85, 105, 135, 255 }, WHITE, 18)) return ResultsAction::RETRY;
    if (Button(nextBtn, "Next Level", Color{ 45, 140, 85, 255 }, Color{ 65, 175, 110, 255 }, WHITE, 18)) return ResultsAction::NEXT_LEVEL;
    if (Button(menuBtn, "Menu", Color{ 100, 60, 75, 255 }, Color{ 135, 80, 95, 255 }, WHITE, 18)) return ResultsAction::MENU;

    return ResultsAction::NONE;
}

} // namespace UI
