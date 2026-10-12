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
        bool shouldClose() const;
        void swapBuffers();
        void pollEvents();

        GLFWwindow* getWindow() const { return window; }

    private:
        GLFWwindow* window;
    };
}