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

		simd::Matrix4x4 view() const
		{
			simd::Vec3f right{ up.Cross(forward).Normalize() };
			simd::Vec3f camUp{ forward.Cross(right).Normalize() };

			return simd::Matrix4x4{ {
				right.getX(), right.getY(), right.getZ(), -right.Dot(pos),
				camUp.getX(), camUp.getY(), camUp.getZ(), -camUp.Dot(pos),
				-forward.getX(), -forward.getY(), -forward.getZ(), forward.Dot(pos),
				0.f, 0.f, 0.f, 1.f
			} };
		}

		simd::Matrix4x4 projection() const
		{
			float yScale = 1 / std::tan(fov / 2);
			float xScale = yScale / aspect;
			float A = (far + near) / (near - far);
			float B = (2 * far * near) / (near - far);
			return simd::Matrix4x4{ {
				xScale, 0.f, 0.f, 0.f,
				0.f, yScale, 0.f, 0.f,
				0.f, 0.f, A, B,
				0.f, 0.f, -1.f, 0.f
			} };
		}
	};
}
