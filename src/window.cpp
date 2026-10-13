#include "window.hpp"

namespace AC_Engine
{
    Window::Window(int width, int height, const char* title)
    {
#if defined(__linux__)
        // Dear ImGui's GLFW multi-viewport backend currently disables
        // detached windows when GLFW runs on native Wayland. Use XWayland
        // so ImGui can create and manage separate native windows.
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // Create a windowed mode window and its OpenGL context
        GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (!window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        this->window = window;
        glfwMakeContextCurrent(window);
    }

    Window::~Window()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool Window::shouldClose() const
    {
        return glfwWindowShouldClose(window);
    }

    void Window::swapBuffers()
    {
        glfwSwapBuffers(window);
    }

    void Window::pollEvents()
    {
        glfwPollEvents();
    }
}