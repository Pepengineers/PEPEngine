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
	public:
		std::vector<std::shared_ptr<GameObject>> objects;
	private:

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
		LockThreadQueue < std::shared_ptr<GameObject>> gameObjectsToRemove;

		void UpdateSceneMaterialBuffer();

		void UpdateSceneLightBuffer();

	public:


		custom_set<Light*> sceneLights = MemoryAllocator::CreateSet<Light*>();

		inline static Scene* currentScene = nullptr;

		FrameResource* GetCurrentFrameResource() const;

		void Prepare();

		void DeleteOldGO();
		void SpawnNewGO();

		void Dispatch(std::shared_ptr<GCommandList> cmdList);
		
		void Update();

		void Render(std::shared_ptr<GCommandList> cmdList);
		void RenderParticle(std::shared_ptr<GCommandList> cmdList);

		void RenderTypedObjects(RenderMode::Mode mode, std::shared_ptr<GCommandList> cmdList);
		std::shared_ptr<GameObject> TryToPickObject(const Vector3& originRay, const Vector3& dirRay,
		                                            const Matrix& invertViewMatrix);

		Scene();

		~Scene() = default;

		void AddGameObject(std::shared_ptr<GameObject> gameObject);

		void UpdateGameObject(std::shared_ptr<GameObject> gameObject);

		void RemoveGameObject(std::shared_ptr<GameObject> gameObject);
		void DrawGUI();

		void Serialize(json& j) ;
		void Deserialize(json& j) ;
	};
}
