#pragma once
#include <string>
#include <vector>
#include <cmath>
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

        void generateTerrain(float scale, float persistence, int octaves);
        void generateObj(const std::string& filename);

    protected:
        
        void applyErosion(int iterations, float erosionFactor);
        void smoothTerrain(int iterations);
        void normalizeTerrain();
        void saveHeightmap(const std::string& filename) const;
        
    private:

        inline float dotGridGradient(int ix, int iy, float x, float y) 
        {
            // Compute the distance vector
            float dx = x - static_cast<float>(ix);
            float dy = y - static_cast<float>(iy);

            // Compute the gradient vector (using a simple hash function)
            int hash = (ix * 73856093) ^ (iy * 19349663);
            float gradientX = static_cast<float>(static_cast<int>(hash & 0xFF) - 128) / 128.0f;
            float gradientY = static_cast<float>(static_cast<int>((hash >> 8) & 0xFF) - 128) / 128.0f;

            if (gradientX == 0.0f && gradientY == 0.0f)
            {
                gradientX = 1.0f;
                gradientY = 0.0f;
            }

            // Normalize the gradient vector
            float length = std::sqrt(gradientX * gradientX + gradientY * gradientY);
            if (length > 0.0f) 
            {
                gradientX /= length;
                gradientY /= length;
            }

            // Compute the dot product
            return (dx * gradientX + dy * gradientY);
        };

        inline float lerp(float a0, float a1, float w) 
        {
            return (1.0f - w) * a0 + w * a1;
        };

        inline float perlin(float x, float y) 
        {
            int x0 = static_cast<int>(std::floor(x));
            int x1 = x0 + 1;
            int y0 = static_cast<int>(std::floor(y));
            int y1 = y0 + 1;

            float sx = x - static_cast<float>(x0);
            float sy = y - static_cast<float>(y0);

            float n0, n1, ix0, ix1, value;

            n0 = dotGridGradient(x0, y0, x, y);
            n1 = dotGridGradient(x1, y0, x, y);
            ix0 = lerp(n0, n1, sx);

            n0 = dotGridGradient(x0, y1, x, y);
            n1 = dotGridGradient(x1, y1, x, y);
            ix1 = lerp(n0, n1, sx);

            value = lerp(ix0, ix1, sy);
            return value;
        };

    private:
        std::vector<TerrainVertex> m_heightmap;
        int m_width;
        int m_height;
    };
}