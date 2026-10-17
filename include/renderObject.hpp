#pragma once
#include "Mat4x4.hpp"
#include "Vec4.hpp"
#include <vector>

struct Vertex
{
    simd::Vec4 pos;
    std::pair<float, float> texCoord; // (u, v)
};

struct renderObject
{
    simd::Matrix4x4 modelMatrix{};
    std::vector<Vertex> vertices{};
    std::vector<std::array<size_t, 3>> faces{};
    std::vector<std::pair<float, float>> uvs{};
    std::vector<int> face_uvs{};

    // TODO : add texture id ...
};