#pragma once
#include "Mat4x4.hpp"
#include "Vec4.hpp"
#include <vector>
#include <array>
#include <utility>

struct Vertex
{
    simd::Vec4 pos;
    std::pair<float, float> texCoord; // (u, v)
	simd::Vec3f normal;
};

struct renderObject
{
    simd::Matrix4x4 modelMatrix{};
    std::vector<Vertex> vertices{};
    std::vector<std::array<size_t, 3>> faces{};
    std::vector<std::pair<float, float>> uvs{};
    std::vector<int> face_uvs{};

    int materialId = 0;

    void initNormals()     {
        
        for (auto& vertex : vertices)
        {
            vertex.normal = simd::Vec3f{ 0.f, 0.f, 0.f };
        }
        
        for (const auto& face : faces)
        {
            const auto& v0 = vertices[face[0]].pos;
            const auto& v1 = vertices[face[1]].pos;
            const auto& v2 = vertices[face[2]].pos;
            simd::Vec3f edge1 = { v1.getX() - v0.getX(), v1.getY() - v0.getY(), v1.getZ() - v0.getZ() };
            simd::Vec3f edge2 = { v2.getX() - v0.getX(), v2.getY() - v0.getY(), v2.getZ() - v0.getZ() };
            simd::Vec3f faceNormal = edge1.Cross(edge2).Normalize();
            vertices[face[0]].normal += faceNormal;
            vertices[face[1]].normal += faceNormal;
            vertices[face[2]].normal += faceNormal;
        }
        
        for (auto& vertex : vertices)
        {
            if(vertex.normal == simd::Vec3f{ 0.f, 0.f, 0.f })
            {
                vertex.normal = simd::Vec3f{ 0.f, 1.f, 0.f }; // Default normal if no faces contribute
            }
            else
            {
                vertex.normal = vertex.normal.Normalize();
            }
        }
	}
};