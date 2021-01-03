#pragma once
#include "GameObject.h"
#include "FrameResource.h"
#include "GraphicPSO.h"

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

		custom_set<Light*> sceneLights = MemoryAllocator::CreateSet<Light*>();
		custom_set<Material*> sceneMaterials = MemoryAllocator::CreateSet<Material*>();
		
		std::set<Renderer*> typedRenderer[RenderMode::Count];

		
		WorldData worldData = {};

		std::shared_ptr<FrameResource> currentFrameResource = nullptr;
		UINT currentFrameResourceIndex = 0;


		void UpdateSceneMaterialBuffer();

		void UpdateSceneLightBuffer();

	public:

		inline static Scene* currentScene = nullptr;

		FrameResource* GetCurrentFrameResource() const;

		void Prepare();

		void Update();

		void Render(RenderMode::Mode mode, std::shared_ptr<GCommandList> cmdList);

		Scene();

		~Scene();

		void AddGameObject(std::shared_ptr<GameObject> gameObject);

		void UpdateGameObjects(std::shared_ptr<GameObject> gameObject);
	};

}

