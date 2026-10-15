#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <GLFW/glfw3.h>
#include <string>
#include <chrono>
#include <vector>
#include <numeric>

namespace AC_Engine
{
    class FrameBuffer;

    struct FrameBufferData
    {
        int width;
        int height;
        float clearColor[4];
    };
}

namespace AC_Engine
{
    /**
     * @brief A class for managing the GUI using ImGui.
     */
    class gui
    {
    public:
        /**
         * @brief Initializes the GUI with the given GLFW window.
         * @param window A pointer to the GLFW window.
         */
        void init(GLFWwindow* window);
        /**
         * @brief Begins a new frame.
         */
        void newFrame();
        /**
         * @brief Renders the GUI.
         */
        void render(const FrameBuffer& frameBuffer);
        /**
         * @brief Shuts down the GUI.
         */
        void shutdown();

    public:
        FrameBufferData getFrameBufferData() const { return { m_sceneWidth, m_sceneHeight, {m_clearColor[3], m_clearColor[2], m_clearColor[1], m_clearColor[0]} }; }

    private:
        /**
         * @brief The delta time between frames.
         */
        float m_deltaTime;
        /**
         * @brief The start time of the frame.
         */
        std::chrono::steady_clock::time_point m_startTime;
        /**
         * @brief The end time of the frame.
         */
        std::chrono::steady_clock::time_point m_endTime;
        /**
         * @brief A history of FPS values for calculating the average FPS.
         */
        std::vector<float> m_fpsHistory;

        unsigned int m_sceneTexture = 0;
        int m_sceneWidth = 640;
        int m_sceneHeight = 480;
        float m_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    };
}