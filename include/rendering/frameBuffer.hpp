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
        void setTransparentPixel(int x, int y, unsigned int color);
        void update(int width, int height, float clearColor[4] = nullptr);
        void clear();
        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }
        const uint32_t* getPixels() const { return m_pixels.data(); }
        const uint32_t* getUiPixels() const { return m_UiPixels.data(); }
        const uint32_t* getTransparentPixels() const { return m_transparentPixels.data(); }
        std::vector<float>& getZBuffer() { return m_zBuffer; }
        uint8_t* getUiPixelsRaw() { return reinterpret_cast<uint8_t*>(m_UiPixels.data()); }
        int getUiWidth()  const { return m_width; }
        int getUiHeight() const { return m_height; }
        void clearZBuffer();

    private:
        int m_width;
        int m_height;
        std::vector<uint32_t> m_pixels;
        std::vector<uint32_t> m_UiPixels;
        std::vector<uint32_t> m_transparentPixels;
        std::vector<float> m_zBuffer;
        uint32_t m_clearColor = 0xFF000000;
    };
}