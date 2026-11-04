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
#include "proceduralTerrain.hpp"
#include <iostream>

#include "ECS/registry.hpp"
#include "ECS/components/cameraComponent.hpp"
#include "ECS/components/meshComponent.hpp"
#include "ECS/components/transformComponent.hpp"
#include "ECS/components/particleEmitterComponent.hpp"

using namespace std::string_literals;

int main() {
    MaterialLibrary materialLibrary{};
    Material mat{};
    Material particleMat{};
    Material terrainMat{};
    AC_Engine::ECS::Registry registry{};
    AC_Engine::MeshManager meshManager{};
    AC_Engine::ParticlePool particlePool{ 1000 };
    AC_Engine::Window window{ 640, 480, "AC-CPUEngine" };
    AC_Engine::FrameBuffer frameBuffer{ 640, 480 };
    AC_Engine::Renderer renderer{ frameBuffer,materialLibrary, meshManager };
    AC_Engine::gui gui{};

    gui.init(window.getWindow());

    AC_Engine::CustomUI customUI{};

    AC_Engine::ECS::Entity camEntity{ registry.createEntity() };
    registry.addComponent<AC_Engine::CameraComponent>(
        camEntity.id, 
        simd::Vec3f{ 1.f, 1.f, 1.f }, 
        simd::Vec3f{ -0.57735f, -0.57735f, -0.57735f }
    );

    AC_Engine::ECS::Entity spot{ registry.createEntity() };
	AC_Engine::ECS::Entity particleEmitterEntity{ registry.createEntity() };
    AC_Engine::ECS::Entity terrainEntity{ registry.createEntity() };
    
    simd::Matrix4x4 rotation{ simd::Matrix4x4::RotationX(0.01f) * simd::Matrix4x4::RotationY(0.01f) };


    AC_Engine::ProceduralTerrain terrain(100, 100);
    terrain.generateTerrain(0.04f, 0.5f, 4, 15.f);
    terrain.generateObj(PROJECT_DIR + "/assets/terrain.obj"s);
    meshManager.addMesh("terrain", PROJECT_DIR + "/assets/terrain.obj"s);
    meshManager.getMesh("terrain")->initNormals();

    meshManager.addMesh("spot", PROJECT_DIR + "/assets/spot.obj"s);
    meshManager.getMesh("spot")->initNormals();

    registry.addComponent<AC_Engine::MeshComponent>(spot.id, meshManager.getMesh("spot"));
    registry.addComponent<AC_Engine::TransformComponent>(spot.id);

    registry.addComponent<AC_Engine::MeshComponent>(terrainEntity.id, meshManager.getMesh("terrain"));
    registry.addComponent<AC_Engine::TransformComponent>(terrainEntity.id);

    Texture texture{ TextureLoader::loadTexture(PROJECT_DIR + "/assets/spot_texture.png"s) };
    Texture particleTexture{ TextureLoader::loadTexture(PROJECT_DIR + "/assets/particle_tex.png"s) };
    Texture terrainTexture{ TextureLoader::loadTexture(PROJECT_DIR + "/assets/terrain.png"s) };

    mat.diffuseMap = std::make_shared<Texture>(texture);
    mat.shininess  = 32.f;
    mat.buildSpecLut();

	particleMat.diffuseMap = std::make_shared<Texture>(particleTexture);
	particleMat.shininess = 32.f;
	particleMat.emissive = { 0.5f, 0.5f, 0.5f };
	particleMat.buildSpecLut();

    terrainMat.diffuseMap = std::make_shared<Texture>(terrainTexture);
    terrainMat.shininess = 100.f;
    terrainMat.emissive = { 0.f, 0.f, 0.f };
    terrainMat.buildSpecLut();

    meshManager.getMesh("spot")->materialId = materialLibrary.add(mat);
    meshManager.getMesh("terrain")->materialId = materialLibrary.add(terrainMat);    

    registry.addComponent<AC_Engine::ParticleEmitterComponent>(particleEmitterEntity.id, 
        simd::Vec3f{ 0.f, 1.f, 0.f }, // velocity
        1.f, // min lifetime
        2.f, // max lifetime
        0.1f, // spawn rate
        &particleMat // material
	);
	registry.addComponent<AC_Engine::TransformComponent>(particleEmitterEntity.id);

    // Main loop
    while (!window.shouldClose()) 
    {
        window.pollEvents();
        customUI.drawText(frameBuffer.getUiPixelsRaw(), window.getWidth(), window.getHeight(),"FPS: 60", 10.0f, 20.0f);
        gui.newFrame();

        frameBuffer.update(window.getWidth(), window.getHeight(), 0x00000000); // Update the framebuffer in case the window size has changed

		particlePool.updateEmitters(registry, 0.016f); // TODO : change to real delta time
		particlePool.updateParticles(0.016f); // TODO : change to real delta time

		renderer.appendParticlesToRender(particlePool, registry, camEntity.id);
        renderer.renderObjects(registry, camEntity.id);
        renderer.renderFrame();
        //gui.render(frameBuffer);
        window.swapBuffers();
        frameBuffer.clear();
    }
    gui.shutdown();
}