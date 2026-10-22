#pragma once
#include "camera.hpp"
#include "editorCamera.hpp"
#include "fixedCamera.hpp"
#include <vector>
#include <variant>

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

        void createCamera(std::variant<EditorCamera, FixedCamera> camera)
        {
            m_cameras.push_back(std::visit([](auto&& cam) -> Camera { return cam; }, camera));
        }
        Camera& getActiveCamera() { return m_cameras[m_activeCameraId]; }
        void setActiveCamera(const int& cameraid) { m_activeCameraId = cameraid; }

    private:
        std::vector<Camera> m_cameras{};
        int m_activeCameraId{};
    };
}