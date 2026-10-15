#pragma once
#include "Mat4x4.hpp"
#include "Vec4.hpp"

class Camera
{
public:
	Camera() = default;
	
	simd::Matrix4x4 look_at();
	simd::Matrix4x4 perspective(float fov, float aspect, float near, float far);

private:
	simd::Vec3f pos;
	simd::Vec3f forward;
	const simd::Vec3f up{ 0.f, 1.f, 0.f };
};
