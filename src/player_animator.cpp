#include "player_animator.h"
#include "constants.h"
#include "audio.h"
#include "collision.h"
#include "rlgl.h"
#include "raymath.h"

#include <cmath>
#include <algorithm>

namespace obby {

PlayerAnimator::PlayerAnimator()
    : m_walkCycle(0.0f),
      m_landingTimer(0.0f),
      m_skidTimer(0.0f) {}

void PlayerAnimator::reset() {
    m_walkCycle = 0.0f;
    m_landingTimer = 0.0f;
    m_skidTimer = 0.0f;
}

Color PlayerAnimator::darkenColor(Color c, float factor) {
    return Color{
        static_cast<unsigned char>(c.r * factor),
        static_cast<unsigned char>(c.g * factor),
        static_cast<unsigned char>(c.b * factor),
        c.a
    };
}

void PlayerAnimator::update(float dt, bool isGrounded, bool wasGrounded, float horizontalSpeed,
                           float previousFallSpeed, float wishSpeed, const Vec3& wishDir,
                           const Vec3& velocity, const Vec3& position, ParticleSystem* particles) {
    if (m_landingTimer > 0.0f) {
        m_landingTimer = std::max(0.0f, m_landingTimer - dt);
    }
    if (m_skidTimer > 0.0f) {
        m_skidTimer = std::max(0.0f, m_skidTimer - dt);
    }

    // Landing event (transition from airborne to grounded with downward speed)
    if (!wasGrounded && isGrounded && previousFallSpeed < -3.5f) {
        AudioManager::Play(SoundId::LAND);
        m_landingTimer = Constants::LANDING_SQUASH_DURATION;
        if (particles) {
            particles->SpawnLandPuff(position.toRaylib(), std::abs(previousFallSpeed));
        }
    }

    // Skid dust puffs on sharp turns at speed
    if (isGrounded && horizontalSpeed > 4.5f && wishSpeed > 0.1f) {
        float velDot = (velocity.x * wishDir.x + velocity.z * wishDir.z) / horizontalSpeed;
        if (velDot < -0.2f && m_skidTimer <= 0.0f) {
            m_skidTimer = 0.10f;
            if (particles) {
                particles->SpawnSkidPuff(position.toRaylib(), Vector3{ velocity.x, 0.0f, velocity.z });
            }
        }
    }

    // Animation progress
    if (isGrounded && horizontalSpeed > 0.5f) {
        m_walkCycle += horizontalSpeed * dt * 2.2f;
    } else if (isGrounded) {
        m_walkCycle = 0.0f;
    }
}

void PlayerAnimator::drawShadow(const Vec3& position, const std::vector<AABB>& colliders) const {
    float hitDist = 0.0f;
    Vector3 rayOrigin = { position.x, position.y + 0.1f, position.z };
    Vector3 rayDir = { 0.0f, -1.0f, 0.0f };

    if (RaycastScene(rayOrigin, rayDir, 0.01f, Constants::SHADOW_MAX_DISTANCE, colliders, &hitDist)) {
        float shadowY = rayOrigin.y - hitDist + Constants::SHADOW_OFFSET_Y;
        float progress = std::clamp(hitDist / Constants::SHADOW_MAX_DISTANCE, 0.0f, 1.0f);

        // Radius scales smoothly with elevation
        float outerRadius = Constants::SHADOW_BASE_RADIUS - progress * (Constants::SHADOW_BASE_RADIUS - Constants::SHADOW_MIN_RADIUS);
        float innerRadius = outerRadius * 0.55f;

        // Quadratic falloff for natural contact shadow look
        float fade = (1.0f - progress);
        fade = fade * fade;
        unsigned char outerAlpha = static_cast<unsigned char>(fade * (Constants::SHADOW_MAX_ALPHA * 0.5f));
        unsigned char innerAlpha = static_cast<unsigned char>(fade * Constants::SHADOW_MAX_ALPHA);

        // Outer soft fringe
        DrawCube(Vector3{ position.x, shadowY, position.z }, outerRadius, Constants::SHADOW_HEIGHT, outerRadius, Color{ 15, 20, 30, outerAlpha });
        // Inner dense core
        DrawCube(Vector3{ position.x, shadowY + 0.002f, position.z }, innerRadius, Constants::SHADOW_HEIGHT, innerRadius, Color{ 10, 15, 25, innerAlpha });
    }
}

void PlayerAnimator::draw(const Vec3& position, float modelYaw, const Vec3& velocity,
                          bool isGrounded, Color shirtColor) const {
    rlPushMatrix();
    rlTranslatef(position.x, position.y, position.z);
    rlRotatef(modelYaw * RAD2DEG, 0.0f, 1.0f, 0.0f);

    // Squash and stretch on landing
    if (m_landingTimer > 0.0f) {
        float squashT = m_landingTimer / Constants::LANDING_SQUASH_DURATION;
        float scaleY = 1.0f - (1.0f - Constants::LANDING_SQUASH_FACTOR) * squashT;
        float scaleXZ = 1.0f + 0.5f * (1.0f - scaleY);
        rlScalef(scaleXZ, scaleY, scaleXZ);
    }

    float hSpeed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
    float speedRatio = std::clamp(hSpeed / Constants::RUN_SPEED, 0.0f, 1.0f);

    // Slight lean into movement direction
    if (isGrounded && speedRatio > 0.05f) {
        rlRotatef(speedRatio * Constants::LEAN_MAX_ANGLE, 1.0f, 0.0f, 0.0f);
    }

    float bobY = 0.0f;
    float leftLegRot = 0.0f;
    float rightLegRot = 0.0f;
    float leftArmRot = 0.0f;
    float rightArmRot = 0.0f;
    float armSplay = 0.0f;

    if (isGrounded) {
        if (hSpeed > 0.5f) {
            // Speed-proportional walking animation
            bobY = fabsf(sinf(m_walkCycle)) * (0.05f * speedRatio);
            float legSwing = sinf(m_walkCycle) * (34.0f * speedRatio);
            float armSwing = sinf(m_walkCycle) * (32.0f * speedRatio);
            leftLegRot = legSwing;
            rightLegRot = -legSwing;
            leftArmRot = -armSwing;
            rightArmRot = armSwing;
        } else {
            // Small idle breathing bob and subtle sway
            float timeVal = static_cast<float>(GetTime());
            bobY = sinf(timeVal * 2.5f) * 0.015f;
            float idleSway = sinf(timeVal * 2.0f) * 2.5f;
            leftArmRot = idleSway;
            rightArmRot = -idleSway;
        }
    } else {
        // Airborne poses
        if (velocity.y > 0.0f) { // Jumping: arms reach up high
            leftArmRot = 135.0f;
            rightArmRot = 135.0f;
            armSplay = 12.0f;
            leftLegRot = -18.0f;
            rightLegRot = -12.0f;
        } else { // Falling: arms outstretched, legs dangle
            leftArmRot = 100.0f;
            rightArmRot = 100.0f;
            armSplay = 18.0f;
            leftLegRot = 8.0f;
            rightLegRot = -8.0f;
        }
    }

    Color shirt = shirtColor;
    Color shirtWire = darkenColor(shirt, 0.65f);
    Color skin = Color{ 255, 225, 90, 255 }; // Classic blocky yellow tone
    Color skinWire = darkenColor(skin, 0.7f);
    Color pants = Color{ 45, 60, 95, 255 };
    Color pantsWire = darkenColor(pants, 0.65f);

    // 1. Torso
    Vector3 torsoPos = { 0.0f, 1.20f + bobY, 0.0f };
    Vector3 torsoSize = { 0.8f, 0.9f, 0.45f };
    DrawCube(torsoPos, torsoSize.x, torsoSize.y, torsoSize.z, shirt);
    DrawCubeWires(torsoPos, torsoSize.x + 0.02f, torsoSize.y + 0.02f, torsoSize.z + 0.02f, shirtWire);

    // 2. Head
    Vector3 headPos = { 0.0f, 1.95f + bobY, 0.0f };
    Vector3 headSize = { 0.55f, 0.55f, 0.55f };
    DrawCube(headPos, headSize.x, headSize.y, headSize.z, skin);
    DrawCubeWires(headPos, headSize.x + 0.02f, headSize.y + 0.02f, headSize.z + 0.02f, skinWire);

    // Face Details on +Z front
    DrawCube(Vector3{ 0.12f, 2.00f + bobY, 0.28f }, 0.07f, 0.07f, 0.02f, Color{ 30, 30, 40, 255 }); // Left Eye
    DrawCube(Vector3{ -0.12f, 2.00f + bobY, 0.28f }, 0.07f, 0.07f, 0.02f, Color{ 30, 30, 40, 255 }); // Right Eye
    DrawCube(Vector3{ 0.0f, 1.86f + bobY, 0.28f }, 0.20f, 0.04f, 0.02f, Color{ 40, 40, 50, 255 }); // Smile

    // 3. Left Leg
    rlPushMatrix();
    rlTranslatef(0.22f, 0.75f, 0.0f);
    rlRotatef(leftLegRot, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{ 0.0f, -0.375f, 0.0f }, 0.34f, 0.75f, 0.34f, pants);
    DrawCubeWires(Vector3{ 0.0f, -0.375f, 0.0f }, 0.35f, 0.76f, 0.35f, pantsWire);
    rlPopMatrix();

    // 4. Right Leg
    rlPushMatrix();
    rlTranslatef(-0.22f, 0.75f, 0.0f);
    rlRotatef(rightLegRot, 1.0f, 0.0f, 0.0f);
    DrawCube(Vector3{ 0.0f, -0.375f, 0.0f }, 0.34f, 0.75f, 0.34f, pants);
    DrawCubeWires(Vector3{ 0.0f, -0.375f, 0.0f }, 0.35f, 0.76f, 0.35f, pantsWire);
    rlPopMatrix();

    // 5. Left Arm
    rlPushMatrix();
    rlTranslatef(0.55f, 1.55f + bobY, 0.0f);
    rlRotatef(leftArmRot, 1.0f, 0.0f, 0.0f);
    if (armSplay != 0.0f) {
        rlRotatef(armSplay, 0.0f, 0.0f, 1.0f);
    }
    // Upper arm (sleeved)
    DrawCube(Vector3{ 0.0f, -0.2f, 0.0f }, 0.28f, 0.4f, 0.28f, shirt);
    DrawCubeWires(Vector3{ 0.0f, -0.2f, 0.0f }, 0.29f, 0.41f, 0.29f, shirtWire);
    // Lower arm / hand (skin)
    DrawCube(Vector3{ 0.0f, -0.55f, 0.0f }, 0.28f, 0.35f, 0.28f, skin);
    DrawCubeWires(Vector3{ 0.0f, -0.55f, 0.0f }, 0.29f, 0.36f, 0.29f, skinWire);
    rlPopMatrix();

    // 6. Right Arm
    rlPushMatrix();
    rlTranslatef(-0.55f, 1.55f + bobY, 0.0f);
    rlRotatef(rightArmRot, 1.0f, 0.0f, 0.0f);
    if (armSplay != 0.0f) {
        rlRotatef(-armSplay, 0.0f, 0.0f, 1.0f);
    }
    // Upper arm (sleeved)
    DrawCube(Vector3{ 0.0f, -0.2f, 0.0f }, 0.28f, 0.4f, 0.28f, shirt);
    DrawCubeWires(Vector3{ 0.0f, -0.2f, 0.0f }, 0.29f, 0.41f, 0.29f, shirtWire);
    // Lower arm / hand (skin)
    DrawCube(Vector3{ 0.0f, -0.55f, 0.0f }, 0.28f, 0.35f, 0.28f, skin);
    DrawCubeWires(Vector3{ 0.0f, -0.55f, 0.0f }, 0.29f, 0.36f, 0.29f, skinWire);
    rlPopMatrix();

    rlPopMatrix();
}

} // namespace obby
