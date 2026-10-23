#pragma once
#include "frameBuffer.hpp"
#include <math.h>
#include "Mat4x4.hpp"
#include "Vec4.hpp"
#include "renderObject.hpp"
#include <vector>
#include "camera.hpp"
#include "texture.hpp"

namespace AC_Engine
{
    class Renderer
    {
    public:
        Renderer(FrameBuffer& frameBuffer) : m_frameBuffer(frameBuffer) {};
        ~Renderer() = default;

        void renderFrame();

        void drawLine(int x0, int y0, int x1, int y1, unsigned int color);
        void drawDemoRGBTriangle(simd::Vec4& v0, simd::Vec4& v1, simd::Vec4& v2, float t);
        void renderObjectsWireframe(std::vector<renderObject> renderObjects, Camera cam);
        void renderObjects(std::vector<std::pair<renderObject, Texture>>& renderObjects, Camera cam);
    private:
        FrameBuffer& m_frameBuffer;

        float barycentricWeights(int x0, int y0, int x1, int y1, int x2, int y2);
        void drawTriangle(int x0, int y0, float z0, int x1, int y1, float z1, int x2, int y2, float z2);
        void drawTriangleTexturedAndLit(int x0, int y0, float z0, float iw0, std::pair<float, float> uv0, int x1, int y1, float z1, float iw1, std::pair<float, float> uv1, int x2, int y2, float z2, float iw2, std::pair<float, float> uv2, const Texture& texture, float brightness);
        void drawTrianglePhong(
            int x0, int y0, float z0, float iw0, std::pair<float, float> uv0, simd::Vec3f n0,
			int x1, int y1, float z1, float iw1, std::pair<float, float> uv1, simd::Vec3f n1,
			int x2, int y2, float z2, float iw2, std::pair<float, float> uv2, simd::Vec3f n2,
            const Texture& texture, simd::Vec3f L, simd::Vec3f H);
		uint32_t color_phong(uint32_t color, float diffuse, float specular);
        void clipTriangle(const std::vector<Vertex>& in, std::vector<Vertex>& out);
        void projectAndDraw(const std::vector<Vertex>& triangle, const Texture& tex, simd::Vec3f L, simd::Vec3f H);

        float ambiantLightIntensity = 0.18f;
		float specularStrength = 0.5f;
    };
}