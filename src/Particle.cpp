#include "Particle.hpp"
#include <random>

namespace AC_Engine
{
	// Helper to randomize particle properties
	inline float randomFloat(float min, float max)
	{
		static std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<float> dist(min, max);
		return dist(rng);
	}

	inline simd::Vec3f randomJitterVector(const simd::Vec3f& jitterRange)
	{
		return simd::Vec3f(
			randomFloat(-jitterRange.getX(), jitterRange.getX()),
			randomFloat(-jitterRange.getY(), jitterRange.getY()),
			randomFloat(-jitterRange.getZ(), jitterRange.getZ())
		);
	}

	// Particle methods
	bool Particle::isAlive() const
	{
		return life >= 0.f;
	}

	// ParticlePool methods
	ParticlePool::ParticlePool(size_t poolSize)
	{
		m_particles.resize(poolSize);
		for (size_t i = 0; i < poolSize - 1; ++i) {
			m_particles[i].nextFreeIndex = static_cast<int>(i + 1);
		}
		m_firstFreeIndex = 0;
	}

	Particle* ParticlePool::createParticle()
	{
		if (m_firstFreeIndex == -1) return nullptr;

		int index = m_firstFreeIndex;
		Particle& p = m_particles[index];
		m_firstFreeIndex = p.nextFreeIndex;
		return &p;
	}

	void ParticlePool::updateParticles(float deltaTime)
	{
		for (size_t i = 0; i < m_particles.size(); ++i) 
		{
			Particle& p = m_particles[i];
			if (p.isAlive()) 
			{
				p.life -= deltaTime;
				if (p.isAlive()) 
				{
					float lifeProgress{ 1.f - p.life / p.maxLife };
					p.size = p.startSize + (p.endSize - p.startSize) * lifeProgress;
					if (p.size < 0.f) p.size = 0.f;

					p.velocity += p.acceleration * deltaTime;
					if (p.drag > 0.f) p.velocity *= 1.f - p.drag * deltaTime;

					p.position += p.velocity * deltaTime;
				} 
				else 
				{
					p.nextFreeIndex = m_firstFreeIndex;
					m_firstFreeIndex = static_cast<int>(i);
				}
			}
		}
	}

	void ParticlePool::updateEmitters(ECS::Registry& registry, float deltaTime)
	{
		for (auto&& [emitter, transform] : registry.view<ParticleEmitterComponent, TransformComponent>())
		{
			emitter.timeSinceLastSpawn += deltaTime;
			float interval = 1.0f / emitter.spawnRate;
			while (emitter.timeSinceLastSpawn >= interval) 
			{
				Particle* p = createParticle();
				if (p) 
				{
					p->position = transform.position;
					p->acceleration = emitter.acceleration;
					p->velocity = emitter.velocity + randomJitterVector(emitter.velocityVariation);

					float finalLife = emitter.ParticleLife + randomFloat(-emitter.lifeVariation, emitter.lifeVariation);
					p->life = finalLife > 0.f ? finalLife : 0.f;
					p->maxLife = p->life;

					float baseSize = emitter.ParticleSize + randomFloat(-emitter.sizeVariation, emitter.sizeVariation);
					p->startSize = baseSize * emitter.startSizeMultiplier;
					p->endSize = baseSize * emitter.endSizeMultiplier;
					p->size = p->startSize;

					p->drag = emitter.drag;

					p->color = 0xFFFFFFFF;
					p->mat = emitter.mat;
				}
				emitter.timeSinceLastSpawn -= interval;
			}
		}
	}

	const std::vector<Particle>& ParticlePool::getParticles() const
	{
		return m_particles;
	}
}
