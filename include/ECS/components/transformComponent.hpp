#pragma once
#include "Vec3f.hpp"

namespace AC_Engine::ECS
{
	struct Transform
	{
		simd::Vec3f pos{ 0.f, 0.f, 0.f };
		simd::Vec3f scale{ 1.f, 1.f, 1.f };
	};
}
