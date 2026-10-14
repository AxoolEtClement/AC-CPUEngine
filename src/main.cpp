#include "window.hpp"
#include "frameBuffer.hpp"
#include "renderer.hpp"
#include "gui.hpp"
#include "Mat4x4.hpp"

int main() {
    AC_Engine::Window window(640, 480, "AC-CPUEngine");
    AC_Engine::FrameBuffer frameBuffer(640, 480);
    AC_Engine::Renderer renderer(frameBuffer);
    AC_Engine::gui gui;
    gui.init(window.getWindow());

    simd::Matrix4x4 mat;
    // Main loop
    while (!window.shouldClose()) 
    {
        window.pollEvents();
        gui.newFrame();
        frameBuffer.update(gui.getSceneWidth(), gui.getSceneHeight()); // Update the framebuffer in case the window size has changed
        renderer.drawDemoRGBTriangle(frameBuffer.getWidth() / 2, 0,  frameBuffer.getWidth() - 1, frameBuffer.getHeight() - 1,  0, frameBuffer.getHeight() - 1);  // Draw a demo RGB triangle
        gui.render(frameBuffer);
        window.swapBuffers();
        frameBuffer.clear();
    }
    gui.shutdown();
}