#include "window.hpp"
#include "frameBuffer.hpp"
#include "renderer.hpp"

int main() {
    AC_Engine::Window window(640, 480, "AC-CPUEngine");
    AC_Engine::FrameBuffer frameBuffer(window.getWindow());
    AC_Engine::Renderer renderer(frameBuffer);

    

    // Main loop
    while (!window.shouldClose()) {
        renderer.drawLine(100, 100, 200, 200, 0xFFFFFFFF); // Draw a white line
        renderer.drawLine(200, 200, 300, 100, 0xFF00FF00); // Draw a green line
        renderer.drawLine(300, 100, 100, 100, 0x000000FF); // Draw a red line
        glDrawPixels( frameBuffer.getWidth(), frameBuffer.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer.getPixels() );
        frameBuffer.update(); // Update the framebuffer in case the window size has changed
        window.swapBuffers();
        window.pollEvents();

    }
}