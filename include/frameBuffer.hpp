#pragma once
#include <vector>
#include <cstdint>

namespace AC_Engine
{
    class FrameBuffer
    {
    public:
        FrameBuffer(int width, int height);
        ~FrameBuffer();
        void setPixel(int x, int y, unsigned int color);
        void clear();
        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }
        const uint32_t* getPixels() const { return m_prixels.data(); }

    private:
        int m_width;
        int m_height;
        std::vector<unsigned int> m_prixels;
    };
}