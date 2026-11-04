#pragma once
#include <math.h>
#include "Mat4x4.hpp"
#include "Vec4.hpp"
#include <vector>

#include "frameBuffer.hpp"
#include "meshManager.hpp"
#include "renderData.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "Particle.hpp"

#include "ECS/registry.hpp"
#include "ECS/components/cameraComponent.hpp"
#include "ECS/components/meshComponent.hpp"
#include "ECS/components/transformComponent.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>

namespace AC_Engine
{
    struct Light 
    {
        simd::Vec3f dir;
        Color3 color { 1.f, 1.f, 1.f };
        float ambient = 0.18f;
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
        Renderer(FrameBuffer& frameBuffer, MaterialLibrary& materialLibrary, MeshManager& meshManager);
        ~Renderer();

        void renderFrame();

        void drawLine(int x0, int y0, int x1, int y1, unsigned int color);
        void renderObjectsWireframe(std::vector<Mesh> renderObjects, Camera& cam);
        void renderObjects(ECS::Registry& registry, unsigned int cameraEntityID);

        void appendParticlesToRender(const ParticlePool& particlePool, ECS::Registry& registry, unsigned int cameraEntityID);
    
    private:
        float barycentricWeights(int x0, int y0, int x1, int y1, int x2, int y2);
        void drawTrianglePhongInTile(
            const ScreenVertex& v0, const ScreenVertex& v1, const ScreenVertex& v2,
            const Material& mat, const Light& light,
            int tileMinY, int tileMaxY);

		uint32_t color_phong(uint32_t color, float diffuse, float specular);
        void clipTriangle(const std::vector<Vertex>& in, std::vector<Vertex>& out);

        FrameBuffer& m_frameBuffer;
        MaterialLibrary& m_materialLibrary;
        MeshManager& m_meshManager;
        std::vector<RenderTriangle> m_trianglesToRender;

        float ambiantLightIntensity = 0.18f;
		float specularStrength = 0.5f;
        Light m_light{ simd::Vec3f{ 0.57735f, 0.57735f, 0.57735f }, { 1.f, 1.f, 1.f }, ambiantLightIntensity };

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