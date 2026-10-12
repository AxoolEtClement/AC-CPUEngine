#include "window.hpp"
#include "frameBuffer.hpp"
#include "renderer.hpp"

int main() {
    AC_Engine::Window window(640, 480, "AC-CPUEngine");
    AC_Engine::FrameBuffer frameBuffer(window.getWindow());
    AC_Engine::Renderer renderer(frameBuffer);

    

    // Main loop
    while (!window.shouldClose()) 
    {
        frameBuffer.update(); // Update the framebuffer in case the window size has changed
        renderer.drawDemoRGBTriangle(frameBuffer.getWidth() / 2, 0,  frameBuffer.getWidth() - 1, frameBuffer.getHeight() - 1,  0, frameBuffer.getHeight() - 1);  // Draw a demo RGB triangle
        glViewport(0, 0, frameBuffer.getWidth(), frameBuffer.getHeight());
        glRasterPos2f(-1.0f, 1.0f);   // coin haut-gauche de la fenêtre
        glPixelZoom(1.0f, -1.0f);     // dessine vers le bas
        glDrawPixels( frameBuffer.getWidth(), frameBuffer.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer.getPixels() );
        window.swapBuffers();
        window.pollEvents();
        frameBuffer.clear();
    }
}