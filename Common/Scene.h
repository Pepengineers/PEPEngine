#pragma once
#include "GameObject.h"
#include "FrameResource.h"
#include "GraphicPSO.h"
#include "AIComponent.h"
#include "LockThreadQueue.h"

namespace PEPEngine::Common
{
	using namespace Utils;

	class Light;
	class Material;
	class Camera;
	class Emitter;

	class Scene
	{
		friend class AScene;
		
		custom_vector<std::shared_ptr<GameObject>> objects = MemoryAllocator::CreateVector<std::shared_ptr<GameObject>
		>();

		custom_vector<std::shared_ptr<FrameResource>> frameResources = MemoryAllocator::CreateVector<std::shared_ptr<
			FrameResource>>();

		std::shared_ptr<GDevice> device = nullptr;

		UINT TotalMaterialCount = 0;


		std::set<Material*> typedMaterials[RenderMode::Count];

		std::unordered_map<Material*, std::unordered_map<Renderer*, std::vector<UINT>>> typedRenderers{};

		std::set<Emitter*> emitters;
		
		std::set<Camera*> cameras{};

		WorldData worldData = {};

		std::shared_ptr<FrameResource> currentFrameResource = nullptr;
		UINT currentFrameResourceIndex = 0;

		LockThreadQueue<std::shared_ptr<GameObject>> addedGameObjects;


		void UpdateSceneMaterialBuffer();

		void UpdateSceneLightBuffer();

	public:


		custom_set<Light*> sceneLights = MemoryAllocator::CreateSet<Light*>();

		inline static Scene* currentScene = nullptr;

		FrameResource* GetCurrentFrameResource() const;

		void Prepare();
		void SpawnNewGO();

		void Dispatch(std::shared_ptr<GCommandList> cmdList);
		
		void Update();

		void Render(std::shared_ptr<GCommandList> cmdList);
		void RenderParticle(std::shared_ptr<GCommandList> cmdList);

		void RenderTypedObjects(RenderMode::Mode mode, std::shared_ptr<GCommandList> cmdList);

		Scene();

		~Scene() = default;

		void AddGameObject(std::shared_ptr<GameObject> gameObject);

		void UpdateGameObjects(std::shared_ptr<GameObject> gameObject);

		void Serialize(json& j) ;
		void Deserialize(json& j) ;
	};
}
