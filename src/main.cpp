#include "window.hpp"
#include "frameBuffer.hpp"
#include "renderer.hpp"
#include "gui.hpp"
#include "Vec4.hpp"
#include "cameraManager.hpp"
#include <iostream>

int main() {
    AC_Engine::Window window(640, 480, "AC-CPUEngine");
    AC_Engine::FrameBuffer frameBuffer(640, 480);
    AC_Engine::Renderer renderer(frameBuffer);
    AC_Engine::gui gui;
    gui.init(window.getWindow());

    simd::Vec4 v0{ -1, -1, -1, 1 };
    simd::Vec4 v1{ 1, -1, -1, 1 };
    simd::Vec4 v2{ 1,  1, -1, 1 };
    simd::Vec4 v3{ -1,  1, -1, 1 };
    simd::Vec4 v4{ -1, -1,  1, 1 };
    simd::Vec4 v5{ 1, -1,  1, 1 };
    simd::Vec4 v6{ 1,  1,  1, 1 };
    simd::Vec4 v7{ -1,  1,  1, 1 };

    AC_Engine::CameraManager cam{};
    cam.createCamera(AC_Engine::FixedCamera{});
    cam.setActiveCamera(0);
    std::vector <std::pair< simd::Vec4, simd::Vec4 >> edges{
        {v0, v1}, {v3, v2}, {v7, v6}, {v4, v5},
        {v0, v3}, {v1, v2}, {v5, v6}, {v4, v7},
        {v0, v4}, {v1, v5}, {v2, v6}, {v3, v7}
    };

    simd::Matrix4x4 model{};
    simd::Matrix4x4 rotation{ simd::Matrix4x4::RotationX(0.01f) * simd::Matrix4x4::RotationY(0.01f) };

    // Main loop
    while (!window.shouldClose()) 
    {
        model *= rotation;
        window.pollEvents();
        gui.newFrame();
        cam.getActiveCamera().updateInfos(gui.getSceneWidth(), gui.getSceneHeight(), 90.f, 0.1f, 100.f);
        frameBuffer.update(gui.getSceneWidth(), gui.getSceneHeight(), gui.getFrameBufferData().clearColor); // Update the framebuffer in case the window size has changed
        renderer.renderObjectsWireframe(edges, cam.getActiveCamera(), model);
        gui.render(frameBuffer);
        window.swapBuffers();
        frameBuffer.clear();
    }
    gui.shutdown();
}