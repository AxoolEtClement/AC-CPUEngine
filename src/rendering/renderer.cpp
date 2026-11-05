#include "rendering/renderer.hpp"
#include <algorithm>

static uint32_t blendOver(uint32_t src, uint32_t dst)   // formats AABBGGRR, alpha droit
{
    float sa = ((src >> 24) & 0xFF) / 255.f;
    float da = ((dst >> 24) & 0xFF) / 255.f;
    float oa = sa + da * (1.f - sa);
    if (oa <= 0.f) return 0;

    auto ch = [&](int shift) -> uint32_t {
        float s = float((src >> shift) & 0xFF);
        float d = float((dst >> shift) & 0xFF);
        return (uint32_t)((s * sa + d * da * (1.f - sa)) / oa + 0.5f);
    };
    return ((uint32_t)(oa * 255.f + 0.5f) << 24) | (ch(16) << 16) | (ch(8) << 8) | ch(0);
}

namespace AC_Engine
{
    Renderer::Renderer(FrameBuffer& frameBuffer, MaterialLibrary& materialLibrary, MeshManager& meshManager)
        : m_frameBuffer(frameBuffer), m_materialLibrary(materialLibrary), 
        m_numThreads(std::max(2u, std::thread::hardware_concurrency())), m_meshManager(meshManager)
    {
        for (int i = 0; i < m_numThreads; ++i) 
        {
            m_threads.emplace_back([this, i]() {
                threadLoop(i);
            });
        }
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
        glDrawPixels( m_frameBuffer.getWidth(), m_frameBuffer.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, m_frameBuffer.getTransparentPixels() );
        glDrawPixels( m_frameBuffer.getWidth() / 4, m_frameBuffer.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, m_frameBuffer.getUiPixels() );
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

    void Renderer::renderObjectsWireframe(ECS::Registry& registry, unsigned int cameraEntityID)
    {
        int width = m_frameBuffer.getWidth();
        int height = m_frameBuffer.getHeight();

		CameraComponent& cam{ registry.getComponent<CameraComponent>(cameraEntityID) };

        simd::Matrix4x4 VP{ cam.projection() * cam.view() };
		for (const auto& [mesh, transform] : registry.view<MeshComponent, TransformComponent>())
        {
			auto& obj = *mesh.mesh;
            std::vector<simd::Vec3f> verticesNdc;
            std::vector<bool> visible;
            simd::Matrix4x4 MVP{ VP * transform.getTransformMatrix() };
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

    inline simd::Vec3f computeHalfVector(const simd::Vec3f& lightDir, const simd::Vec3f& viewDir)
    {
        return (lightDir + viewDir).Normalize();
	}

    void Renderer::renderObjects(ECS::Registry& registry, unsigned int cameraEntityID)
    {
        int width = m_frameBuffer.getWidth();
        int height = m_frameBuffer.getHeight();
        m_frameBuffer.clearZBuffer();

        CameraComponent& cam{ registry.getComponent<CameraComponent>(cameraEntityID) };
        cam.aspect = static_cast<float>(width) / static_cast<float>(height);
        simd::Matrix4x4 VP{ cam.projection() * cam.view() };
        
		simd::Vec3f camPos = cam.pos;

        // Les objets transparents sont mis de cote (avec leur distance a la camera)
        // pour etre dessines apres les opaques, du plus loin au plus proche
        std::vector<std::pair<float, decltype(m_trianglesToRender)>> transparentObjects;

		// Iterate over all entities with MeshComponent and TransformComponent
        for (const auto& [mesh, transform] : registry.view<MeshComponent, TransformComponent>())
        {
            auto& obj = *mesh.mesh;
            const Material& mat = m_materialLibrary.get(obj.materialId);
            simd::Matrix4x4 MVP{ VP * transform.getTransformMatrix() };

            const bool isTransparent = mat.opacity < 1.0f;
            decltype(m_trianglesToRender) objectTriangles;
            auto& targetTriangles = isTransparent ? objectTriangles : m_trianglesToRender;

            std::vector<simd::Vec4> clipCoord{};
            std::vector<simd::Vec3f> worldNormals{};
			std::vector<simd::Vec3f> worldPositions{};
            
            // project and clip vertices
            for (const auto& vertex : obj.vertices) 
            {
                simd::Vec4 clip{ MVP * vertex.pos };
                clipCoord.push_back(clip);

				simd::Vec4 tempWorldPos = transform.getTransformMatrix() * vertex.pos;
                simd::Vec3f worldPos{ tempWorldPos.getX(), tempWorldPos.getY(), tempWorldPos.getZ() };
				worldPositions.push_back(worldPos);

                simd::Vec3f transformedNormal = transform.getTransformMatrix().TransformDirection(vertex.normal);
				float lengthSq = transformedNormal.MagnitudeSquared();

                if (lengthSq > 0.0001f) {
                    worldNormals.push_back(transformedNormal.Normalize());
                } else {
                    worldNormals.push_back(simd::Vec3f{ 0.0f, 1.0f, 0.0f }); 
                }
            }

			// Iterate over faces and prepare triangles for rendering
            for (size_t f = 0; f < obj.faces.size(); f++) {
                Vertex v0{}, v1{}, v2{};
                v0.pos = clipCoord[obj.faces[f][0]];
                v1.pos = clipCoord[obj.faces[f][1]];
                v2.pos = clipCoord[obj.faces[f][2]];

                simd::Vec3f wp0 = worldPositions[obj.faces[f][0]];
                simd::Vec3f wp1 = worldPositions[obj.faces[f][1]];
                simd::Vec3f wp2 = worldPositions[obj.faces[f][2]];

				simd::Vec3f h0 = computeHalfVector(m_light.dir, (camPos - wp0).Normalize());
				simd::Vec3f h1 = computeHalfVector(m_light.dir, (camPos - wp1).Normalize());
				simd::Vec3f h2 = computeHalfVector(m_light.dir, (camPos - wp2).Normalize());

                if (obj.face_uvs.size() >= (f + 1) * 3) {
                    v0.texCoord = obj.uvs[obj.face_uvs[f * 3 + 0]];
                    v1.texCoord = obj.uvs[obj.face_uvs[f * 3 + 1]];
                    v2.texCoord = obj.uvs[obj.face_uvs[f * 3 + 2]];
                } else {
                    v0.texCoord = v1.texCoord = v2.texCoord = { 0.f, 0.f };
                }   

                v0.normal = worldNormals[obj.faces[f][0]];
                v1.normal = worldNormals[obj.faces[f][1]];
                v2.normal = worldNormals[obj.faces[f][2]];

                std::vector<Vertex> clippedVertices{};
				std::vector<simd::Vec3f> clippedH{};
                clipTriangle({ v0, v1, v2 }, { h0, h1, h2 }, clippedVertices, clippedH);

                for (size_t i = 0; i < clippedVertices.size() / 3; i++) {
                    ScreenVertex sv0 = projectVertex(clippedVertices[i * 3 + 0], width, height);
                    ScreenVertex sv1 = projectVertex(clippedVertices[i * 3 + 1], width, height);
                    ScreenVertex sv2 = projectVertex(clippedVertices[i * 3 + 2], width, height);

					sv0.halfVector = clippedH[i * 3 + 0];
					sv1.halfVector = clippedH[i * 3 + 1];
					sv2.halfVector = clippedH[i * 3 + 2];

                    if (barycentricWeights(sv0.x, sv0.y, sv1.x, sv1.y, sv2.x, sv2.y) > 0) {
                        targetTriangles.push_back({ {sv0, sv1, sv2}, &mat });
                    }
                }
            }

            if (isTransparent)
            {
                simd::Vec3f center = transform.getTransformMatrix().TransformPoint(simd::Vec3f{ 0.f, 0.f, 0.f });
                transparentObjects.emplace_back(camPos.DistanceSquared(center), std::move(objectTriangles));
            }
        }

        // Transparent objects: farthest first, appended after all opaque triangles
        std::sort(transparentObjects.begin(), transparentObjects.end(),
            [](const auto& a, const auto& b) { return a.first > b.first; });
        for (auto& entry : transparentObjects)
        {
            m_trianglesToRender.insert(m_trianglesToRender.end(), entry.second.begin(), entry.second.end());
        }

		// Notify threads to start rendering
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_activeThreads = m_numThreads;
            m_frameCounter++;
        }
        m_cvStartThreads.notify_all();
		// Wait for all threads to finish rendering
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cvThreadsDone.wait(lock, [this]() {
                return m_activeThreads == 0;
                });
        }

        // Clear the triangles to render for the next frame
        m_trianglesToRender.clear();
    }

    float Renderer::barycentricWeights(int x0, int y0, int x1, int y1, int x2, int y2)
    {
        return (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
    }

    inline float pow32(float x) {
        float x2 = x * x;
        float x4 = x2 * x2;
		float x8 = x4 * x4;
		float x16 = x8 * x8;
        return x16 * x16; // x^32
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

		// initial values for interpolation
        float z0 = v0.z, z1 = v1.z, z2 = v2.z;
        float iw0 = v0.invW, iw1 = v1.invW, iw2 = v2.invW;

        float invArea = 1.f / static_cast<float>(area);

        float u0w = v0.uv.first * iw0, v0w = v0.uv.second * iw0;
        float u1w = v1.uv.first * iw1, v1w = v1.uv.second * iw1;
        float u2w = v2.uv.first * iw2, v2w = v2.uv.second * iw2;

        simd::Vec3f n0w = v0.normal * iw0;
        simd::Vec3f n1w = v1.normal * iw1;
        simd::Vec3f n2w = v2.normal * iw2;

		simd::Vec3f h0w = v0.halfVector * iw0;
		simd::Vec3f h1w = v1.halfVector * iw1;
		simd::Vec3f h2w = v2.halfVector * iw2;

        int A0 = y1 - y2, B0 = x2 - x1;
        int A1 = y2 - y0, B1 = x0 - x2;
        int A2 = y0 - y1, B2 = x1 - x0;

        // Top-left fill rule: avoids drawing shared edges twice (double blending on transparent triangles)
        const int bias0 = (A0 > 0 || (A0 == 0 && B0 > 0)) ? 0 : -1;
        const int bias1 = (A1 > 0 || (A1 == 0 && B1 > 0)) ? 0 : -1;
        const int bias2 = (A2 > 0 || (A2 == 0 && B2 > 0)) ? 0 : -1;

		// deltas for interpolation
        float dzdx = (A0 * z0 + A1 * z1 + A2 * z2) * invArea;
        float dzdy = (B0 * z0 + B1 * z1 + B2 * z2) * invArea;

        float duwdx = (A0 * u0w + A1 * u1w + A2 * u2w) * invArea;
        float duwdy = (B0 * u0w + B1 * u1w + B2 * u2w) * invArea;

        float dvwdx = (A0 * v0w + A1 * v1w + A2 * v2w) * invArea;
        float dvwdy = (B0 * v0w + B1 * v1w + B2 * v2w) * invArea;

        simd::Vec3f dnwdx = (A0 * n0w + A1 * n1w + A2 * n2w) * invArea;
        simd::Vec3f dnwdy = (B0 * n0w + B1 * n1w + B2 * n2w) * invArea;

		simd::Vec3f dhwdx = (A0 * h0w + A1 * h1w + A2 * h2w) * invArea;
		simd::Vec3f dhwdy = (B0 * h0w + B1 * h1w + B2 * h2w) * invArea;

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
		simd::Vec3f row_hw = init_e0 * h0w + init_e1 * h1w + init_e2 * h2w;

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
			simd::Vec3f hw = row_hw;

            bool inside = false;

            for (int x = minX; x <= maxX; x++)
            {
                if (((e0 + bias0) | (e1 + bias1) | (e2 + bias2)) < 0)
                {
                    if (inside) break;
                    e0 += A0; e1 += A1; e2 += A2;
                    z += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx; nw += dnwdx;
					hw += dhwdx;
                    continue;
                }
                inside = true;
                int idx = y * m_frameBuffer.getWidth() + x;

                if (z >= m_frameBuffer.getZBuffer()[idx])
                {
                    e0 += A0; e1 += A1; e2 += A2;
                    z += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx; nw += dnwdx;
					hw += dhwdx;
                    continue;
                }

                float u = uw / iw;
                float v = vw / iw;
                simd::Vec3f n = (nw / iw).Normalize();
				simd::Vec3f H = (hw / iw).Normalize();

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

                uint32_t out = (to8(b) << 16) | (to8(g) << 8) | to8(r);

                m_frameBuffer.getZBuffer()[idx] = z;

                if (mat.opacity < 1.0f)
                {
                    uint32_t src = ((uint32_t)(mat.opacity * 255.f + 0.5f) << 24) | out;
                    uint32_t dst = m_frameBuffer.getTransparentPixels()[idx];
                    m_frameBuffer.setTransparentPixel(x, y, blendOver(src, dst));
                }
                else
                {
                    m_frameBuffer.setPixel(x, y, 0xFF000000 | out);
                }

                e0 += A0; e1 += A1; e2 += A2;
                z += dzdx; uw += duwdx; vw += dvwdx; iw += diwdx; nw += dnwdx;
				hw += dhwdx;
            }

            start_e0 += B0; start_e1 += B1; start_e2 += B2;
            row_z += dzdy; row_uw += duwdy; row_vw += dvwdy; row_iw += diwdy; row_nw += dnwdy;
			row_hw += dhwdy;
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

    inline Vertex clipLerp(const Vertex& v0, const Vertex& v1, float t, const simd::Vec3f& h0, const simd::Vec3f& h1, simd::Vec3f& outH) {
        Vertex r{};
        r.pos = v0.pos + t * (v1.pos - v0.pos);
        r.texCoord.first = v0.texCoord.first + t * (v1.texCoord.first - v0.texCoord.first);
        r.texCoord.second = v0.texCoord.second + t * (v1.texCoord.second - v0.texCoord.second);
        r.normal = v0.normal + t * (v1.normal - v0.normal);
        
		outH = h0 + t * (h1 - h0);
        return r;
    }

    void Renderer::clipTriangle(const std::vector<Vertex>& in, const std::vector<simd::Vec3f>& inh, std::vector<Vertex>& out, std::vector<simd::Vec3f>& outH)
    {
        std::vector<Vertex> poly{};
		std::vector<simd::Vec3f> polyH{};
        int n{};

        for (int i = 0; i < 3; i++)
        {
            Vertex a = in[i];
            Vertex b = in[(i + 1) % 3];
			simd::Vec3f ha = inh[i];
			simd::Vec3f hb = inh[(i + 1) % 3];

            float da = a.pos.getZ() + a.pos.getW();
            float db = b.pos.getZ() + b.pos.getW();
            bool a_in = da >= 0.0f;
            bool b_in = db >= 0.0f;

            if (a_in && b_in)
            {
                poly.push_back(b);
				polyH.push_back(hb);
                n++;
            }
            else if (a_in && !b_in)
            {
                float t = da / (da - db);
				simd::Vec3f interpolatedH;
                poly.push_back(clipLerp(a, b, t, ha, hb, interpolatedH));
				polyH.push_back(interpolatedH);
                n++;
            }
            else if (!a_in && b_in)
            {
                float t = da / (da - db);
                simd::Vec3f interpolatedH;
                poly.push_back(clipLerp(a, b, t, ha, hb, interpolatedH));
                polyH.push_back(interpolatedH);
                poly.push_back(b);
                polyH.push_back(hb);
                n += 2;
            }
        }
        if (poly.size() >= 3) 
        {
            out.push_back(poly[0]); out.push_back(poly[1]); out.push_back(poly[2]);
            outH.push_back(polyH[0]); outH.push_back(polyH[1]); outH.push_back(polyH[2]);
        }
        if (poly.size() == 4) 
        {
            out.push_back(poly[0]); out.push_back(poly[2]); out.push_back(poly[3]);
            outH.push_back(polyH[0]); outH.push_back(polyH[2]); outH.push_back(polyH[3]);
        }
    }

    void Renderer::appendParticlesToRender(const ParticlePool& particlePool, ECS::Registry& registry, unsigned int cameraEntityID)
    {
		const CameraComponent& cam = registry.getComponent<CameraComponent>(cameraEntityID);

        simd::Vec3f camRight{ cam.forward.Cross(cam.up).Normalize() };
		simd::Vec3f camUp{ camRight.Cross(cam.forward).Normalize() };
        
        simd::Matrix4x4 VP{ cam.projection() * cam.view() };

        int width = m_frameBuffer.getWidth();
        int height = m_frameBuffer.getHeight();

		// creating billboards and separating them into triangles for rendering for each alive particle
        for (const auto& p : particlePool.getParticles()) {
            if (!p.isAlive()) continue;

            float s = p.size * 0.5f;

            simd::Vec4 p0_world{ p.position.getX() - camRight.getX() * s - camUp.getX() * s,
                                 p.position.getY() - camRight.getY() * s - camUp.getY() * s,
                                 p.position.getZ() - camRight.getZ() * s - camUp.getZ() * s, 1.0f };
            simd::Vec4 p1_world{ p.position.getX() + camRight.getX() * s - camUp.getX() * s,
                                 p.position.getY() + camRight.getY() * s - camUp.getY() * s,
                                 p.position.getZ() + camRight.getZ() * s - camUp.getZ() * s, 1.0f };
            simd::Vec4 p2_world{ p.position.getX() + camRight.getX() * s + camUp.getX() * s,
                                 p.position.getY() + camRight.getY() * s + camUp.getY() * s,
                                 p.position.getZ() + camRight.getZ() * s + camUp.getZ() * s, 1.0f };
            simd::Vec4 p3_world{ p.position.getX() - camRight.getX() * s + camUp.getX() * s,
                                 p.position.getY() - camRight.getY() * s + camUp.getY() * s,
                                 p.position.getZ() - camRight.getZ() * s + camUp.getZ() * s, 1.0f };

            simd::Vec4 c0 = VP * p0_world;
            simd::Vec4 c1 = VP * p1_world;
            simd::Vec4 c2 = VP * p2_world;
            simd::Vec4 c3 = VP * p3_world;

            auto toScreenVertex = [&](const simd::Vec4& clip, std::pair<float, float> uv) -> ScreenVertex {
                float inv = 1.0f / clip.getW();
                int sx = static_cast<int>((clip.getX() * inv + 1.0f) * 0.5f * width);
                int sy = static_cast<int>((1.0f - (clip.getY() * inv + 1.0f) * 0.5f) * height);
                float sz = clip.getZ() * inv;
                return { sx, sy, sz, inv, uv, {0.f, 0.f, 1.f}, m_light.dir };
                };

            ScreenVertex sv0 = toScreenVertex(c0, { 0.f, 0.f });
            ScreenVertex sv1 = toScreenVertex(c1, { 1.f, 0.f });
            ScreenVertex sv2 = toScreenVertex(c2, { 1.f, 1.f });
            ScreenVertex sv3 = toScreenVertex(c3, { 0.f, 1.f });

            if (barycentricWeights(sv0.x, sv0.y, sv1.x, sv1.y, sv2.x, sv2.y) > 0) {
                m_trianglesToRender.push_back({ {sv0, sv1, sv2}, p.mat });
            }
            if (barycentricWeights(sv0.x, sv0.y, sv2.x, sv2.y, sv3.x, sv3.y) > 0) {
                m_trianglesToRender.push_back({ {sv0, sv2, sv3}, p.mat });
            }
        }
    }

    void Renderer::threadLoop(int threadId)
    {
		uint64_t lastFrame = 0;
        while (true) {
            std::unique_lock<std::mutex> lock(m_mutex);

            // Wait for the main thread to signal that a new frame is ready to be rendered
            m_cvStartThreads.wait(lock, [this, lastFrame]() {
                return m_stopThreads || m_frameCounter > lastFrame;
                });

            if (m_stopThreads) break;

            lastFrame = m_frameCounter;

            lock.unlock();

            renderTile(threadId);
			// Notify the main thread that this thread has finished rendering its tile
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
		// Calculates the vertical slice of the framebuffer for this thread vefore rendering
		// could prevent doing it each frame somehow but this is to endure proper scaling if the window is resized
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

    ScreenVertex Renderer::projectVertex(const Vertex& v, int width, int height) {
        float inv = 1.0f / v.pos.getW();
        int sx = static_cast<int>((v.pos.getX() * inv + 1.0f) * 0.5f * width);
        int sy = static_cast<int>((1.0f - (v.pos.getY() * inv + 1.0f) * 0.5f) * height);
        float sz = v.pos.getZ() * inv;
        return { sx, sy, sz, inv, v.texCoord, v.normal };
    }
}