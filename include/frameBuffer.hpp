#pragma once
#include <vector>
#include <cstdint>
#include <algorithm>
#include <GLFW/glfw3.h>

namespace AC_Engine
{
    class FrameBuffer
    {
    public:
        FrameBuffer(int width, int height);
        ~FrameBuffer();
        void setPixel(int x, int y, unsigned int color);
        void update(int width, int height, float clearColor[4] = nullptr);
        void clear();
        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }
        const uint32_t* getPixels() const { return m_prixels.data(); }
        std::vector<float>& getZBuffer() { return m_zBuffer; }
        void clearZBuffer();

    private:
        int m_width;
        int m_height;
        std::vector<uint32_t> m_prixels;
        std::vector<float> m_zBuffer;
        uint32_t m_clearColor = 0xFF000000;
    };
}