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
        FrameBuffer(GLFWwindow* window);
        ~FrameBuffer();
        void setPixel(int x, int y, unsigned int color);
        void update();
        void clear();
        int getWidth() const { return m_width; }
        int getHeight() const { return m_height; }
        const uint32_t* getPixels() const { return m_prixels.data(); }

    private:
        int m_width;
        int m_height;
        std::vector<uint32_t> m_prixels;
        GLFWwindow* m_window;
    };
}