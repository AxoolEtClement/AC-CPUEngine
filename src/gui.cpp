#include "gui.hpp"
#include "frameBuffer.hpp"

namespace AC_Engine
{
    void gui::init(GLFWwindow* window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
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
        ImGui::End();

        // Display the scene in a window
        ImGui::Begin("Scene");
        const int width = frameBuffer.getWidth();
        const int height = frameBuffer.getHeight();
        glBindTexture(GL_TEXTURE_2D, m_sceneTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer.getPixels());
        glBindTexture(GL_TEXTURE_2D, 0);

        const ImVec2 availableSize = ImGui::GetContentRegionAvail();
        const float scale = std::min(availableSize.x / static_cast<float>(width),
                                     availableSize.y / static_cast<float>(height));
        const ImVec2 imageSize(width * scale, height * scale);
        ImGui::Image(static_cast<ImTextureID>(m_sceneTexture), imageSize,
                     ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
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
        ImGui::DestroyContext();
    }
}