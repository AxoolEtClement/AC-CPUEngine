#define STB_IMAGE_IMPLEMENTATION
#include "window.hpp"
#include "frameBuffer.hpp"
#include "renderer.hpp"
#include "gui.hpp"
#include "Vec4.hpp"
#include "cameraManager.hpp"
#include "objparser.hpp"
#include "texture.hpp"
#include <iostream>

using namespace std::string_literals;

int main() {
    AC_Engine::Window window(640, 480, "AC-CPUEngine");
    AC_Engine::FrameBuffer frameBuffer(640, 480);
    AC_Engine::Renderer renderer(frameBuffer);
    AC_Engine::gui gui;
    gui.init(window.getWindow());

    AC_Engine::CameraManager cam{};
    cam.createCamera(AC_Engine::FixedCamera{});
    cam.setActiveCamera(0);

    simd::Matrix4x4 rotation{ simd::Matrix4x4::RotationX(0.01f) * simd::Matrix4x4::RotationY(0.01f) };

    renderObject obj = AC_Engine::ObjParser::parseObjFile(PROJECT_DIR + "/assets/spot.obj"s);
	obj.initNormals();

    std::cout << "Vertices: " << obj.vertices.size() << std::endl;
    std::cout << "Faces: " << obj.faces.size() << std::endl;

    Texture texture = TextureLoader::loadTexture(PROJECT_DIR + "/assets/spot_texture.png"s);
    std::cout << "Texture loaded: " << texture.width << "x" << texture.height << std::endl;

    std::vector<std::pair<renderObject, Texture>> renderObjects = { {obj, texture} };
    // Main loop
    while (!window.shouldClose()) 
    {
        renderObjects[0].first.modelMatrix *= rotation;
        window.pollEvents();
        gui.newFrame();
        cam.getActiveCamera().updateInfos(gui.getFrameBufferData().width, gui.getFrameBufferData().height, 90.f, 0.1f, 100.f);
        frameBuffer.update(gui.getFrameBufferData().width, gui.getFrameBufferData().height, gui.getFrameBufferData().clearColor); // Update the framebuffer in case the window size has changed
        renderer.renderObjects(renderObjects, cam.getActiveCamera());
        gui.render(frameBuffer);
        window.swapBuffers();
        frameBuffer.clear();
    }
    gui.shutdown();
}