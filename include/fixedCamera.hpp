#pragma once 
#include "camera.hpp"

namespace AC_Engine
{
    /**
     * @brief A class for a fixed camera that inherits from the Camera class.
     */
    class FixedCamera : public Camera
    {
    public:
        FixedCamera() = default;
        ~FixedCamera() = default;

        void update(GLFWwindow& window,float deltaTime) override
        {
            // Fixed camera does not update its position or orientation
        }
    };
}