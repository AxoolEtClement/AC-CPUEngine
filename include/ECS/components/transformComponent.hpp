#pragma once
#include "Mat4x4.hpp"

namespace AC_Engine
{
	struct TransformComponent
	{
		TransformComponent() = default;
		TransformComponent(simd::Vec3f pos, simd::Vec3f scale)
		{
			transform *= simd::Matrix4x4::Scale(scale) * simd::Matrix4x4::Translation(pos);
		}

		simd::Matrix4x4 transform{};
	};
}
