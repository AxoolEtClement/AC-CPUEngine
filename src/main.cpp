#define STB_IMAGE_IMPLEMENTATION
#include "window.hpp"
#include "frameBuffer.hpp"
#include "renderer.hpp"
#include "gui.hpp"
#include "Vec4.hpp"
#include "cameraManager.hpp"
#include "meshManager.hpp"
#include "texture.hpp"
#include "font.hpp"
#include "material.hpp"
#include "Particle.hpp"
#include <iostream>
#include "ECS/registry.hpp"

using namespace std::string_literals;

int main() {
    MaterialLibrary materialLibrary{};
    Material mat{};
    Material particleMat{};
    AC_Engine::ECS::Registry registry{};
    AC_Engine::MeshManager meshManager{};
    AC_Engine::Window window{ 640, 480, "AC-CPUEngine" };
    AC_Engine::FrameBuffer frameBuffer{ 640, 480 };
    AC_Engine::Renderer renderer{ frameBuffer,materialLibrary, meshManager };
    AC_Engine::gui gui{};
    
    gui.init(window.getWindow());

    AC_Engine::CustomUI customUI{};

    /*AC_Engine::CameraManager cam{};
    cam.createCamera(AC_Engine::FixedCamera{});
    cam.setActiveCamera(0);*/

    AC_Engine::ECS::Entity camEntity{ registry.createEntity() };
    registry.addComponent<AC_Engine::CameraComponent>(
        camEntity.id, 
        simd::Vec3f{ 1.f, 1.f, 1.f }, 
        simd::Vec3f{ -0.57735f, -0.57735f, -0.57735f }
    );
    AC_Engine::ECS::Entity spot{ registry.createEntity() };
    

    simd::Matrix4x4 rotation{ simd::Matrix4x4::RotationX(0.01f) * simd::Matrix4x4::RotationY(0.01f) };

    meshManager.addMesh("spot", PROJECT_DIR + "/assets/spot.obj"s);
    meshManager.getMesh("spot")->initNormals();

    registry.addComponent<AC_Engine::MeshComponent>(spot.id, meshManager.getMesh("spot"));
    registry.addComponent<AC_Engine::TransformComponent>(spot.id);

    Texture texture = TextureLoader::loadTexture(PROJECT_DIR + "/assets/spot_texture.png"s);
    Texture particleTexture = TextureLoader::loadTexture(PROJECT_DIR + "/assets/particle_tex.png"s);

    mat.diffuseMap = std::make_shared<Texture>(texture);
    mat.shininess  = 32.f;
    mat.buildSpecLut();

	particleMat.diffuseMap = std::make_shared<Texture>(particleTexture);
	particleMat.shininess = 32.f;
	particleMat.emissive = { 0.5f, 0.5f, 0.5f };
	particleMat.buildSpecLut();

    meshManager.getMesh("spot")->materialId = materialLibrary.add(mat);

	AC_Engine::ParticlePool particlePool(1000);
	AC_Engine::ParticleEmitter particleEmitter(&particlePool, { 0.f, 0.f, 0.f }, 1.f, 2.f, 0.1f, &particleMat);
	particleEmitter.setVelocity({ 0.f, 1.f, 0.f });

    std::vector<Mesh> renderObjects = {*meshManager.getMesh("spot")};

    // Main loop
    while (!window.shouldClose()) 
    {
        renderObjects[0].modelMatrix *= rotation;
        window.pollEvents();
        customUI.drawText(frameBuffer.getUiPixelsRaw(), window.getWidth(), window.getHeight(),"FPS: 60", 10.0f, 20.0f);
        gui.newFrame();

        frameBuffer.update(window.getWidth(), window.getHeight(), 0x00000000); // Update the framebuffer in case the window size has changed

		particleEmitter.update(0.016f); // TODO : change to real delta time
		particlePool.updateParticles(0.016f); // TODO : change to real delta time

		//renderer.appendParticlesToRender(particlePool, cam.getActiveCamera());
        renderer.renderObjects(registry, camEntity.id);
        renderer.renderFrame();
        //gui.render(frameBuffer);
        window.swapBuffers();
        frameBuffer.clear();
    }
    gui.shutdown();
}