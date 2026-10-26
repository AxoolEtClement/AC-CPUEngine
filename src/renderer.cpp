#include "renderer.hpp"
#include <iostream>

namespace AC_Engine
{
    Renderer::Renderer(FrameBuffer& frameBuffer, MaterialLibrary& materialLibrary)
        : m_frameBuffer(frameBuffer), m_materialLibrary(materialLibrary), 
        m_numThreads(std::max(2u, std::thread::hardware_concurrency()))
    {
        for (int i = 0; i < m_numThreads; ++i) 
        {
            m_threads.emplace_back([this, i]() {
                threadLoop(i);
            });
        }
		std::cout << m_numThreads << " threads created for rendering." << std::endl;
    }
    Renderer::~Renderer()
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stopThreads = true;
        }
        m_cvStartThreads.notify_all();
        for (auto& t : m_threads) {
            if (t.joinable()) t.join();
        }
    }
    void Renderer::renderFrame()
    {
        glDrawPixels( m_frameBuffer.getWidth(), m_frameBuffer.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, m_frameBuffer.getPixels() );
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawPixels( m_frameBuffer.getWidth(), m_frameBuffer.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, m_frameBuffer.getUiPixels() );
        glDisable(GL_BLEND);
    }

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

    void Renderer::renderObjects(std::vector<renderObject>& objects, Camera& cam)
    {
        int width = m_frameBuffer.getWidth();
        int height = m_frameBuffer.getHeight();
        m_frameBuffer.clearZBuffer();

        Light light{ simd::Vec3f{ 0.57735f, 0.57735f, 0.57735f }, {1.f, 1.f, 1.f}, ambiantLightIntensity };
        //simd::Vec3f V{ cam.pos.getX(), cam.pos.getY(), cam.pos.getZ() };
        //V.Normalize();

        m_trianglesToRender.clear();

        simd::Matrix4x4 VP{ cam.perspective() * cam.look_at() };

        for (const auto& obj : objects) {
            const Material& mat = m_materialLibrary.get(obj.materialId);
            simd::Matrix4x4 MVP{ VP * obj.modelMatrix };

            std::vector<simd::Vec4> clipCoord{};
            std::vector<simd::Vec3f> worldNormals{};

            for (const auto& vertex : obj.vertices) {
                simd::Vec4 clip{ MVP * vertex.pos };
                clipCoord.push_back(clip);
                worldNormals.push_back(obj.modelMatrix.TransformDirection(vertex.normal).Normalize());
            }

            for (size_t f = 0; f < obj.faces.size(); f++) {
                Vertex v0{}, v1{}, v2{};
                v0.pos = clipCoord[obj.faces[f][0]];
                v1.pos = clipCoord[obj.faces[f][1]];
                v2.pos = clipCoord[obj.faces[f][2]];

                int ta = obj.face_uvs[f * 3 + 0];
                int tb = obj.face_uvs[f * 3 + 1];
                int tc = obj.face_uvs[f * 3 + 2];
                v0.texCoord = obj.uvs[ta];
                v1.texCoord = obj.uvs[tb];
                v2.texCoord = obj.uvs[tc];

                v0.normal = worldNormals[obj.faces[f][0]];
                v1.normal = worldNormals[obj.faces[f][1]];
                v2.normal = worldNormals[obj.faces[f][2]];

                std::vector<Vertex> clippedVertices{};
                clipTriangle({ v0, v1, v2 }, clippedVertices);

                for (size_t i = 0; i < clippedVertices.size() / 3; i++) {
                    ScreenVertex sv0 = projectVertex(clippedVertices[i * 3 + 0], width, height);
                    ScreenVertex sv1 = projectVertex(clippedVertices[i * 3 + 1], width, height);
                    ScreenVertex sv2 = projectVertex(clippedVertices[i * 3 + 2], width, height);

                    if (barycentricWeights(sv0.x, sv0.y, sv1.x, sv1.y, sv2.x, sv2.y) > 0) {
                        m_trianglesToRender.push_back({ {sv0, sv1, sv2}, &mat });
                    }
                }
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_activeThreads = m_numThreads;
            m_frameCounter++;
        }
        m_cvStartThreads.notify_all();

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cvThreadsDone.wait(lock, [this]() {
                return m_activeThreads == 0;
                });
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

    inline float pow32(float x) {
        float x2 = x * x;
        float x4 = x2 * x2;
		float x8 = x4 * x4;
		float x16 = x8 * x8;
        return x16 * x16; // x^32
	}

    void Renderer::drawTrianglePhong(
        int x0, int y0, float z0, float iw0, std::pair<float, float> uv0, simd::Vec3f n0,
        int x1, int y1, float z1, float iw1, std::pair<float, float> uv1, simd::Vec3f n1,
        int x2, int y2, float z2, float iw2, std::pair<float, float> uv2, simd::Vec3f n2,
        const Material& mat, const Light& light, simd::Vec3f V)
    {
        int area = barycentricWeights(x0, y0, x1, y1, x2, y2);
        if (area <= 0) return;

        int minX = std::max(0, std::min({ x0, x1, x2 }));
        int maxX = std::min(m_frameBuffer.getWidth() - 1, std::max({ x0, x1, x2 }));
        int minY = std::max(0, std::min({ y0, y1, y2 }));
        int maxY = std::min(m_frameBuffer.getHeight() - 1, std::max({ y0, y1, y2 }));

        float invArea = 1.f / static_cast<float>(area);
        simd::Vec3f H = (light.dir + V).Normalize();

        float u0w = uv0.first * iw0, v0w = uv0.second * iw0;
        float u1w = uv1.first * iw1, v1w = uv1.second * iw1;
        float u2w = uv2.first * iw2, v2w = uv2.second * iw2;
		simd::Vec3f n0w = n0 * iw0;
		simd::Vec3f n1w = n1 * iw1;
		simd::Vec3f n2w = n2 * iw2;

        int A0 = y1 - y2, B0 = x2 - x1;
        int A1 = y2 - y0, B1 = x0 - x2;
        int A2 = y0 - y1, B2 = x1 - x0;

        float dzdx = (A0 * z0 + A1 * z1 + A2 * z2) * invArea;
        float dzdy = (B0 * z0 + B1 * z1 + B2 * z2) * invArea;

        float duwdx = (A0 * u0w + A1 * u1w + A2 * u2w) * invArea;
        float duwdy = (B0 * u0w + B1 * u1w + B2 * u2w) * invArea;

        float dvwdx = (A0 * v0w + A1 * v1w + A2 * v2w) * invArea;
        float dvwdy = (B0 * v0w + B1 * v1w + B2 * v2w) * invArea;

        simd::Vec3f dnwdx = (A0 * n0w + A1 * n1w + A2 * n2w) * invArea;
        simd::Vec3f dnwdy = (B0 * n0w + B1 * n1w + B2 * n2w) * invArea;

        float diwdx = (A0 * iw0 + A1 * iw1 + A2 * iw2) * invArea;
        float diwdy = (B0 * iw0 + B1 * iw1 + B2 * iw2) * invArea;

        int start_e0 = barycentricWeights(x1, y1, x2, y2, minX, minY);
        int start_e1 = barycentricWeights(x2, y2, x0, y0, minX, minY);
        int start_e2 = barycentricWeights(x0, y0, x1, y1, minX, minY);

        float init_e0 = start_e0 * invArea;
        float init_e1 = start_e1 * invArea;
        float init_e2 = start_e2 * invArea;

        float row_z = init_e0 * z0 + init_e1 * z1 + init_e2 * z2;
        float row_uw = init_e0 * u0w + init_e1 * u1w + init_e2 * u2w;
        float row_vw = init_e0 * v0w + init_e1 * v1w + init_e2 * v2w;
        float row_iw = init_e0 * iw0 + init_e1 * iw1 + init_e2 * iw2;
        simd::Vec3f row_nw = init_e0 * n0w + init_e1 * n1w + init_e2 * n2w;

        for (int y = minY; y <= maxY; y++)
        {
            int e0 = start_e0;
            int e1 = start_e1;
            int e2 = start_e2;

            float z = row_z;
            float uw = row_uw;
            float vw = row_vw;
            float iw = row_iw;
            simd::Vec3f nw = row_nw;

            bool inside = false;

            for (int x = minX; x <= maxX; x++)
            {

                if ((e0 | e1 | e2) < 0)
                {
                    if (inside) break; // If we were inside the triangle and now we're outside, we can break early
                    e0 += A0; e1 += A1; e2 += A2;
                    z += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx;
					nw += dnwdx;
                    continue;
                }
                inside = true; // We are inside the triangle
                int idx = y * m_frameBuffer.getWidth() + x;

                if (z >= m_frameBuffer.getZBuffer()[idx])
                {
                    e0 += A0; e1 += A1; e2 += A2;
                    z += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx;
					nw += dnwdx;
                    continue;
                }
                float u = uw / iw;
                float v = vw / iw;

				simd::Vec3f n = nw / iw;

                if (std::isnan(u) || std::isnan(v) || std::isnan(n.getX()) || std::isnan(n.getY()) || std::isnan(n.getZ()) ||
                    std::isinf(u) || std::isinf(v) || std::isinf(n.getX()) || std::isinf(n.getY()) || std::isinf(n.getZ()))
                {
                    // If we hit a NaN, color this specific pixel bright magenta/blue as a warning!
                    m_frameBuffer.getZBuffer()[idx] = z;
                    m_frameBuffer.setPixel(x, y, 0xFFFF00FF); // Magenta is great for spotting math bugs
                    e0 += A0; e1 += A1; e2 += A2;
                    z += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx; nw += dnwdx;
                    continue;
                }

                n = n.Normalize();
                float ndotl = std::max(0.f, n.Dot(light.dir));
                float ndoth = std::clamp(n.Dot(H), 0.f, 1.f);
                float spec  = (ndotl > 0.f) ? mat.specLut[(int)(ndoth * 255.f)] : 0.f;
                float k     = light.ambient + (1.f - light.ambient) * ndotl;

                float tr = 1.f, tg = 1.f, tb = 1.f;
                if (mat.diffuseMap) {
                    uint32_t c = TextureLoader::sampleTextureBilinear(*mat.diffuseMap, u, v);
                    tr = (c & 0xFF) / 255.f;
                    tg = ((c >> 8) & 0xFF) / 255.f;
                    tb = ((c >> 16) & 0xFF) / 255.f;
                }

                float r = mat.diffuse.r * tr * k + mat.specular.r * spec + mat.emissive.r;
                float g = mat.diffuse.g * tg * k + mat.specular.g * spec + mat.emissive.g;
                float b = mat.diffuse.b * tb * k + mat.specular.b * spec + mat.emissive.b;

                auto to8 = [](float x) { return (uint32_t)(std::clamp(x, 0.f, 1.f) * 255.f + 0.5f); };

                m_frameBuffer.getZBuffer()[idx] = z;
                m_frameBuffer.setPixel(x, y, 0xFF000000 | (to8(b) << 16) | (to8(g) << 8) | to8(r));

                e0 += A0; e1 += A1; e2 += A2;
                z += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx;
				nw += dnwdx;
            }

            start_e0 += B0;
            start_e1 += B1;
            start_e2 += B2;

            row_z += dzdy;
            row_uw += duwdy;
            row_vw += dvwdy;
            row_iw += diwdy;
			row_nw += dnwdy;
        }
    }

    void Renderer::drawTrianglePhongInTile(
        const ScreenVertex& v0, const ScreenVertex& v1, const ScreenVertex& v2,
        const Material& mat, const Light& light,
        int tileMinY, int tileMaxY)
    {
        int x0 = v0.x, y0 = v0.y;
        int x1 = v1.x, y1 = v1.y;
        int x2 = v2.x, y2 = v2.y;

        int area = barycentricWeights(x0, y0, x1, y1, x2, y2);
        if (area <= 0) return;

        int minX = std::max(0, std::min({ x0, x1, x2 }));
        int maxX = std::min(m_frameBuffer.getWidth() - 1, std::max({ x0, x1, x2 }));
        int minY = std::max(tileMinY, std::min({ y0, y1, y2 }));
        int maxY = std::min(tileMaxY, std::max({ y0, y1, y2 }));

        if (minX > maxX || minY > maxY) return;

        float z0 = v0.z, z1 = v1.z, z2 = v2.z;
        float iw0 = v0.invW, iw1 = v1.invW, iw2 = v2.invW;

        float invArea = 1.f / static_cast<float>(area);
        simd::Vec3f H = (light.dir + simd::Vec3f{ 1.f, 1.f, 1.f }).Normalize(); // TODO change to use cam pos

        float u0w = v0.uv.first * iw0, v0w = v0.uv.second * iw0;
        float u1w = v1.uv.first * iw1, v1w = v1.uv.second * iw1;
        float u2w = v2.uv.first * iw2, v2w = v2.uv.second * iw2;

        simd::Vec3f n0w = v0.normal * iw0;
        simd::Vec3f n1w = v1.normal * iw1;
        simd::Vec3f n2w = v2.normal * iw2;

        int A0 = y1 - y2, B0 = x2 - x1;
        int A1 = y2 - y0, B1 = x0 - x2;
        int A2 = y0 - y1, B2 = x1 - x0;

        float dzdx = (A0 * z0 + A1 * z1 + A2 * z2) * invArea;
        float dzdy = (B0 * z0 + B1 * z1 + B2 * z2) * invArea;

        float duwdx = (A0 * u0w + A1 * u1w + A2 * u2w) * invArea;
        float duwdy = (B0 * u0w + B1 * u1w + B2 * u2w) * invArea;

        float dvwdx = (A0 * v0w + A1 * v1w + A2 * v2w) * invArea;
        float dvwdy = (B0 * v0w + B1 * v1w + B2 * v2w) * invArea;

        simd::Vec3f dnwdx = (A0 * n0w + A1 * n1w + A2 * n2w) * invArea;
        simd::Vec3f dnwdy = (B0 * n0w + B1 * n1w + B2 * n2w) * invArea;

        float diwdx = (A0 * iw0 + A1 * iw1 + A2 * iw2) * invArea;
        float diwdy = (B0 * iw0 + B1 * iw1 + B2 * iw2) * invArea;

        // Start evaluation from minX, minY of the tile/triangle intersection
        int start_e0 = barycentricWeights(x1, y1, x2, y2, minX, minY);
        int start_e1 = barycentricWeights(x2, y2, x0, y0, minX, minY);
        int start_e2 = barycentricWeights(x0, y0, x1, y1, minX, minY);

        float init_e0 = start_e0 * invArea;
        float init_e1 = start_e1 * invArea;
        float init_e2 = start_e2 * invArea;

        float row_z = init_e0 * z0 + init_e1 * z1 + init_e2 * z2;
        float row_uw = init_e0 * u0w + init_e1 * u1w + init_e2 * u2w;
        float row_vw = init_e0 * v0w + init_e1 * v1w + init_e2 * v2w;
        float row_iw = init_e0 * iw0 + init_e1 * iw1 + init_e2 * iw2;
        simd::Vec3f row_nw = init_e0 * n0w + init_e1 * n1w + init_e2 * n2w;

        for (int y = minY; y <= maxY; y++)
        {
            int e0 = start_e0;
            int e1 = start_e1;
            int e2 = start_e2;

            float z = row_z;
            float uw = row_uw;
            float vw = row_vw;
            float iw = row_iw;
            simd::Vec3f nw = row_nw;

            bool inside = false;

            for (int x = minX; x <= maxX; x++)
            {
                if ((e0 | e1 | e2) < 0)
                {
                    if (inside) break;
                    e0 += A0; e1 += A1; e2 += A2;
                    z += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx; nw += dnwdx;
                    continue;
                }
                inside = true;
                int idx = y * m_frameBuffer.getWidth() + x;

                if (z >= m_frameBuffer.getZBuffer()[idx])
                {
                    e0 += A0; e1 += A1; e2 += A2;
                    z += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx; nw += dnwdx;
                    continue;
                }

                float u = uw / iw;
                float v = vw / iw;
                simd::Vec3f n = (nw / iw).Normalize();

                float ndotl = std::max(0.f, n.Dot(light.dir));
                float ndoth = std::clamp(n.Dot(H), 0.f, 1.f);
                float spec = (ndotl > 0.f) ? mat.specLut[(int)(ndoth * 255.f)] : 0.f;
                float k = light.ambient + (1.f - light.ambient) * ndotl;

                float tr = 1.f, tg = 1.f, tb = 1.f;
                if (mat.diffuseMap) {
                    uint32_t c = TextureLoader::sampleTextureBilinear(*mat.diffuseMap, u, v);
                    tr = (c & 0xFF) / 255.f;
                    tg = ((c >> 8) & 0xFF) / 255.f;
                    tb = ((c >> 16) & 0xFF) / 255.f;
                }

                float r = mat.diffuse.r * tr * k + mat.specular.r * spec + mat.emissive.r;
                float g = mat.diffuse.g * tg * k + mat.specular.g * spec + mat.emissive.g;
                float b = mat.diffuse.b * tb * k + mat.specular.b * spec + mat.emissive.b;

                auto to8 = [](float val) { return (uint32_t)(std::clamp(val, 0.f, 1.f) * 255.f + 0.5f); };

                m_frameBuffer.getZBuffer()[idx] = z;
                m_frameBuffer.setPixel(x, y, 0xFF000000 | (to8(b) << 16) | (to8(g) << 8) | to8(r));

                e0 += A0; e1 += A1; e2 += A2;
                z += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx; nw += dnwdx;
            }

            start_e0 += B0; start_e1 += B1; start_e2 += B2;
            row_z += dzdy; row_uw += duwdy; row_vw += dvwdy; row_iw += diwdy; row_nw += dnwdy;
        }
    }

    uint32_t Renderer::color_phong(uint32_t color, float diffuse, float specular)
    {
        // AABBGGRR
        uint32_t a = (color >> 24) & 0xFF;

        float fb = ((color >> 16) & 0xFF) * diffuse + 255.f * specular;
        float fg = ((color >> 8) & 0xFF) * diffuse + 255.f * specular;
        float fr = (color & 0xFF) * diffuse + 255.f * specular;

        uint8_t b = static_cast<uint8_t>(std::clamp(fb, 0.0f, 255.0f));
        uint8_t g = static_cast<uint8_t>(std::clamp(fg, 0.0f, 255.0f));
        uint8_t r = static_cast<uint8_t>(std::clamp(fr, 0.0f, 255.0f));

        return (a << 24) | (b << 16) | (g << 8) | r;
    }

    inline Vertex clipLerp(Vertex v0, Vertex v1, float t) {
        Vertex r{};
        r.pos = v0.pos + t * (v1.pos - v0.pos);
        r.texCoord.first = v0.texCoord.first + t * (v1.texCoord.first - v0.texCoord.first);
        r.texCoord.second = v0.texCoord.second + t * (v1.texCoord.second - v0.texCoord.second);
        r.normal = v0.normal + t * (v1.normal - v0.normal);

        return r;
    }

    void Renderer::clipTriangle(const std::vector<Vertex>& in, std::vector<Vertex>& out)
    {
        std::vector<Vertex> poly{};
        int n{};

        for (int i = 0; i < 3; i++)
        {
            Vertex a = in[i];
            Vertex b = in[(i + 1) % 3];
            float da = a.pos.getZ() + a.pos.getW();
            float db = b.pos.getZ() + b.pos.getW();
            bool a_in = da >= 0.0f;
            bool b_in = db >= 0.0f;

            if (a_in && b_in)
            {
                poly.push_back(b);
                n++;
            }
            else if (a_in && !b_in)
            {
                float t = da / (da - db);
                poly.push_back(clipLerp(a, b, t));
                n++;
            }
            else if (!a_in && b_in)
            {
                float t = da / (da - db);
                poly.push_back(clipLerp(a, b, t));
                poly.push_back(b);
                n += 2;
            }

            if (n == 3) 
            {
                out.push_back(poly[0]);
                out.push_back(poly[1]);
                out.push_back(poly[2]);
            }
            else if (n == 4)
            {
                out.push_back(poly[0]);
                out.push_back(poly[1]);
                out.push_back(poly[2]);

                out.push_back(poly[0]);
                out.push_back(poly[2]);
                out.push_back(poly[3]);
            }
        }
        if (poly.size() >= 3) 
        {
            out.push_back(poly[0]); out.push_back(poly[1]); out.push_back(poly[2]);
        }
        if (poly.size() == 4) 
        {
            out.push_back(poly[0]); out.push_back(poly[2]); out.push_back(poly[3]);
        }
    }

    void Renderer::projectAndDraw(const std::vector<Vertex>& triangle, const Material& mat, const Light& light, simd::Vec3f V)
    {
        std::array<int, 3> screen_x{};
        std::array<int, 3> screen_y{};
        std::array<float, 3> ndc_z{};
        std::array<float, 3> inv_w{};

        for (size_t i = 0; i < 3; i++)
        {
            float inv = 1.0f / triangle[i].pos.getW();
            screen_x[i] = (int)((triangle[i].pos.getX() * inv + 1.0f) * 0.5f * m_frameBuffer.getWidth());
            screen_y[i] = (int)((1.0f - (triangle[i].pos.getY() * inv + 1.0f) * 0.5f) * m_frameBuffer.getHeight());
            ndc_z[i] = triangle[i].pos.getZ() * inv;
            inv_w[i] = inv;
        }

        if (barycentricWeights(screen_x[0], screen_y[0], screen_x[1], screen_y[1], screen_x[2], screen_y[2]) <= 0) return;

        drawTrianglePhong(
            screen_x[0], screen_y[0], ndc_z[0], inv_w[0], triangle[0].texCoord, triangle[0].normal,
            screen_x[1], screen_y[1], ndc_z[1], inv_w[1], triangle[1].texCoord, triangle[1].normal,
            screen_x[2], screen_y[2], ndc_z[2], inv_w[2], triangle[2].texCoord, triangle[2].normal,
            mat, light, V
        );

        //for (size_t f = 0; f < obj.first.faces.size(); f++) {
        //    const auto& face = obj.first.faces[f];
        //    int a = face[0];
        //    int b = face[1];
        //    int c = face[2];
        //    //if (!visible[face[0]] || !visible[face[1]] || !visible[face[2]]) continue;
        //    if (barycentricWeights(screen_x[a], screen_y[a], screen_x[b], screen_y[b], screen_x[c], screen_y[c]) <= 0) continue;

        //    simd::Vec3f p0{ obj.first.vertices[a].pos.getX(), obj.first.vertices[a].pos.getY(), obj.first.vertices[a].pos.getZ() };
        //    simd::Vec3f p1{ obj.first.vertices[b].pos.getX(), obj.first.vertices[b].pos.getY(), obj.first.vertices[b].pos.getZ() };
        //    simd::Vec3f p2{ obj.first.vertices[c].pos.getX(), obj.first.vertices[c].pos.getY(), obj.first.vertices[c].pos.getZ() };

        //    simd::Vec3f edge1 = p1 - p0;
        //    simd::Vec3f edge2 = p2 - p0;
        //    simd::Vec3f n = obj.first.modelMatrix.TransformDirection(edge1.Cross(edge2)).Normalize();
        //    float ndotl = n.Dot(lightDir);
        //    if (ndotl < 0.0f) ndotl = 0.0f;
        //    float brightness = ambiantLightIntensity + (1.0f - ambiantLightIntensity) * ndotl;

        //    int ta = obj.first.face_uvs[f * 3 + 0];
        //    int tb = obj.first.face_uvs[f * 3 + 1];
        //    int tc = obj.first.face_uvs[f * 3 + 2];
        //    std::pair<float, float> uva = obj.first.uvs[ta];
        //    std::pair<float, float> uvb = obj.first.uvs[tb];
        //    std::pair<float, float> uvc = obj.first.uvs[tc];

        //    drawTrianglePhong(
        //        screen_x[a], screen_y[a], ndc_z[a], ndc_w[a], uva, worldNormals[a],
        //        screen_x[b], screen_y[b], ndc_z[b], ndc_w[b], uvb, worldNormals[b],
        //        screen_x[c], screen_y[c], ndc_z[c], ndc_w[c], uvc, worldNormals[c],
        //        obj.second, lightDir, halfVector
        //    );
        //}
    }

    void Renderer::appendParticlesToRender(const ParticlePool& particlePool, Camera& cam)
    {
        for (const auto& p : particlePool.getParticles())
        {

        }
    }

    void Renderer::threadLoop(int threadId)
    {
		uint64_t lastFrame = 0;
        while (true) {
            std::unique_lock<std::mutex> lock(m_mutex);

            m_cvStartThreads.wait(lock, [this, lastFrame]() {
                return m_stopThreads || m_frameCounter > lastFrame;
                });

            if (m_stopThreads) break;

            lastFrame = m_frameCounter;

            lock.unlock();

            renderTile(threadId);

            lock.lock();
            m_activeThreads--;
            if (m_activeThreads == 0) 
                {
                m_cvThreadsDone.notify_one();
			}
        }
    }

    void Renderer::renderTile(int threadId)
    {
		int height = m_frameBuffer.getHeight();
		int sliceHeight = height / m_numThreads;
		int startY = threadId * sliceHeight;
		int endY = (threadId == m_numThreads - 1) ? height - 1 : startY + sliceHeight - 1;

        for (const auto& tri : m_trianglesToRender)
        {
            drawTrianglePhongInTile(
                tri.vertices[0], tri.vertices[1], tri.vertices[2],
                *tri.material, m_light,
                startY, endY
            );
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

    ScreenVertex Renderer::projectVertex(const Vertex& v, int width, int height) {
        float inv = 1.0f / v.pos.getW();
        int sx = static_cast<int>((v.pos.getX() * inv + 1.0f) * 0.5f * width);
        int sy = static_cast<int>((1.0f - (v.pos.getY() * inv + 1.0f) * 0.5f) * height);
        float sz = v.pos.getZ() * inv;
        return { sx, sy, sz, inv, v.texCoord, v.normal };
    }
}