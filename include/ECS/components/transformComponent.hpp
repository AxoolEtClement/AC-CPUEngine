#pragma once
#include "Mat4x4.hpp"

namespace AC_Engine
{
	struct TransformComponent
	{
		TransformComponent() = default;
		TransformComponent(simd::Vec3f pos, simd::Vec3f scale, simd::Vec3f rotation)
			: position(pos), scale(scale), rotation(rotation) {}

		simd::Matrix4x4 getTransformMatrix() const
		{
			simd::Matrix4x4 translationMatrix = simd::Matrix4x4::Translation(position);
			simd::Matrix4x4 scaleMatrix = simd::Matrix4x4::Scale(scale);
			simd::Matrix4x4 rotationXMatrix = simd::Matrix4x4::RotationX(rotation.getX());
			simd::Matrix4x4 rotationYMatrix = simd::Matrix4x4::RotationY(rotation.getY());
			simd::Matrix4x4 rotationZMatrix = simd::Matrix4x4::RotationZ(rotation.getZ());
			return translationMatrix * rotationZMatrix * rotationYMatrix * rotationXMatrix * scaleMatrix;
		}

		simd::Vec3f position{ 0.0f, 0.0f, 0.0f };
		simd::Vec3f scale{ 1.0f, 1.0f, 1.0f };
		simd::Vec3f rotation{ 0.0f, 0.0f, 0.0f };
	};
}
