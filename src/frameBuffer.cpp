#include "frameBuffer.hpp"

namespace AC_Engine
{
    FrameBuffer::FrameBuffer(int width, int height)
        : m_width(width), m_height(height), m_prixels(width * height, 0) {}

    FrameBuffer::~FrameBuffer() {}

    void FrameBuffer::setPixel(int x, int y, unsigned int color)
    {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return; // Out of bounds
        }
        m_prixels[y * m_width + x] = color;
    }

    void FrameBuffer::clear()
    {
        std::fill(m_prixels.begin(), m_prixels.end(), 0);
    }
}