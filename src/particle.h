#pragma once
#include "raylib.h"
#include "object_pool.h"

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
    void SpawnSkidPuff(Vector3 feetPos, Vector3 horizVel);
    void SpawnCheckpointBurst(Vector3 padPos);
    void SpawnConfettiBurst(Vector3 center);
    void Update(float dt);
    void Draw() const;
    void Clear();

    size_t GetActiveCount() const { return m_pool.activeCount(); }

private:
    ObjectPool<Particle> m_pool;
};
