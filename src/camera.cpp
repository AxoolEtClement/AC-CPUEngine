#include "camera.hpp"

simd::Matrix4x4 Camera::look_at()
{
	simd::Vec3f right{ up.Cross(forward).Normalize() };
	simd::Vec3f camUp{ forward.Cross(right) };

	return simd::Matrix4x4{{ 
		right.getX(), right.getY(), right.getZ(), -right.Dot(pos),
		camUp.getX(), camUp.getY(), camUp.getZ(), -camUp.Dot(pos),
		forward.getX(), forward.getY(), forward.getZ(), -forward.Dot(pos),
		0.f, 0.f, 0.f, 1.f
	} };
}
