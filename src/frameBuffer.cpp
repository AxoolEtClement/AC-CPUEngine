#include "frameBuffer.hpp"

namespace AC_Engine
{
    FrameBuffer::FrameBuffer(int width, int height) : m_width(width), m_height(height)
    {
        m_prixels.resize(m_width * m_height, 0);
        m_zBuffer.resize(m_width * m_height, 1e30f);

        for (std::size_t i = 0; i < m_prixels.size(); ++i) 
        {
            m_prixels[i] = 0x00FF0000; // Set all pixels to opaque blue for framebuffer size testing
        }
    }

    FrameBuffer::~FrameBuffer() {}

    void FrameBuffer::setPixel(int x, int y, unsigned int color)
    {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
            return; // Out of bounds
        }
        m_prixels[y * m_width + x] = color;
    }

    void FrameBuffer::update(int width, int height, float clearColor[4])
    {
        if (clearColor) {
            m_clearColor = (static_cast<uint32_t>(clearColor[0] * 255) << 24) |
                           (static_cast<uint32_t>(clearColor[1] * 255) << 16) |
                           (static_cast<uint32_t>(clearColor[2] * 255) << 8) |
                           (static_cast<uint32_t>(clearColor[3] * 255));
        }

        width = std::max(width, 1);   // évite une taille nulle (fenêtre réduite)
        height = std::max(height, 1);
        if (width == m_width && height == m_height) return;
        
        // This function can be used to update the framebuffer if needed.
        // For now, we'll just resize the pixel buffer to match the current framebuffer size.
        m_width = width;
        m_height = height;
        m_prixels.resize(m_width * m_height);
        m_zBuffer.resize(m_width * m_height);
    }

    void FrameBuffer::clear()
    {
        std::fill(m_prixels.begin(), m_prixels.end(), m_clearColor ? m_clearColor : 0x00000000);

    }
    void FrameBuffer::clearZBuffer()
    {
        for (size_t i = 0; i < m_zBuffer.size(); i++) m_zBuffer[i] = 1e30f;
    }
}