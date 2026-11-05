#pragma once
#include "ECS/components/cameraComponent.hpp"
#include "GLFW/glfw3.h"
#include "ECS/registry.hpp"

void cameraMover(GLFWwindow* window, AC_Engine::ECS::Registry& registry, unsigned int cameraID, float deltaTime)
{
    constexpr float speed = 5.f;

    auto& cam = registry.getComponent<AC_Engine::CameraComponent>(cameraID);
    const simd::Vec3f right = cam.forward.Cross(cam.up).Normalize();
    const float step = speed * deltaTime;

    struct KeyBinding
    {
        int key;
        simd::Vec3f direction;
    };

    const KeyBinding bindings[] = {
        { GLFW_KEY_W,            cam.forward },
        { GLFW_KEY_S,           -cam.forward },
        { GLFW_KEY_A,            right       },
        { GLFW_KEY_D,           -right       },
        { GLFW_KEY_SPACE,        cam.up      },
        { GLFW_KEY_LEFT_CONTROL, -cam.up     },
    };

    for (const auto& [key, direction] : bindings)
    {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            cam.pos += direction * step;
    }
}