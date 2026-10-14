#include "window.hpp"
#include "frameBuffer.hpp"
#include "renderer.hpp"
#include "gui.hpp"
#include "Vec4.hpp"

int main() {
    AC_Engine::Window window(640, 480, "AC-CPUEngine");
    AC_Engine::FrameBuffer frameBuffer(640, 480);
    AC_Engine::Renderer renderer(frameBuffer);
    AC_Engine::gui gui;
    gui.init(window.getWindow());

    simd::Vec4 v0{ 0.f, -100.f, 0.f, 1.f };
    simd::Vec4 v1{ 100.f, 50.f, 0.f, 1.f };
    simd::Vec4 v2{ -100.f, 50.f, 0.f, 1.f };

    // Main loop
    while (!window.shouldClose()) 
    {
        window.pollEvents();
        gui.newFrame();
        frameBuffer.update(gui.getSceneWidth(), gui.getSceneHeight()); // Update the framebuffer in case the window size has changed
        renderer.drawDemoRGBTriangle(v0, v1, v2, 0.01f);  // Draw a demo RGB triangle
        gui.render(frameBuffer);
        window.swapBuffers();
        frameBuffer.clear();
    }
    gui.shutdown();
}