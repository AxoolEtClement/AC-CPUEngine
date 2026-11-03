#pragma once
#include "Particle.hpp"
#include "material.hpp"

namespace AC_Engine
{
	struct ParticleEmitterComponent
	{
		ParticleEmitterComponent(simd::Vec3f velocity,float spawnRate, float particleLife, float particleSize, Material* mat)
			: velocity(velocity), spawnRate(spawnRate), ParticleLife(particleLife), ParticleSize(particleSize), mat(mat)
		{
		}

		simd::Vec3f velocity;
		float spawnRate;
		float ParticleLife;
		float ParticleSize;
		float timeSinceLastSpawn = 0.0f;
		Material* mat;
	};
}
