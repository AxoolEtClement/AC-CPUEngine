#include "window.hpp"
#include "frameBuffer.hpp"
#include "renderer.hpp"

int main() {
    AC_Engine::Window window(640, 480, "AC-CPUEngine");
    AC_Engine::FrameBuffer frameBuffer(640, 480);
    AC_Engine::Renderer renderer(frameBuffer);

    renderer.drawLine(100, 100, 200, 200, 0xFFFFFFFF); // Draw a white line

    // Main loop
    while (!window.shouldClose()) {
        glDrawPixels( frameBuffer.getWidth(), frameBuffer.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer.getPixels() );
        window.swapBuffers();
        window.pollEvents();

    }
}