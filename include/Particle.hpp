#pragma once
#include <vector>
#include "Vec3f.hpp"
#include "material.hpp"
#include "ECS/registry.hpp"
#include "ECS/components/particleEmitterComponent.hpp"
#include "ECS/components/transformComponent.hpp"

namespace AC_Engine
{
    struct Particle {
        simd::Vec3f position;
        simd::Vec3f velocity;
        simd::Vec3f acceleration;
        uint32_t color = 0x00000000;

        float size = 0.1f;
        float startSize = 0.1f;
		float endSize = 0.1f;
        float drag = 0.f;
        
        float life = 0.0f;
		float maxLife = 1.0f;
		int nextFreeIndex = -1;
        Material* mat = nullptr;

        Particle* nextFreeParticle = nullptr;
        bool isAlive() const;
    };

    class ParticlePool {
    public:
        ParticlePool(size_t poolSize);
        Particle* createParticle();
        void updateParticles(float deltaTime);
		void updateEmitters(AC_Engine::ECS::Registry& registry, float deltaTime);

        const std::vector<Particle>& getParticles() const;
    private:
        std::vector<Particle> m_particles;
        int m_firstFreeIndex = -1;
    };
}