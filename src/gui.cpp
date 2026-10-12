#include "gui.hpp"

namespace AC_Engine
{
    void gui::init(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void gui::newFrame()
    {
        m_startTime = std::chrono::high_resolution_clock::now();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void gui::windows()
    {
        // Example window
        ImGui::Begin("Example Window");
        ImGui::Text("Hello, world!");
        ImGui::End();
    }

    void gui::render()
    {
        m_endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> deltaTime = m_endTime - m_startTime;
        m_deltaTime = deltaTime.count();

        // Update FPS history
        if (m_fpsHistory.size() >= 100) {
            m_fpsHistory.erase(m_fpsHistory.begin());
        }
        m_fpsHistory.push_back(1.0f / m_deltaTime);

        // Calculate average FPS
        float averageFPS = std::accumulate(m_fpsHistory.begin(), m_fpsHistory.end(), 0.0f) / m_fpsHistory.size();

        // Display FPS in a window
        ImGui::Begin("Performance");
        ImGui::Text("Average FPS: %.1f", averageFPS);
        ImGui::End();

        // Render the GUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void gui::shutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}