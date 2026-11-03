#pragma once
#include "Vec3f.hpp"

namespace AC_Engine
{
	struct CameraComponent
	{
		CameraComponent(simd::Vec3f _pos, simd::Vec3f _forward)
			: pos(_pos), forward(_forward) {
		}

		simd::Vec3f pos{};
		simd::Vec3f forward{};
		simd::Vec3f up{ 0.f, 1.f, 0.f };
		float fov{ 90.f };
		float aspect{ 1.f };
		float near{ 0.1f };
		float far{ 100.f };
	};
}
