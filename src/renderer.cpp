#include "renderer.hpp"
#include <iostream>

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

                if ((e0 | e1 | e2) >= 0) {
                    m_frameBuffer.setPixel(x, y, pack(e0, e1, e2, area));
                }
            }
        }
    }

    void Renderer::renderObjectsWireframe(std::vector <renderObject> renderObjects, Camera cam)
    {
        int width = m_frameBuffer.getWidth();
        int height = m_frameBuffer.getHeight();

        simd::Matrix4x4 VP{ cam.perspective() * cam.look_at() };
        for (const auto& obj : renderObjects) {
            std::vector<simd::Vec3f> verticesNdc;
            std::vector<bool> visible;
            simd::Matrix4x4 MVP{ VP * obj.modelMatrix };
            float dmin = 1e30f, dmax = -1e30f;

            for (const auto& vertex : obj.vertices)
            {
                simd::Vec4 clip{ MVP * vertex.pos };
                if (clip.getW() > 0.0001f)
                {
                    visible.push_back(true);
                    float ndc_x = clip.getX() / clip.getW();
                    float ndc_y = clip.getY() / clip.getW();

                    verticesNdc.push_back({ ndc_x, ndc_y, clip.getW() });
                } 
                else
                {
                    visible.push_back(false);
                    verticesNdc.push_back({});
                }
                if (clip.getW() < dmin) dmin = clip.getW();
                if (clip.getW() > dmax) dmax = clip.getW();
            }
            float drange = (dmax > dmin) ? (dmax - dmin) : 1.0f;
            for (const auto& face : obj.faces) {
                if (!visible[face[0]] || !visible[face[1]] || !visible[face[2]]) continue;
                simd::Vec3f& v0 = verticesNdc[face[0]];
                simd::Vec3f& v1 = verticesNdc[face[1]];
                simd::Vec3f& v2 = verticesNdc[face[2]];

                float avgDepth = (v0.getZ() + v1.getZ() + v2.getZ()) / 3.f;
                float normalizedDepth = (avgDepth - dmin) / drange;

                uint8_t level = 240 - static_cast<uint8_t>(normalizedDepth * 200);

                uint32_t color = 0xFF000000 | (level << 16) | (level << 8) | (level);

                int screen_x0 = static_cast<int>((v0.getX() + 1.0f) * 0.5f * width);
                int screen_y0 = static_cast<int>((1.f - v0.getY()) * 0.5f * height);

                int screen_x1 = static_cast<int>((v1.getX() + 1.0f) * 0.5f * width);
                int screen_y1 = static_cast<int>((1.f - v1.getY()) * 0.5f * height);

                int screen_x2 = static_cast<int>((v2.getX() + 1.0f) * 0.5f * width);
                int screen_y2 = static_cast<int>((1.f - v2.getY()) * 0.5f * height);

                drawLine(screen_x0, screen_y0, screen_x1, screen_y1, color);
                drawLine(screen_x0, screen_y0, screen_x2, screen_y2, color);
                drawLine(screen_x2, screen_y2, screen_x1, screen_y1, color);
            }
        }
        
        /*for (const auto& obj : edges)
        {
            simd::Vec4 v0clip{ MVP * obj.first };
            simd::Vec4 v1clip{ MVP * obj.second };

            if (v0clip.getW() <= 0.0001f || v1clip.getW() <= 0.0001f)
            {
                continue;
            }

            float ndc_x0 = v0clip.getX() / v0clip.getW();
            float ndc_y0 = v0clip.getY() / v0clip.getW();

            float ndc_x1 = v1clip.getX() / v1clip.getW();
            float ndc_y1 = v1clip.getY() / v1clip.getW();

            int screen_x0 = static_cast<int>((ndc_x0 + 1.f) * 0.5 * m_frameBuffer.getWidth());
            int screen_y0 = static_cast<int>((1.f - (ndc_y0 + 1.0f) * 0.5f) * m_frameBuffer.getHeight());

            int screen_x1 = static_cast<int>((ndc_x1 + 1.f) * 0.5 * m_frameBuffer.getWidth());
            int screen_y1 = static_cast<int>((1.f - (ndc_y1 + 1.0f) * 0.5f) * m_frameBuffer.getHeight());

            drawLine(screen_x0, screen_y0, screen_x1, screen_y1, 0xFFFFFFFF);
        }*/
    }
}