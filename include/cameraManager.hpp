#pragma once
#include "camera.hpp"
#include "editorCamera.hpp"
#include "fixedCamera.hpp"
#include <vector>
#include <GLFW/glfw3.h>

namespace AC_Engine
{
    /**
     * @brief A class for a camera manager that inherits from the Camera class.
     */
    class CameraManager
    {
    public:
        CameraManager() = default;
        ~CameraManager() = default;

        template <typename T>
        void createCamera(T&& camera) 
        {
            m_cameras.push_back(std::make_unique<std::decay_t<T>>(std::forward<T>(camera)));
        }
        void updateActiveCamera(GLFWwindow& window)
        {
            m_cameras[m_activeCameraId]->update(window ,0.016f); // Assuming a fixed delta time of 16ms for simplicity
        }
        Camera& getActiveCamera() { return *m_cameras[m_activeCameraId]; }
        void setActiveCamera(const int& cameraid) { m_activeCameraId = cameraid; }

    private:
        std::vector<std::unique_ptr<Camera>> m_cameras;
        int m_activeCameraId{};
    };
}