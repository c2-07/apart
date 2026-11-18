#include "play_state.h"
#include "game.h"
#include "ui.h"
#include "constants.h"
#include "raylib.h"
#include "rlgl.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

namespace obby {

PlayState::PlayState(Game& game)
    : GameState(game),
      m_heartShakeTimer(0.0f),
      m_previousHearts(Constants::MAX_HEARTS) {}

void PlayState::handleInput() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        m_game.RequestStateChange(StateType::PAUSED);
        return;
    }

    // F1 Cheat: Noclip
    if (IsKeyPressed(KEY_F1)) {
        m_game.noclipMode = !m_game.noclipMode;
        m_game.player.setVelocity(Vec3{ 0.0f, 0.0f, 0.0f });
    }

    // T Cheat: Teleport
    if (IsKeyPressed(KEY_T)) {
        bool teleported = false;
        for (const auto* cp : m_game.level.getCheckpoints()) {
            if (!cp->isActivated()) {
                m_game.player.setPosition(cp->getPosition());
                m_game.player.setVelocity(Vec3{ 0.0f, 0.0f, 0.0f });
                teleported = true;
                break;
            }
        }
        if (!teleported && m_game.level.getFinishZone() != nullptr) {
            m_game.player.setPosition(m_game.level.getFinishZone()->getBounds().getCenter());
            m_game.player.setVelocity(Vec3{ 0.0f, 0.0f, 0.0f });
        }
    }

    // G: Ghost Clone Toggle
    if (IsKeyPressed(KEY_G)) {
        if (m_game.level.ghost.state == GhostRecorder::State::IDLE) {
            m_game.level.ghost.StartRecording();
        } else if (m_game.level.ghost.state == GhostRecorder::State::RECORDING) {
            m_game.level.ghost.StopRecording();
            m_game.player.setPosition(m_game.level.spawnPos);
            m_game.player.setVelocity(Vec3{ 0.0f, 0.0f, 0.0f });
        } else {
            m_game.level.ghost.Reset();
        }
    }

    // F3: Toggle FPS Display
    if (IsKeyPressed(KEY_F3)) {
        m_game.showFps = !m_game.showFps;
    }

    // F2 or C: Toggle between 2D Side-Scroller and 3D Orbit Camera
    if (IsKeyPressed(KEY_F2) || IsKeyPressed(KEY_C)) {
        m_game.Toggle2DMode();
    }
}

void PlayState::update(float frameDelta) {
    handleInput();

    std::vector<AABB> colliders = m_game.level.GetSolidColliders();

    // Camera update with screen shake
    Vec3 camFocus = m_game.player.getPosition();
    if (m_game.screenShakeTimer > 0.0f) {
        m_game.screenShakeTimer -= frameDelta;
        float shakeProgress = m_game.screenShakeTimer / Constants::SCREEN_SHAKE_DURATION;
        float shakeMag = Constants::SCREEN_SHAKE_MAGNITUDE * shakeProgress;
        auto randOffset = [shakeMag]() {
            return (static_cast<float>(rand() % 2000) / 1000.0f - 1.0f) * shakeMag;
        };
        camFocus.x += randOffset();
        camFocus.y += randOffset();
        camFocus.z += randOffset();
    }

    m_game.camera.update(camFocus, frameDelta, colliders, m_game.save.sensitivity, m_game.save.invertY, m_game.player.getVelocity());
    m_game.particles.Update(frameDelta);

    m_game.levelPlayTime += frameDelta;

    if (m_game.player.getHearts() < m_previousHearts) {
        m_heartShakeTimer = Constants::HEART_SHAKE_DURATION;
    }
    m_previousHearts = m_game.player.getHearts();
    if (m_heartShakeTimer > 0.0f) {
        m_heartShakeTimer = std::max(0.0f, m_heartShakeTimer - frameDelta);
    }

    // Smooth number animation for checkpoint counter
    float targetCp = static_cast<float>(m_game.level.GetActivatedCheckpointCount());
    m_game.displayedCheckpoints += (targetCp - m_game.displayedCheckpoints) * std::min(1.0f, frameDelta * Constants::UI_CHECKPOINT_LERP_SPEED);

    if (m_game.respawnFadeAlpha > 0.0f) {
        m_game.respawnFadeAlpha = std::max(0.0f, m_game.respawnFadeAlpha - frameDelta * Constants::RESPAWN_FADE_SPEED);
    }
    if (m_game.levelLoadFadeAlpha > 0.0f) {
        m_game.levelLoadFadeAlpha = std::max(0.0f, m_game.levelLoadFadeAlpha - frameDelta * Constants::LEVEL_LOAD_FADE_SPEED);
    }

    if (m_game.noclipMode) {
        float flySpeed = 16.0f;
        float yaw = m_game.camera.getYawRadians();
        Vec3 forward = { -sinf(yaw), 0.0f, -cosf(yaw) };
        Vec3 right   = {  cosf(yaw), 0.0f, -sinf(yaw) };

        Vec3 pos = m_game.player.getPosition();
        if (IsKeyDown(KEY_W)) pos += forward * (flySpeed * frameDelta);
        if (IsKeyDown(KEY_S)) pos -= forward * (flySpeed * frameDelta);
        if (IsKeyDown(KEY_D)) pos += right * (flySpeed * frameDelta);
        if (IsKeyDown(KEY_A)) pos -= right * (flySpeed * frameDelta);
        if (IsKeyDown(KEY_SPACE)) pos.y += flySpeed * frameDelta;
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_C)) pos.y -= flySpeed * frameDelta;
        m_game.player.setPosition(pos);

        m_game.level.Update(frameDelta, m_game.player, &m_game.particles);
    } else {
        m_game.accumulator += frameDelta;
        while (m_game.accumulator >= Constants::PHYSICS_DT) {
            PlayerInput input;
            if (IsKeyDown(KEY_W)) input.moveForward += 1.0f;
            if (IsKeyDown(KEY_S)) input.moveForward -= 1.0f;
            if (IsKeyDown(KEY_D)) input.moveRight   += 1.0f;
            if (IsKeyDown(KEY_A)) input.moveRight   -= 1.0f;
            input.jumpPressed    = IsKeyPressed(KEY_SPACE);
            input.jumpHeld       = IsKeyDown(KEY_SPACE);
            input.respawnPressed = IsKeyPressed(KEY_R);

            m_game.level.Update(Constants::PHYSICS_DT, m_game.player, &m_game.particles);
            colliders = m_game.level.GetSolidColliders();
            m_game.player.updatePhysics(Constants::PHYSICS_DT, input, m_game.camera.getYawRadians(), colliders, &m_game.particles);

            if (m_game.level.deaths > m_game.previousDeaths) {
                m_game.previousDeaths = m_game.level.deaths;
                m_game.screenShakeTimer = Constants::SCREEN_SHAKE_DURATION;
                m_game.respawnFadeAlpha = 0.85f;
            } else if (input.respawnPressed) {
                m_game.respawnFadeAlpha = 0.85f;
            }

            m_game.accumulator -= Constants::PHYSICS_DT;
        }
    }

    if (m_game.level.isCompleted) {
        m_game.CompleteLevel();
    }
}

void PlayState::draw() const {
    std::vector<AABB> colliders = m_game.level.GetSolidColliders();

    m_game.level.DrawSkybox();
    BeginMode3D(m_game.camera.getRaylibCamera());
    m_game.ApplyLightingShader(m_game.camera.getCurrentEye().toRaylib(), m_game.level.worldId);
    m_game.level.Draw();
    m_game.player.draw();
    if (m_game.shaderLoaded) EndShaderMode();

    // Transparent pass with depth writing disabled
    rlDisableDepthMask();
    m_game.player.drawShadow(colliders);
    m_game.particles.Draw();
    rlEnableDepthMask();

    EndMode3D();

    float maxFade = std::max(m_game.respawnFadeAlpha, m_game.levelLoadFadeAlpha);
    if (maxFade > 0.001f) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                      Color{ 10, 15, 25, static_cast<unsigned char>(255 * maxFade) });
    }

    // Dynamic HUD control hint fade (visible for 6s, then fades over next 2s)
    float hintAlpha = 0.0f;
    if (m_game.state == AppState::PAUSED) {
        hintAlpha = 1.0f;
    } else if (m_game.levelPlayTime < Constants::UI_CONTROL_HINT_FADE_START) {
        hintAlpha = 1.0f;
    } else if (m_game.levelPlayTime < Constants::UI_CONTROL_HINT_FADE_END) {
        hintAlpha = 1.0f - (m_game.levelPlayTime - Constants::UI_CONTROL_HINT_FADE_START) /
                           (Constants::UI_CONTROL_HINT_FADE_END - Constants::UI_CONTROL_HINT_FADE_START);
    }

    // Large level title banner at level start, fades out after 2s
    float titleAlpha = 0.0f;
    if (m_game.levelPlayTime < Constants::UI_LEVEL_TITLE_DURATION) {
        float t = m_game.levelPlayTime / Constants::UI_LEVEL_TITLE_DURATION;
        titleAlpha = (t < 0.7f) ? 1.0f : (1.0f - (t - 0.7f) / 0.3f);
    }

    UI::DrawHUD(m_game.level.name, m_game.level.elapsedTime, m_game.level.parTime,
                static_cast<int>(std::round(m_game.displayedCheckpoints)), m_game.level.GetTotalCheckpointCount(), m_game.level.deaths,
                m_game.player.getHearts(), m_game.player.getMaxHearts(), m_heartShakeTimer,
                m_game.player.getCoins(),
                hintAlpha, titleAlpha);

    // Context hint banner
    if (m_game.noclipMode) {
        DrawRectangle(20, 140, 460, 28, Color{ 220, 80, 40, 220 });
        DrawText("[NOCLIP ON] Fly: WASD+Space/Shift | T: Teleport to Checkpoint", 30, 146, 14, WHITE);
    } else if (m_game.level.ghost.state == GhostRecorder::State::RECORDING) {
        DrawRectangle(20, 140, 420, 28, Color{ 230, 50, 60, 200 });
        char ghostBuf[64];
        snprintf(ghostBuf, sizeof(ghostBuf), "[REC] RECORDING GHOST (%.1fs / 20s) - Press G to Finish", m_game.level.ghost.recordTimer);
        DrawText(ghostBuf, 30, 146, 14, WHITE);
    } else if (m_game.level.ghost.state == GhostRecorder::State::PLAYING) {
        DrawRectangle(20, 140, 420, 28, Color{ 40, 160, 220, 200 });
        DrawText("[PLAY] GHOST ACTIVE (Holding buttons) - Press G to Reset", 30, 146, 14, WHITE);
    }

    // Toggleable FPS counter (F3), off by default
    if (m_game.showFps) {
        DrawFPS(GetScreenWidth() - 85, 20);
    }

    // 2D/3D Mode toast banner
    if (m_game.modeToastTimer > 0.0f) {
        float alpha = std::min(1.0f, m_game.modeToastTimer);
        int tw = MeasureText(m_game.modeToastText.c_str(), 18);
        Rectangle banner = { (GetScreenWidth() - tw - 40) * 0.5f, 60.0f, (float)(tw + 40), 32.0f };
        DrawRectangleRounded(banner, 0.3f, 4, ColorAlpha(Color{ 15, 20, 30, 220 }, alpha));
        DrawRectangleRoundedLines(banner, 0.3f, 4, ColorAlpha(Color{ 70, 160, 240, 255 }, alpha));
        DrawText(m_game.modeToastText.c_str(), (int)(banner.x + 20), (int)(banner.y + 7), 18, ColorAlpha(Color{ 255, 220, 60, 255 }, alpha));
    }

    // Camera Mode badge in bottom-right corner
    const char* modeBadge = m_game.is2DMode ? "[F2/C] View: 2D Side-Scroller" : "[F2/C] View: 3D Orbit";
    DrawRectangle(GetScreenWidth() - 215, GetScreenHeight() - 30, 205, 22, Color{ 15, 20, 30, 180 });
    DrawText(modeBadge, GetScreenWidth() - 205, GetScreenHeight() - 26, 12, Color{ 180, 215, 255, 220 });
}

} // namespace obby
