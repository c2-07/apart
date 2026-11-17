#include "particle.h"
#include "raymath.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

static float RandRange(float minVal, float maxVal) {
    float r = static_cast<float>(rand() % 10000) / 10000.0f;
    return minVal + r * (maxVal - minVal);
}

ParticleSystem::ParticleSystem() {
    particles.reserve(64);
}

void ParticleSystem::SpawnJumpPuff(Vector3 feetPos) {
    constexpr int count = 8;
    for (int i = 0; i < count; ++i) {
        float angle = (2.0f * PI * i) / count + RandRange(0.0f, 0.4f);
        float speed = RandRange(1.8f, 3.0f);
        Particle p;
        p.position = Vector3{
            feetPos.x + cosf(angle) * 0.2f,
            feetPos.y + 0.1f,
            feetPos.z + sinf(angle) * 0.2f
        };
        p.velocity = Vector3{
            cosf(angle) * speed,
            RandRange(1.2f, 2.0f),
            sinf(angle) * speed
        };
        p.size = RandRange(0.18f, 0.26f);
        p.maxLife = RandRange(0.35f, 0.50f);
        p.life = p.maxLife;
        p.color = Color{ 230, 240, 255, 230 };
        particles.push_back(p);
    }
}

void ParticleSystem::SpawnLandPuff(Vector3 feetPos, float impactSpeed) {
    int count = std::clamp(static_cast<int>(impactSpeed * 0.8f), 8, 16);
    for (int i = 0; i < count; ++i) {
        float angle = (2.0f * PI * i) / count + RandRange(0.0f, 0.3f);
        float speed = 2.0f + (impactSpeed * 0.15f) + RandRange(0.0f, 1.5f);
        Particle p;
        p.position = Vector3{
            feetPos.x + cosf(angle) * 0.3f,
            feetPos.y + 0.05f,
            feetPos.z + sinf(angle) * 0.3f
        };
        p.velocity = Vector3{
            cosf(angle) * speed,
            RandRange(0.8f, 1.8f),
            sinf(angle) * speed
        };
        p.size = RandRange(0.22f, 0.32f);
        p.maxLife = RandRange(0.4f, 0.6f);
        p.life = p.maxLife;
        p.color = Color{ 220, 230, 245, 240 };
        particles.push_back(p);
    }
}

void ParticleSystem::SpawnCheckpointBurst(Vector3 padPos) {
    constexpr int count = 24;
    Color colors[3] = {
        Color{ 255, 220, 50, 255 }, // Gold
        Color{ 60, 240, 120, 255 }, // Emerald
        Color{ 100, 225, 255, 255 } // Cyan
    };
    for (int i = 0; i < count; ++i) {
        float angle = (2.0f * PI * i) / count + RandRange(-0.2f, 0.2f);
        float speed = RandRange(2.5f, 5.0f);
        Particle p;
        p.position = Vector3{
            padPos.x + cosf(angle) * 0.4f,
            padPos.y + 0.15f,
            padPos.z + sinf(angle) * 0.4f
        };
        p.velocity = Vector3{
            cosf(angle) * speed,
            RandRange(3.5f, 6.5f),
            sinf(angle) * speed
        };
        p.size = RandRange(0.16f, 0.26f);
        p.maxLife = RandRange(0.6f, 0.95f);
        p.life = p.maxLife;
        p.color = colors[i % 3];
        particles.push_back(p);
    }
}

void ParticleSystem::SpawnConfettiBurst(Vector3 center) {
    constexpr int count = 60;
    Color palette[5] = {
        Color{ 255, 215, 0, 255 },   // Gold
        Color{ 50, 220, 240, 255 },  // Cyan
        Color{ 255, 60, 150, 255 },  // Magenta
        Color{ 60, 240, 100, 255 },  // Emerald
        Color{ 255, 140, 40, 255 }   // Sunset Orange
    };
    for (int i = 0; i < count; ++i) {
        float angle = RandRange(0.0f, 2.0f * PI);
        float pitch = RandRange(-0.3f, 1.2f);
        float speed = RandRange(3.0f, 8.0f);
        Particle p;
        p.position = Vector3{
            center.x + RandRange(-0.5f, 0.5f),
            center.y + RandRange(0.5f, 2.5f),
            center.z + RandRange(-0.5f, 0.5f)
        };
        p.velocity = Vector3{
            cosf(angle) * cosf(pitch) * speed,
            sinf(pitch) * speed + 3.0f,
            sinf(angle) * cosf(pitch) * speed
        };
        p.size = RandRange(0.18f, 0.32f);
        p.maxLife = RandRange(1.2f, 1.8f);
        p.life = p.maxLife;
        p.color = palette[i % 5];
        particles.push_back(p);
    }
}

void ParticleSystem::Update(float dt) {
    for (size_t i = 0; i < particles.size();) {
        Particle& p = particles[i];
        p.life -= dt;
        if (p.life <= 0.0f) {
            particles[i] = particles.back();
            particles.pop_back();
            continue;
        }

        p.position = Vector3Add(p.position, Vector3Scale(p.velocity, dt));
        p.velocity.y -= 12.0f * dt;
        p.velocity.x *= (1.0f - dt * 2.5f);
        p.velocity.z *= (1.0f - dt * 2.5f);

        ++i;
    }
}

void ParticleSystem::Draw() const {
    for (const auto& p : particles) {
        float progress = p.life / p.maxLife;
        float currentSize = p.size * progress;
        Color c = p.color;
        c.a = static_cast<unsigned char>(progress * 255.0f);
        DrawCube(p.position, currentSize, currentSize, currentSize, c);
        DrawCubeWires(p.position, currentSize + 0.01f, currentSize + 0.01f, currentSize + 0.01f,
                      Color{ static_cast<unsigned char>(c.r * 0.7f),
                             static_cast<unsigned char>(c.g * 0.7f),
                             static_cast<unsigned char>(c.b * 0.7f), c.a });
    }
}

void ParticleSystem::Clear() {
    particles.clear();
}
