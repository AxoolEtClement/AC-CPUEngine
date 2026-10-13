#include "window.hpp"
#include "frameBuffer.hpp"
#include "renderer.hpp"
#include "gui.hpp"

int main() {
    AC_Engine::Window window(640, 480, "AC-CPUEngine");
    AC_Engine::FrameBuffer frameBuffer(window.getWindow());
    AC_Engine::Renderer renderer(frameBuffer);
    AC_Engine::gui gui;
    gui.init(window.getWindow());

    // Main loop
    while (!window.shouldClose()) 
    {
        window.pollEvents();
        gui.newFrame();
        frameBuffer.update(); // Update the framebuffer in case the window size has changed
        renderer.drawLine(100, 100, 200, 200, 0xFFFFFFFF); // Draw a white line
        renderer.drawLine(200, 200, 300, 100, 0xFF00FF00); // Draw a green line
        renderer.drawLine(300, 100, 100, 100, 0xFF0000FF); // Draw a red line
        gui.render(frameBuffer);
        window.swapBuffers();

    }
    gui.shutdown();
}