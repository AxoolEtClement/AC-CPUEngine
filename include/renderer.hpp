#pragma once
#include "frameBuffer.hpp"
#include <math.h>
#include "Mat4x4.hpp"
#include "Vec4.hpp"
#include "renderObject.hpp"
#include <vector>
#include "camera.hpp"

namespace AC_Engine
{
    class Renderer
    {
    public:
        Renderer(FrameBuffer& frameBuffer) : m_frameBuffer(frameBuffer) {};
        ~Renderer() = default;

        void drawLine(int x0, int y0, int x1, int y1, unsigned int color);
        void drawDemoRGBTriangle(simd::Vec4& v0, simd::Vec4& v1, simd::Vec4& v2, float t);
        void renderObjectsWireframe(std::vector<renderObject> renderObjects, Camera cam);
    private:
        FrameBuffer& m_frameBuffer;
    };
}