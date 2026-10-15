#include "gui.hpp"
#include "frameBuffer.hpp"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

namespace AC_Engine
{
    void gui::init(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGui::LoadIniSettingsFromDisk("../config/imguiDefaultLayout.ini");
        ImGuiIO& io = ImGui::GetIO();
        ImGui::StyleColorsDark();
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        // Keep detached ImGui windows in their own native GLFW windows.
        io.ConfigViewportsNoAutoMerge = true;
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;

        glGenTextures(1, &m_sceneTexture);
        glBindTexture(GL_TEXTURE_2D, m_sceneTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void gui::newFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create a docking space that fills the whole viewport.
        // This must be done after ImGui::NewFrame() and before any dockable windows are created.
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_None, nullptr);
    }

    void gui::render(const FrameBuffer& frameBuffer)
    {
        // Display FPS in a window
        ImGui::Begin("Performance");
        ImGui::Text("FPS: %.1f (%.2f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
        m_fpsHistory.push_back(ImGui::GetIO().Framerate);
        if (m_fpsHistory.size() > 100)
            m_fpsHistory.erase(m_fpsHistory.begin());
        if (ImPlot::BeginPlot("FPS History", ImVec2(-1, 150)))
        {
            ImPlot::SetupAxes("Frame", "FPS");
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, 100, ImPlotCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 200, ImPlotCond_Always);
            ImPlot::PlotLine("FPS", m_fpsHistory.data(), static_cast<int>(m_fpsHistory.size()));
            ImPlot::EndPlot();
        }
        ImGui::ColorPicker4("Clear Color", m_clearColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        ImGui::End();

        // Display the scene in a window
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        const bool sceneVisible = ImGui::Begin("Scene", nullptr,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::PopStyleVar();
        
        if (sceneVisible)
        {
            // Taille disponible dans la fenêtre : sera utilisée par le framebuffer à la frame suivante
            const ImVec2 avail = ImGui::GetContentRegionAvail();
            m_sceneWidth  = std::max(1, static_cast<int>(avail.x));
            m_sceneHeight = std::max(1, static_cast<int>(avail.y));
        
            const int width = frameBuffer.getWidth();
            const int height = frameBuffer.getHeight();
            glBindTexture(GL_TEXTURE_2D, m_sceneTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer.getPixels());
            glBindTexture(GL_TEXTURE_2D, 0);
            
            // Affichage 1:1 (le framebuffer a la taille de la zone)
            ImGui::Image(static_cast<ImTextureID>(m_sceneTexture),
                         ImVec2(static_cast<float>(m_sceneWidth), static_cast<float>(m_sceneHeight)),
                         ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
        }
        ImGui::End();

        // Render the GUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupCurrentContext);
        }
    }

    void gui::shutdown()
    {
        if (m_sceneTexture != 0)
        {
            glDeleteTextures(1, &m_sceneTexture);
            m_sceneTexture = 0;
        }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }
}