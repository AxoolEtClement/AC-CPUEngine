#pragma once
#include "frameBuffer.hpp"
#include <math.h>
#include "Mat4x4.hpp"
#include "Vec4.hpp"
#include "renderObject.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include "camera.hpp"
#include "texture.hpp"
#include "material.hpp"

namespace AC_Engine
{
    struct Light 
    {
        simd::Vec3f dir;
        Color3 color   { 1.f, 1.f, 1.f };
        float  ambient = 0.18f;
    };

    struct ScreenVertex {
        int x, y;
        float z;
        float invW;
        std::pair<float, float> uv;
        simd::Vec3f normal;
    };

    struct RenderTriangle {
        std::array<ScreenVertex, 3> vertices;
        const Material* material;
    };

    class Renderer
    {
    public:
        Renderer(FrameBuffer& frameBuffer, MaterialLibrary& materialLibrary);
        ~Renderer();

        void renderFrame();

        void drawLine(int x0, int y0, int x1, int y1, unsigned int color);
        void drawDemoRGBTriangle(simd::Vec4& v0, simd::Vec4& v1, simd::Vec4& v2, float t);
        void renderObjectsWireframe(std::vector<renderObject> renderObjects, Camera cam);
        void renderObjects(std::vector<renderObject>& renderObjects, Camera& cam);
    private:
        FrameBuffer& m_frameBuffer;
        MaterialLibrary& m_materialLibrary;
		std::vector<RenderTriangle> m_trianglesToRender;


        float barycentricWeights(int x0, int y0, int x1, int y1, int x2, int y2);
        void drawTriangle(int x0, int y0, float z0, int x1, int y1, float z1, int x2, int y2, float z2);
        void drawTriangleTexturedAndLit(int x0, int y0, float z0, float iw0, std::pair<float, float> uv0, int x1, int y1, float z1, float iw1, std::pair<float, float> uv1, int x2, int y2, float z2, float iw2, std::pair<float, float> uv2, const Texture& texture, float brightness);
        void drawTrianglePhong(
            int x0, int y0, float z0, float iw0, std::pair<float, float> uv0, simd::Vec3f n0,
			int x1, int y1, float z1, float iw1, std::pair<float, float> uv1, simd::Vec3f n1,
			int x2, int y2, float z2, float iw2, std::pair<float, float> uv2, simd::Vec3f n2,
            const Material& mat, const Light& light, simd::Vec3f V);

        void drawTrianglePhongInTile(
            const ScreenVertex& v0, const ScreenVertex& v1, const ScreenVertex& v2,
            const Material& mat, const Light& light,
            int tileMinY, int tileMaxY);

		uint32_t color_phong(uint32_t color, float diffuse, float specular);
        void clipTriangle(const std::vector<Vertex>& in, std::vector<Vertex>& out);
        void projectAndDraw(const std::vector<Vertex>& triangle, const Material& mat, const Light& light, simd::Vec3f V);
        uint32_t shade(const Material& m, float u, float v,
                   simd::Vec3f n, const Light& light, simd::Vec3f V);

        float ambiantLightIntensity = 0.18f;
		float specularStrength = 0.5f;
        Light m_light{ simd::Vec3f{ 0.57735f, 0.57735f, 0.57735f }, {1.f, 1.f, 1.f}, ambiantLightIntensity };

		// Multithreading members
		std::vector<std::thread> m_threads;
		std::mutex m_mutex;
		std::condition_variable m_cvStartThreads;
		std::condition_variable m_cvThreadsDone;

		bool m_stopThreads = false;
		uint64_t m_frameCounter = 0;
		int m_activeThreads = 0;
        int m_numThreads;
		void threadLoop(int threadId);
        void renderTile(int threadId);
        ScreenVertex projectVertex(const Vertex& vertex, int width, int height);
    };
}