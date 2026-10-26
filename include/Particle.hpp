#pragma once
#include <vector>
#include "Vec3f.hpp"
#include "material.hpp"

namespace AC_Engine
{
    struct Particle {
        simd::Vec3f position;
        simd::Vec3f velocity;
        simd::Vec3f acceleration;
        uint32_t color = 0x00000000;
        float size = 0.1f;
        float life = 0.0f;
		float maxLife = 1.0f;
		int nextFreeIndex = -1;
        Material* mat = nullptr;

        Particle* nextFreeParticle = nullptr;
        bool isAlive();
    };

    class ParticlePool {
    public:
        ParticlePool(size_t poolSize);
        Particle* createParticle();
        void updateParticles(float deltaTime);

        const std::vector<Particle>& getParticles() const;
    private:
        std::vector<Particle> m_particles;
        int m_firstFreeIndex = -1;
    };

    class ParticleEmitter {
    public:
        ParticleEmitter(const simd::Vec3f& position, float spawnRate, float particleLife, float particleSize, Material* mat);

        void update(float deltaTime, ParticlePool& pool);
    private:
        simd::Vec3f m_position;
		simd::Vec3f m_velocity;
        float m_spawnRate;
        float m_ParticleLife;
		float m_ParticleSize;
		float m_timeSinceLastSpawn = 0.0f;
        Material* m_mat;
    };
}