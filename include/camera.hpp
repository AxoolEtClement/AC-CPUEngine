#pragma once
#include "Mat4x4.hpp"
#include "Vec4.hpp"
#include <math.h>

class Camera
{
public:
	Camera() = default;
	
	simd::Matrix4x4 look_at();
	simd::Matrix4x4 perspective();

private:
	simd::Vec3f pos{ 2.f, 2.f, 2.f };
	simd::Vec3f forward{ -0.57735f, -0.57735f, -0.57735f };
	const simd::Vec3f up{ 0.f, 1.f, 0.f };
	float fov{ 90.f };
	float aspect{ 1.f };
	float near{ 0.1f };
	float far{ 100.f };
};
