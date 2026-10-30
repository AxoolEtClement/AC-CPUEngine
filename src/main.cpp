#define STB_IMAGE_IMPLEMENTATION
#include "window.hpp"
#include "frameBuffer.hpp"
#include "renderer.hpp"
#include "gui.hpp"
#include "Vec4.hpp"
#include "cameraManager.hpp"
#include "objparser.hpp"
#include "texture.hpp"
#include "font.hpp"
#include "material.hpp"
#include "Particle.hpp"
#include "proceduralTerrain.hpp"
#include <iostream>

using namespace std::string_literals;

int main() {
    MaterialLibrary materialLibrary;
    Material mat;
	Material particleMat;
    Material terrainMat;
    AC_Engine::Window window(640, 480, "AC-CPUEngine");
    AC_Engine::FrameBuffer frameBuffer(640, 480);
    AC_Engine::Renderer renderer(frameBuffer,materialLibrary);
    AC_Engine::gui gui;
    gui.init(window.getWindow());

    AC_Engine::CustomUI customUI;

    AC_Engine::CameraManager cam{};
    cam.createCamera(AC_Engine::EditorCamera{});
    cam.setActiveCamera(0);

    simd::Matrix4x4 rotation{ simd::Matrix4x4::RotationX(0.01f) * simd::Matrix4x4::RotationY(0.01f) };


    AC_Engine::ProceduralTerrain terrain(100, 100);
    terrain.generateTerrain(0.04f, 0.5f, 4, 15.f);
    terrain.generateObj(PROJECT_DIR + "/assets/terrain.obj"s);
    renderObject obj = AC_Engine::ObjParser::parseObjFile(PROJECT_DIR + "/assets/spot.obj"s);
    renderObject terrainObj = AC_Engine::ObjParser::parseObjFile(PROJECT_DIR + "/assets/terrain.obj"s);
    std::cout << "Terrain vertices: " << terrainObj.vertices.size() << std::endl;
    terrainObj.initNormals();
	obj.initNormals();

    std::cout << "Terrain faces: " << terrainObj.faces.size() << std::endl;

    std::cout << "Vertices: " << obj.vertices.size() << std::endl;
    std::cout << "Faces: " << obj.faces.size() << std::endl;

    Texture texture = TextureLoader::loadTexture(PROJECT_DIR + "/assets/spot_texture.png"s);
    std::cout << "Texture loaded: " << texture.width << "x" << texture.height << std::endl;
    Texture particleTexture = TextureLoader::loadTexture(PROJECT_DIR + "/assets/particle_tex.png"s);
    std::cout << "Texture loaded: " << particleTexture.width << "x" << particleTexture.height << std::endl;
    Texture TerrainTexture = TextureLoader::loadTexture(PROJECT_DIR + "/assets/terrain.png"s);
    std::cout << "Texture loaded: " << TerrainTexture.width << "x" << TerrainTexture.height << std::endl;

    mat.diffuseMap = std::make_shared<Texture>(texture);
    mat.shininess  = 32.f;
    mat.buildSpecLut();

	particleMat.diffuseMap = std::make_shared<Texture>(particleTexture);
	particleMat.shininess = 32.f;
	particleMat.emissive = { 0.5f, 0.5f, 0.5f };
	particleMat.buildSpecLut();

    terrainMat.diffuseMap = std::make_shared<Texture>(TerrainTexture);
    terrainMat.shininess = 100.f;
    terrainMat.emissive = { 0.f, 0.f, 0.f };
    terrainMat.buildSpecLut();

    obj.materialId = materialLibrary.add(mat);
    terrainObj.materialId = materialLibrary.add(terrainMat);
    std::vector<renderObject> renderObjects = {obj, terrainObj};

	AC_Engine::ParticlePool particlePool(1000);
	AC_Engine::ParticleEmitter particleEmitter(&particlePool, { 0.f, 0.f, 0.f }, 1.f, 2.f, 0.1f, &particleMat);
	particleEmitter.setVelocity({ 0.f, 1.f, 0.f });

    // Main loop
    while (!window.shouldClose()) 
    {
        renderObjects[0].modelMatrix *= rotation;
        window.pollEvents();
        customUI.drawText(frameBuffer.getUiPixelsRaw(), window.getWidth(), window.getHeight(),"FPS: 60", 10.0f, 20.0f);
        gui.newFrame();

        cam.getActiveCamera().updateInfos(window.getWidth(), window.getHeight(), 90.f, 0.1f, 100.f);
        cam.updateActiveCamera(*window.getWindow());

        frameBuffer.update(window.getWidth(), window.getHeight(), 0x00000000); // Update the framebuffer in case the window size has changed

		particleEmitter.update(0.016f); // TODO : change to real delta time
		particlePool.updateParticles(0.016f); // TODO : change to real delta time

		renderer.appendParticlesToRender(particlePool, cam.getActiveCamera());
        renderer.renderObjects(renderObjects, cam.getActiveCamera());
        renderer.renderFrame();
        //gui.render(frameBuffer);
        window.swapBuffers();
        frameBuffer.clear();
    }
    gui.shutdown();
}