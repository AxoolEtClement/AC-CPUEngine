#pragma once
#include "frameBuffer.hpp"
#include <math.h>

namespace AC_Engine
{
    class Renderer
    {
    public:
        Renderer(FrameBuffer& frameBuffer) : m_frameBuffer(frameBuffer) {};
        ~Renderer() = default;

        void drawLine(int x0, int y0, int x1, int y1, unsigned int color);
        void drawDemoRGBTriangle(int x0, int y0, int x1, int y1, int x2, int y2);
    private:
        FrameBuffer& m_frameBuffer;
    };
}