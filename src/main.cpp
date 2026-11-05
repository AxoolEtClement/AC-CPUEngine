#define STB_IMAGE_IMPLEMENTATION
#include "window.hpp"
#include "rendering/frameBuffer.hpp"
#include "rendering/renderer.hpp"
#include "gui.hpp"

#include "meshManager.hpp"
#include "rendering/texture.hpp"
#include "rendering/font.hpp"
#include "rendering/material.hpp"
#include "Particle.hpp"
#include "proceduralTerrain.hpp"
#include "moveCamera.hpp"

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

	// entity creation and component addition
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

	// Procedural terrain generation and mesh loading
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

	// Load textures and set up materials
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

	// Particle emitter component setup
    auto& emitter{ registry.addComponent<AC_Engine::ParticleEmitterComponent>(particleEmitterEntity.id,
        simd::Vec3f{ 0.f, 1.f, 0.f }, // velocity
		20.f, // spawn rate
        2.f, // base lifetime
        0.1f, // size
        &particleMat // material
    ) };
    emitter.velocityVariation = simd::Vec3f{ 0.5f, 0.5f, 0.5f };
    emitter.lifeVariation = 0.5f;
    emitter.sizeVariation = 0.05f;
    emitter.startSizeMultiplier = 1.f;
    emitter.endSizeMultiplier = 0.5f;
	emitter.drag = 0.2f;
	emitter.acceleration = simd::Vec3f{ 0.f, -0.5f, 0.f };
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
        cameraMover(window.getWindow(),registry,camEntity.id, 0.016f); // TODO : change to real delta time
        window.swapBuffers();
        frameBuffer.clear();
    }
    gui.shutdown();
}