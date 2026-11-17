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

void DrawHUD(const std::string& levelName, float timer, float parTime,
             int checkpoints, int totalCheckpoints, int deaths,
             float hintAlpha, float titleAlpha) {
    // 1. Polished HUD Panel with Drop Shadow & Consistent Padding
    Rectangle hudPanel = { 20, 20, 340, 115 };
    // Subtle drop shadow
    DrawRectangleRounded(Rectangle{ hudPanel.x + 3.0f, hudPanel.y + 3.0f, hudPanel.width, hudPanel.height }, 0.2f, 4, Color{ 0, 0, 0, 75 });
    // Main panel background
    DrawRectangleRounded(hudPanel, 0.2f, 4, Color{ 22, 28, 40, 230 });
    // Slightly lighter rounded border
    DrawRectangleRoundedLines(hudPanel, 0.2f, 4, Color{ 95, 120, 165, 220 });

    float padX = hudPanel.x + 18.0f;
    float curY = hudPanel.y + 14.0f;

    // Level Title
    DrawText(levelName.c_str(), (int)padX, (int)curY, 20, Color{ 250, 252, 255, 255 });
    curY += 26.0f;

    // Color-Graded Timer: green under par, yellow near par (within 125%), red over par
    Color timeColor;
    if (timer <= parTime) {
        timeColor = Color{ 85, 230, 120, 255 }; // Under par: Vibrant Green
    } else if (timer <= parTime * 1.25f) {
        timeColor = Color{ 245, 215, 60, 255 }; // Near par: Bright Yellow
    } else {
        timeColor = Color{ 245, 85, 75, 255 };  // Over par: Warning Red
    }
    char timeBuf[64];
    snprintf(timeBuf, sizeof(timeBuf), "Time: %.2fs (Par: %.1fs)", timer, parTime);
    DrawText(timeBuf, (int)padX, (int)curY, 16, timeColor);
    curY += 22.0f;

    // Checkpoints with Gold Completion Highlight
    char cpBuf[64];
    snprintf(cpBuf, sizeof(cpBuf), "Checkpoints: %d / %d", checkpoints, totalCheckpoints);
    Color cpColor = (checkpoints == totalCheckpoints && totalCheckpoints > 0)
        ? Color{ 255, 220, 60, 255 }
        : Color{ 215, 225, 240, 255 };
    DrawText(cpBuf, (int)padX, (int)curY, 16, cpColor);
    curY += 22.0f;

    // Deaths Counter
    char deathBuf[32];
    snprintf(deathBuf, sizeof(deathBuf), "Deaths: %d", deaths);
    Color deathColor = (deaths == 0) ? Color{ 145, 230, 155, 255 } : Color{ 245, 110, 100, 255 };
    DrawText(deathBuf, (int)padX, (int)curY, 16, deathColor);

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    // 2. Large Animated Level Title at Level Start (Fades over 2s)
    if (titleAlpha > 0.01f) {
        int titleSize = 34;
        int titleW = MeasureText(levelName.c_str(), titleSize);
        float bannerW = titleW + 64.0f;
        float bannerH = 56.0f;
        float bannerX = (screenWidth - bannerW) * 0.5f;
        float bannerY = 80.0f;
        unsigned char bannerBgAlpha = static_cast<unsigned char>(titleAlpha * 210.0f);
        unsigned char bannerTextAlpha = static_cast<unsigned char>(titleAlpha * 255.0f);

        Rectangle bannerRec = { bannerX, bannerY, bannerW, bannerH };
        DrawRectangleRounded(Rectangle{ bannerRec.x + 3.0f, bannerRec.y + 3.0f, bannerRec.width, bannerRec.height }, 0.25f, 4, Color{ 0, 0, 0, static_cast<unsigned char>(bannerBgAlpha * 0.6f) });
        DrawRectangleRounded(bannerRec, 0.25f, 4, Color{ 18, 24, 36, bannerBgAlpha });
        DrawRectangleRoundedLines(bannerRec, 0.25f, 4, Color{ 255, 215, 60, static_cast<unsigned char>(titleAlpha * 180.0f) });
        DrawText(levelName.c_str(), (int)(bannerX + (bannerW - titleW) * 0.5f), (int)(bannerY + (bannerH - titleSize) * 0.5f), titleSize, Color{ 255, 225, 75, bannerTextAlpha });
    }

    // 3. Fading Control Hints at Bottom (Fades out after 8s of play, reappears when paused)
    if (hintAlpha > 0.01f) {
        const char* hint = "WASD: Move | Space: Jump | R: Respawn | ESC: Pause | G: Ghost";
        int hintWidth = MeasureText(hint, 15);
        unsigned char bgAlpha = static_cast<unsigned char>(hintAlpha * 190.0f);
        unsigned char textAlpha = static_cast<unsigned char>(hintAlpha * 230.0f);

        Rectangle hintBox = { (float)(screenWidth / 2 - hintWidth / 2 - 16), (float)(screenHeight - 40), (float)(hintWidth + 32), 28.0f };
        DrawRectangleRounded(Rectangle{ hintBox.x + 2.0f, hintBox.y + 2.0f, hintBox.width, hintBox.height }, 0.35f, 4, Color{ 0, 0, 0, static_cast<unsigned char>(bgAlpha * 0.5f) });
        DrawRectangleRounded(hintBox, 0.35f, 4, Color{ 16, 22, 32, bgAlpha });
        DrawRectangleRoundedLines(hintBox, 0.35f, 4, Color{ 80, 105, 140, static_cast<unsigned char>(hintAlpha * 150.0f) });
        DrawText(hint, screenWidth / 2 - hintWidth / 2, screenHeight - 34, 15, Color{ 225, 235, 245, textAlpha });
    }
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
