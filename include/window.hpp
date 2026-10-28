#pragma once
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace AC_Engine
{
    class Window 
    {
    public:
        Window(int width, int height, const char* title);
        ~Window();
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
        bool shouldClose() const;
        void swapBuffers();
        void pollEvents();

        GLFWwindow* getWindow() const { return window; }
        int getWidth() const
        {
            int w = 0, h = 0;
            glfwGetWindowSize(window, &w, &h);
            return w;
        }
        int getHeight() const
        {
            int w = 0, h = 0;
            glfwGetWindowSize(window, &w, &h);
            return h;
        }

    private:
        GLFWwindow* window;
    };
}