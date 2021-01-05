#pragma once
#include "GameObject.h"
#include "FrameResource.h"
#include "GraphicPSO.h"
#include "LockThreadQueue.h"

namespace PEPEngine::Common
{
	using namespace Utils;
	
	class Light;
	class Material;
	
	class Scene
	{
		custom_vector<std::shared_ptr<GameObject>> objects = MemoryAllocator::CreateVector<std::shared_ptr<GameObject>>();

		custom_vector<std::shared_ptr<FrameResource>> frameResources = MemoryAllocator::CreateVector<std::shared_ptr<FrameResource>>();

		std::shared_ptr<GDevice> device = nullptr;

		UINT TotalMaterialCount = 0;


		std::set<Material*> typedMaterials[RenderMode::Count];
		std::vector<Renderer*> typedRenderers[RenderMode::Count];
		
		
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

		void Update();

		void Render(RenderMode::Mode mode, std::shared_ptr<GCommandList> cmdList);

		Scene();

		~Scene() = default;

		void AddGameObject(std::shared_ptr<GameObject> gameObject);

		void UpdateGameObjects(std::shared_ptr<GameObject> gameObject);
	};

}

