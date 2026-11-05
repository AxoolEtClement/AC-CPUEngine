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
		Material* mat;

		simd::Vec3f velocityVariation{ 0.0f, 0.0f, 0.0f };
		float lifeVariation{ 0.f };
		float sizeVariation{ 0.f };

		float startSizeMultiplier{ 1.f };
		float endSizeMultiplier{ 0.f };
		float drag{ 0.f };
		simd::Vec3f acceleration{ 0.0f, 0.0f, 0.0f };

		float timeSinceLastSpawn{ 0.f };
	};
}
