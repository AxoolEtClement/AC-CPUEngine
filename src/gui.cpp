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


    CustomUI::CustomUI()
    {
        m_fontLoaded = m_font.Load("/home/clement/dev/AC-CPUEngine/assets/CreatoDisplay-Regular.otf", 16);
        if (!m_fontLoaded)
            fprintf(stderr, "Impossible de charger la police\n");
    }

    void CustomUI::drawText(uint8_t* dst, int dstW, int dstH,
                            const char* text, float x, float y, uint32_t color) const
    {
        if (!m_fontLoaded || !dst) return;
        auto mark = [&](int x0, int y0, uint8_t r, uint8_t g, uint8_t b)
        {
            for (int j = 0; j < 8; ++j)
                for (int i = 0; i < 8; ++i)
                {
                    int px = x0 + i, py = y0 + j;
                    if (px < 0 || px >= dstW || py < 0 || py >= dstH) continue;
                    uint8_t* d = &dst[(py * dstW + px) * 4];
                    d[0] = r; d[1] = g; d[2] = b; d[3] = 255;
                }
        };
        mark(0,        0,        255, 0, 0);   // rouge : coin (0,0) du buffer
        mark(dstW - 8, 0,        0, 255, 0);   // vert  : coin (dstW-1, 0)
        mark(0,        dstH - 8, 0, 0, 255);   // bleu  : coin (0, dstH-1)

        const float cr = (color >> 24) & 0xFF;
        const float cg = (color >> 16) & 0xFF;
        const float cb = (color >>  8) & 0xFF;
        const float ca =  color        & 0xFF;

        for (; *text; ++text)
        {
            int c = static_cast<unsigned char>(*text);
            if (c < 32 || c >= 128) continue;
            const stbtt_bakedchar& g = m_font.m_chars[c - 32];

            const int gw = g.x1 - g.x0;
            const int gh = g.y1 - g.y0;
            const int px = static_cast<int>(x + g.xoff);
            const int py = static_cast<int>(y + g.yoff);

            for (int j = 0; j < gh; ++j)
            {
                const int dy = py + j;
                if (dy < 0 || dy >= dstH) continue;

                for (int i = 0; i < gw; ++i)
                {
                    const int dx = px + i;
                    if (dx < 0 || dx >= dstW) continue;

                    const int cov = m_font.m_atlas[(g.y0 + j) * Font::ATLAS_W + (g.x0 + i)];
                    if (!cov) continue;

                    const int oy = dstH - 1 - dy;
                    uint8_t* d = &dst[(oy * dstW + dx) * 4];
                    const float a  = (cov / 255.0f) * (ca / 255.0f);
                    const float da = d[3] / 255.0f;
                    const float oa = a + da * (1.0f - a);
                    if (oa <= 0.0f) continue;

                    d[0] = static_cast<uint8_t>((cr * a + d[0] * da * (1.0f - a)) / oa);
                    d[1] = static_cast<uint8_t>((cg * a + d[1] * da * (1.0f - a)) / oa);
                    d[2] = static_cast<uint8_t>((cb * a + d[2] * da * (1.0f - a)) / oa);
                    d[3] = static_cast<uint8_t>(oa * 255.0f);
                }
            }
            x += g.xadvance;
        }
    }
}