#pragma once
#include "ECS/components/cameraComponent.hpp"
#include "GLFW/glfw3.h"
#include "ECS/registry.hpp"

void cameraMover(GLFWwindow* window, AC_Engine::ECS::Registry& registry,unsigned int cameraID, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        auto& cam = registry.getComponent<AC_Engine::CameraComponent>(cameraID);
        cam.pos += cam.forward * deltaTime * 5.f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        auto& cam = registry.getComponent<AC_Engine::CameraComponent>(cameraID);
        cam.pos -= cam.forward * deltaTime * 5.f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        auto& cam = registry.getComponent<AC_Engine::CameraComponent>(cameraID);
        simd::Vec3f right = cam.forward.Cross(cam.up).Normalize();
        cam.pos += right * deltaTime * 5.f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        auto& cam = registry.getComponent<AC_Engine::CameraComponent>(cameraID);
        simd::Vec3f right = cam.forward.Cross(cam.up).Normalize();
        cam.pos -= right * deltaTime * 5.f;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        auto& cam = registry.getComponent<AC_Engine::CameraComponent>(cameraID);
        cam.pos += cam.up * deltaTime * 5.f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        auto& cam = registry.getComponent<AC_Engine::CameraComponent>(cameraID);
        cam.pos -= cam.up * deltaTime * 5.f;
    }
}