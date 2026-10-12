#include "frameBuffer.hpp"

namespace AC_Engine
{
    FrameBuffer::FrameBuffer(GLFWwindow* window) : m_window(window)
    {
        glfwGetFramebufferSize(window, &m_width, &m_height);
        m_prixels.resize(m_width * m_height, 0);

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

    void FrameBuffer::update()
    {
        // This function can be used to update the framebuffer if needed.
        glfwGetFramebufferSize(m_window, &m_width, &m_height);
        m_prixels.resize(m_width * m_height, 0);
        for (std::size_t i = 0; i < m_prixels.size(); ++i) 
        {
            m_prixels[i] = 0x00FF0000; // Set all pixels to opaque blue for framebuffer size testing
        }
    }

    void FrameBuffer::clear()
    {
        std::fill(m_prixels.begin(), m_prixels.end(), 0);
    }
}