#include "Particle.hpp"

namespace AC_Engine
{
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
		for (size_t i = 0; i < m_particles.size(); ++i) {
			Particle& p = m_particles[i];
			if (p.isAlive()) {
				p.life -= deltaTime;
				if (p.isAlive()) {
					p.velocity += p.acceleration * deltaTime;
					p.position += p.velocity * deltaTime;
				} else {
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
			while (emitter.timeSinceLastSpawn >= interval) {
				Particle* p = createParticle();
				if (p) {
					p->position = transform.position;
					p->velocity = emitter.velocity;
					p->color = 0xFFFFFFFF;
					p->size = emitter.ParticleSize;
					p->life = emitter.ParticleLife;
					p->maxLife = emitter.ParticleLife;
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
