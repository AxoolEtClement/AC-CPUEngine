#pragma once
#include "camera.hpp"
#include <GLFW/glfw3.h>

namespace AC_Engine
{
    /**
     * @brief A class for an editor camera that inherits from the Camera class.
     */
    class EditorCamera : public Camera
    {
    public:
        EditorCamera() = default;
        ~EditorCamera() = default;

        void update(GLFWwindow& window,float deltaTime) override;

    };
}