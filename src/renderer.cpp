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
		simd::Matrix4x4 translation{ simd::Matrix4x4::Translation(simd::Vec3f{ 320.f, 240.f, 0.f }) };
    
        v0 = rotation * v0;
        v1 = rotation * v1;
        v2 = rotation * v2;

		simd::Vec4 transformedV0 = translation * v0;
		simd::Vec4 transformedV1 = translation * v1;
		simd::Vec4 transformedV2 = translation * v2;

        int x0{ static_cast<int>(transformedV0.getX()) };
        int x1{ static_cast<int>(transformedV1.getX()) };
        int x2{ static_cast<int>(transformedV2.getX()) };

        int y0{ static_cast<int>(transformedV0.getY()) };
        int y1{ static_cast<int>(transformedV1.getY()) };
        int y2{ static_cast<int>(transformedV2.getY()) };

        auto edge = [](int x0, int y0, int x1, int y1, int x, int y) -> int {
            return (y0 - y1) * (x - x1) + (x1 - x0) * (y - y1);
            };

        auto pack = [](float r, float g, float b, float area) -> uint32_t {
            return ((0xFF << 24) | static_cast<uint32_t>(r * 255 / area) << 16) | (static_cast<uint32_t>(g * 255 / area) << 8) | static_cast<uint32_t>(b * 255 / area);
            };

        int area = edge(x0, y0, x1, y1, x2, y2);
        if (area <= 0) return;

        int minX = std::max(0, std::min({ x0, x1, x2 }));
        int maxX = std::min(m_frameBuffer.getWidth() - 1, std::max({ x0, x1, x2 }));
        int minY = std::max(0, std::min({ y0, y1, y2 }));
        int maxY = std::min(m_frameBuffer.getHeight() - 1, std::max({ y0, y1, y2 }));

        for (int y = minY; y <= maxY; y++) {
            for (int x = minX; x <= maxX; x++) {
				int e0 = edge(x1, y1, x2, y2, x, y);
				int e1 = edge(x2, y2, x0, y0, x, y);
				int e2 = edge(x0, y0, x1, y1, x, y);

                if (e0 >= 0 && e1 >= 0 && e2 >= 0) {
                    m_frameBuffer.setPixel(x, y, pack(e0, e1, e2, area));
                }
            }
        }
    }
}