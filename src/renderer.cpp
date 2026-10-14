#include "renderer.hpp"

namespace AC_Engine
{
    void Renderer::drawLine(int x0, int y0, int x1, int y1, unsigned int color)
    {
        // Bresenham's line algorithm
        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;

        while (true) {
            m_frameBuffer.setPixel(x0, y0, color);

            if (x0 == x1 && y0 == y1) break;
            int e2 = err * 2;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    }

    void Renderer::drawDemoRGBTriangle(simd::Vec4& v0, simd::Vec4& v1, simd::Vec4& v2, float t)
    {
        simd::Matrix4x4 rotation{ simd::Matrix4x4::RotationZ(t) };

        v0 = rotation * v0;
        v1 = rotation * v1;
        v2 = rotation * v2;

        int x0{ static_cast<int>(v0.getX()) };
        int x1{ static_cast<int>(v1.getX()) };
        int x2{ static_cast<int>(v2.getX()) };

        int y0{ static_cast<int>(v0.getY()) };
        int y1{ static_cast<int>(v1.getY()) };
        int y2{ static_cast<int>(v2.getY()) };


        auto pack = [](float r, float g, float b) -> uint32_t {
            auto c = [](float v) { return static_cast<uint32_t>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f); };
            return 0xFF000000u | (c(b) << 16) | (c(g) << 8) | c(r);
            };

        int det = (y1 - y2) * ( - x2) + (x2 - x1) * (y0 - y2);
        if (det == 0) return;
        int s = (det > 0) ? 1 : -1;
        float invDet = 1.0f / static_cast<float>(det * s);

        int minX = std::max(0, std::min({ x0, x1, x2 }));
        int maxX = std::min(m_frameBuffer.getWidth() - 1, std::max({ x0, x1, x2 }));
        int minY = std::max(0, std::min({ y0, y1, y2 }));
        int maxY = std::min(m_frameBuffer.getHeight() - 1, std::max({ y0, y1, y2 }));

        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
                int n0 = s * ((y1 - y2) * (x - x2) + (x2 - x1) * (y - y2));
                int n1 = s * ((y2 - y0) * (x - x2) + (x0 - x2) * (y - y2));
                int n2 = det * s - n0 - n1;

                if (n0 >= 0 && n1 >= 0 && n2 >= 0) {
                    m_frameBuffer.setPixel(x, y, pack(n0 * invDet, n1 * invDet, n2 * invDet));
                }
            }
        }
    }
}