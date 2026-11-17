#pragma once
#include "raylib.h"
#include <vector>

struct Particle {
    Vector3 position;
    Vector3 velocity;
    float size;
    float life;
    float maxLife;
    Color color;
};

class ParticleSystem {
public:
    ParticleSystem();
    void SpawnJumpPuff(Vector3 feetPos);
    void SpawnLandPuff(Vector3 feetPos, float impactSpeed);
    void Update(float dt);
    void Draw() const;
    void Clear();

private:
    std::vector<Particle> particles;
};
