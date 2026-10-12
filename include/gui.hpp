#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <string>
#include <chrono>
#include <vector>
#include <numeric>

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
        static void init(GLFWwindow* window);
        /**
         * @brief Begins a new frame.
         */
        void newFrame();
        /**
         * @brief Displays the GUI windows.
         */
        void windows();
        /**
         * @brief Renders the GUI.
         */
        void render();
        /**
         * @brief Shuts down the GUI.
         */
        static void shutdown();

    private:
        /**
         * @brief The delta time between frames.
         */
        float m_deltaTime;
        /**
         * @brief The start time of the frame.
         */
        std::chrono::high_resolution_clock::time_point m_startTime;
        /**
         * @brief The end time of the frame.
         */
        std::chrono::high_resolution_clock::time_point m_endTime;
        /**
         * @brief A history of FPS values for calculating the average FPS.
         */
        std::vector<float> m_fpsHistory;
    };
}