#include "Mat4x4.hpp"
#include "Vec3f.hpp"
#include <vector>

struct renderObject
{
	simd::Matrix4x4 modelMatrix{};
	std::vector<simd::Vec3f> vectices{};

	// TODO : add texture id ...
};
