#pragma once
#include "Mat4x4.hpp"
#include "Vec4.hpp"
#include <math.h>
#include <GLFW/glfw3.h>
#include <iostream>

class Camera
{
public:
	Camera() = default;
	
	simd::Matrix4x4 look_at();
	simd::Matrix4x4 perspective();
	simd::Vec3f pos{ 1.f, 1.f, 1.f };
	void updateInfos(int width, int height, float fov, float near, float far)
	{
		this->fov = fov;
		this->near = near;
		this->far = far;
		this->aspect = static_cast<float>(width) / static_cast<float>(height);
	};
	virtual void update(GLFWwindow& window,float deltaTime) = 0;
	simd::Vec3f getForward() const { return forward; }
	simd::Vec3f getUp() const { return up; }

private:
	
	simd::Vec3f forward{ -0.57735f, -0.57735f, -0.57735f };
	// TODO : Fix up vector
	const simd::Vec3f up{ 0.f, -1.f, 0.f };
	float fov{ 90.f };
	float aspect{ 1.f };
	float near{ 0.1f };
	float far{ 100.f };
};
