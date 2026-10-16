#include "Mat4x4.hpp"
#include "Vec3f.hpp"
#include <vector>

struct Vertex
{
    simd::Vec3f pos;
};

struct renderObject
{
    simd::Matrix4x4 modelMatrix{};
    std::vector<Vertex> vectices{};
    std::vector<std::array<size_t, 3>> faces{};

    // TODO : add texture id ...
};