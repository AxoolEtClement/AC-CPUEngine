#include "Particle.hpp"

namespace AC_Engine
{
	// Particle methods
	bool Particle::isAlive()
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

	const std::vector<Particle>& ParticlePool::getParticles() const
	{
		return m_particles;
	}

	// ParticleEmitter methods
	void ParticleEmitter::update(float deltaTime, ParticlePool& pool)
	{
		m_timeSinceLastSpawn += deltaTime;
		float interval = 1.0f / m_spawnRate;
		while (m_timeSinceLastSpawn >= interval) {
			Particle* p = pool.createParticle();
			if (p) {
				p->position = m_position;
				p->velocity = m_velocity;
				p->color = 0xFFFFFFFF; // White color
				p->size = m_ParticleSize;
				p->life = m_ParticleLife;
				p->maxLife = m_ParticleLife;
			}
			m_timeSinceLastSpawn -= interval;
		}
	}
}
