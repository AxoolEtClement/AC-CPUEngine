#include "editorCamera.hpp"


void AC_Engine::EditorCamera::update(GLFWwindow& window,float deltaTime)
{
    if (glfwGetKey(&window, GLFW_KEY_W) == GLFW_PRESS)
    {
        pos += getForward() * deltaTime * 5.f;
    }
    if (glfwGetKey(&window, GLFW_KEY_S) == GLFW_PRESS)
    {
        pos -= getForward() * deltaTime * 5.f;
    }
    if (glfwGetKey(&window, GLFW_KEY_A) == GLFW_PRESS)
    {
        pos += getForward().Cross(getUp()).Normalize() * deltaTime * 5.f;
    }
    if (glfwGetKey(&window, GLFW_KEY_D) == GLFW_PRESS)
    {
        pos -= getForward().Cross(getUp()).Normalize() * deltaTime * 5.f;
    }
    if (glfwGetKey(&window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        pos -= getUp() * deltaTime * 5.f;
    }
    if (glfwGetKey(&window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        pos += getUp() * deltaTime * 5.f;
    }
}