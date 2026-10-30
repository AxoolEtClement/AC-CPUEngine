#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>

namespace AC_Engine
{
    struct TerrainVertex
    {
        float x;
        float y;
    };
    class ProceduralTerrain
    {
    public:
        ProceduralTerrain(int width, int height);

        void generateTerrain(float scale, float persistence, int octaves, float heightScale = 15.0f);
        void generateObj(const std::string& filename);

    protected:
        void applyErosion(int iterations, float erosionFactor);
        void smoothTerrain(int iterations);
        void normalizeTerrain();
        void saveHeightmap(const std::string& filename) const;

    private:
        inline float fade(float t)
        {
            return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
        }

        inline float lerp(float a0, float a1, float w)
        {
            return (1.0f - w) * a0 + w * a1;
        }

        inline float dotGridGradient(int ix, int iy, float x, float y)
        {
            float dx = x - static_cast<float>(ix);
            float dy = y - static_cast<float>(iy);

            uint32_t h = static_cast<uint32_t>(ix) * 73856093u ^ static_cast<uint32_t>(iy) * 19349663u;
            h ^= h >> 13; h *= 0x5bd1e995u; h ^= h >> 15;

            float angle = (h & 0xFFFFu) / 65535.0f * 6.2831853f;
            return dx * std::cos(angle) + dy * std::sin(angle);
        }

        inline float perlin(float x, float y)
        {
            int x0 = static_cast<int>(std::floor(x));
            int y0 = static_cast<int>(std::floor(y));
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            float sx = fade(x - static_cast<float>(x0));
            float sy = fade(y - static_cast<float>(y0));

            float ix0 = lerp(dotGridGradient(x0, y0, x, y), dotGridGradient(x1, y0, x, y), sx);
            float ix1 = lerp(dotGridGradient(x0, y1, x, y), dotGridGradient(x1, y1, x, y), sx);
            return lerp(ix0, ix1, sy);
        }

    private:
        std::vector<TerrainVertex> m_heightmap;
        int m_width;
        int m_height;
    };
}