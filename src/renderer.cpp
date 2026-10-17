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
    }

    void Renderer::renderObjects(std::vector<std::pair<renderObject, Texture>>& renderObjects, Camera cam)
    {
        int width = m_frameBuffer.getWidth();
        int height = m_frameBuffer.getHeight();
        m_frameBuffer.clearZBuffer();
        simd::Vec3f lightDir{ 0.57735f, 0.57735f, 0.57735f };

        simd::Matrix4x4 VP{ cam.perspective() * cam.look_at() };
        for (const auto& obj : renderObjects) {
            std::vector<bool> visible;
            simd::Matrix4x4 MVP{ VP * obj.first.modelMatrix };

            std::vector<float> ndc_z = {};
            std::vector<float> ndc_w = {};
            std::vector<int> screen_x = {};
            std::vector<int> screen_y = {};

            for (const auto& vertex : obj.first.vertices)
            {
                simd::Vec4 clip{ MVP * vertex.pos };
                if (clip.getW() > 0.0001f)
                {
                    float inv = 1.f / clip.getW();
                    screen_x.push_back(static_cast<int>((clip.getX() * inv + 1.0f) * 0.5f * width));
                    screen_y.push_back(static_cast<int>((1.f - clip.getY() * inv) * 0.5f * height));
                    ndc_z.push_back(clip.getZ() * inv);
                    ndc_w.push_back(inv);
                    visible.push_back(true);
                }
                else
                {
                    visible.push_back(false);
                    screen_x.push_back(0);
                    screen_y.push_back(0);
                    ndc_z.push_back(0.f);
                    ndc_w.push_back(0.f);
                }
            }
            float drange = 1.0f;
            for (size_t f = 0; f < obj.first.faces.size(); f++) {
                const auto& face = obj.first.faces[f];
                int a = face[0];
                int b = face[1];
                int c = face[2];
                if (!visible[face[0]] || !visible[face[1]] || !visible[face[2]]) continue;
                if (barycentricWeights(screen_x[a], screen_y[a], screen_x[b], screen_y[b], screen_x[c], screen_y[c]) <= 0) continue;

                simd::Vec3f p0 = {obj.first.vertices[a].pos.getX(), obj.first.vertices[a].pos.getY(), obj.first.vertices[a].pos.getZ() };
                simd::Vec3f p1 = {obj.first.vertices[b].pos.getX(), obj.first.vertices[b].pos.getY(), obj.first.vertices[b].pos.getZ() };
                simd::Vec3f p2 = {obj.first.vertices[c].pos.getX(), obj.first.vertices[c].pos.getY(), obj.first.vertices[c].pos.getZ() };

                simd::Vec3f edge1 = p1 - p0;
                simd::Vec3f edge2 = p2 - p0;
                simd::Vec3f n = obj.first.modelMatrix.TransformDirection(edge1.Cross(edge2)).Normalize();
                float ndotl = n.Dot(lightDir);
                if (ndotl < 0.0f) ndotl = 0.0f;
                float brightness = ambiantLightIntensity + (1.0f - ambiantLightIntensity) * ndotl;

                int ta = obj.first.face_uvs[f * 3 + 0];
                int tb = obj.first.face_uvs[f * 3 + 1];
                int tc = obj.first.face_uvs[f * 3 + 2];
                std::pair<float,float> uva = obj.first.uvs[ta];
                std::pair<float,float> uvb = obj.first.uvs[tb];
                std::pair<float,float> uvc = obj.first.uvs[tc];

                drawTriangleTexturedAndLit(screen_x[a], screen_y[a], ndc_z[a], ndc_w[a], uva, screen_x[b], screen_y[b], ndc_z[b], ndc_w[b], uvb, screen_x[c], screen_y[c], ndc_z[c], ndc_w[c], uvc, obj.second, brightness);
            }
        }
    }

    float Renderer::barycentricWeights(int x0, int y0, int x1, int y1, int x2, int y2)
    {
        return (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
    }

    void Renderer::drawTriangleTexturedAndLit(int x0, int y0, float z0, float iw0, std::pair<float, float> uv0, int x1, int y1, float z1, float iw1, std::pair<float, float> uv1, int x2, int y2, float z2, float iw2, std::pair<float, float> uv2, const Texture& texture, float brightness)
    {
        int area = barycentricWeights(x0, y0, x1, y1, x2, y2);
        if (area <= 0) return;

        int minX = std::max(0, std::min({ x0, x1, x2 }));
        int maxX = std::min(m_frameBuffer.getWidth() - 1, std::max({ x0, x1, x2 }));
        int minY = std::max(0, std::min({ y0, y1, y2 }));
        int maxY = std::min(m_frameBuffer.getHeight() - 1, std::max({ y0, y1, y2 }));

        float invArea = 1.f / static_cast<float>(area);

        float u0w = uv0.first * iw0, v0w = uv0.second * iw0;
        float u1w = uv1.first * iw1, v1w = uv1.second * iw1;
        float u2w = uv2.first * iw2, v2w = uv2.second * iw2;

        int A0 = y1 - y2, B0 = x2 - x1;
        int A1 = y2 - y0, B1 = x0 - x2;
        int A2 = y0 - y1, B2 = x1 - x0;

        float dzdx = (A0 * z0 + A1 * z1 + A2 * z2) * invArea;
        float dzdy = (B0 * z0 + B1 * z1 + B2 * z2) * invArea;

        float duwdx = (A0 * u0w + A1 * u1w + A2 * u2w) * invArea;
        float duwdy = (B0 * u0w + B1 * u1w + B2 * u2w) * invArea;

        float dvwdx = (A0 * v0w + A1 * v1w + A2 * v2w) * invArea;
        float dvwdy = (B0 * v0w + B1 * v1w + B2 * v2w) * invArea;

        float diwdx = (A0 * iw0 + A1 * iw1 + A2 * iw2) * invArea;
        float diwdy = (B0 * iw0 + B1 * iw1 + B2 * iw2) * invArea;

        int start_e0 = barycentricWeights(x1, y1, x2, y2, minX, minY);
        int start_e1 = barycentricWeights(x2, y2, x0, y0, minX, minY);
        int start_e2 = barycentricWeights(x0, y0, x1, y1, minX, minY);

        float init_e0 = start_e0 * invArea;
        float init_e1 = start_e1 * invArea;
        float init_e2 = start_e2 * invArea;

        float row_z  = init_e0 * z0  + init_e1 * z1  + init_e2 * z2;
        float row_uw = init_e0 * u0w + init_e1 * u1w + init_e2 * u2w;
        float row_vw = init_e0 * v0w + init_e1 * v1w + init_e2 * v2w;
        float row_iw = init_e0 * iw0 + init_e1 * iw1 + init_e2 * iw2;

        for (int y = minY; y <= maxY; y++)
        {
            int e0 = start_e0;
            int e1 = start_e1;
            int e2 = start_e2;

            float z  = row_z;
            float uw = row_uw;
            float vw = row_vw;
            float iw = row_iw;

            bool inside = false;

            for (int x = minX; x <= maxX; x++)
            {

                if ((e0 | e1 | e2) < 0) 
                {
                    if (inside) break; // If we were inside the triangle and now we're outside, we can break early
                    e0 += A0; e1 += A1; e2 += A2;
                    z  += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx;
                    continue;
                }
                inside = true; // We are inside the triangle
                int idx = y * m_frameBuffer.getWidth() + x;

                if (z >= m_frameBuffer.getZBuffer()[idx])
                {
                    e0 += A0; e1 += A1; e2 += A2;
                    z  += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx;
                    continue;
                }
                float u = uw / iw;
                float v = vw / iw;

                m_frameBuffer.getZBuffer()[idx] = z;

                uint32_t color = TextureLoader::sampleTexture(texture, u, v);
                m_frameBuffer.setPixel(x, y, TextureLoader::color_scale(color, brightness));

                e0 += A0; e1 += A1; e2 += A2;
                z  += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx;
            }

            start_e0 += B0;
            start_e1 += B1;
            start_e2 += B2;

            row_z  += dzdy;
            row_uw += duwdy;
            row_vw += dvwdy;
            row_iw += diwdy;
        }
    }
    void Renderer::drawTriangle(int x0, int y0, float z0, int x1, int y1, float z1, int x2, int y2, float z2)
    {
        int area = barycentricWeights(x0, y0, x1, y1, x2, y2);
        if (area <= 0.f) return;

        int minX = std::max(0, std::min({ x0, x1, x2 }));
        int maxX = std::min(m_frameBuffer.getWidth() - 1, std::max({ x0, x1, x2 }));
        int minY = std::max(0, std::min({ y0, y1, y2 }));
        int maxY = std::min(m_frameBuffer.getHeight() - 1, std::max({ y0, y1, y2 }));

        float invArea = 1.f / static_cast<float>(area);
        for (int y = minY; y <= maxY; y++)
            for (int x = minX; x <= maxX; x++)
            {
                int e0 = barycentricWeights(x1, y1, x2, y2, x, y);
                int e1 = barycentricWeights(x2, y2, x0, y0, x, y);
                int e2 = barycentricWeights(x0, y0, x1, y1, x, y);

                if ((e0 | e1 | e2) < 0) continue;

                float normalizedE0 = e0 * invArea;
                float normalizedE1 = e1 * invArea;
                float normalizedE2 = e2 * invArea;

                float z = normalizedE0 * z0 + normalizedE1 * z1 + normalizedE2 * z2;

                int idx = y * m_frameBuffer.getWidth() + x;

                if (z >= m_frameBuffer.getZBuffer()[idx]) continue;
                m_frameBuffer.getZBuffer()[idx] = z;

                m_frameBuffer.setPixel(x, y, 0xFFFFFFFF);
            }
    }
}