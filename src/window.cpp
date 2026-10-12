#include "window.hpp"

namespace AC_Engine
{
    Window::Window(int width, int height, const char* title)
    {
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