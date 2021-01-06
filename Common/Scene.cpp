#include "Scene.h"
#include "Light.h"
#include "Renderer.h"
#include "Material.h"
#include "GDeviceFactory.h"


namespace PEPEngine::Common
{
	void Scene::UpdateSceneMaterialBuffer()
	{
		auto currentMaterialBuffer = currentFrameResource->MaterialsBuffer.get();
		auto materialIt = sceneMaterials.begin();
		UINT count = 0;
		while (materialIt != sceneMaterials.end())
		{
			auto material = *materialIt;
			material->Update();

			auto& constantData = material->GetMaterialConstantData();
			currentMaterialBuffer->CopyData(count++, constantData);

			++materialIt;
		}
	}

	void Scene::UpdateSceneLightBuffer()
	{
		UINT count = 0;
		auto currentLightsBuffer = currentFrameResource->LightsBuffer.get();
		auto lightIt = sceneLights.begin();
		while (lightIt != sceneLights.end())
		{
			auto light = *lightIt;
			auto& lightData = light->GetData();
			currentLightsBuffer->CopyData(count++, lightData);

			++lightIt;
		}
	}

	void Scene::Prepare()
	{
		for (int i = 0; i < globalCountFrameResources; ++i)
		{
			frameResources.push_back(
				std::make_shared<FrameResource>(device, sceneMaterials.size(), sceneLights.size()));
		}

		for (auto && material : sceneMaterials)
		{
			material->InitMaterial(device);
		}

		
		currentFrameResource = frameResources[currentFrameResourceIndex];

		currentScene = this;
	}

	void Scene::Update()
	{
		currentFrameResource = frameResources[currentFrameResourceIndex];

		for (auto&& object : objects)
		{
			object->Update();
		}

		/*worldData.DeltaTime = gt.DeltaTime();
		worldData.TotalTime = gt.TotalTime();*/
		worldData.LightsCount = sceneLights.size();

		currentFrameResource->WorldBuffer->CopyData(0, worldData);

		UpdateSceneMaterialBuffer();

		UpdateSceneLightBuffer();

		currentFrameResourceIndex = (currentFrameResourceIndex + 1) % globalCountFrameResources;
	}

	void Scene::Render(RenderMode::Mode mode, std::shared_ptr<GCommandList> cmdList)
	{
		for (auto && renderer : typedRenderer[mode])
		{
			renderer->PopulateDrawCommand(cmdList);
		}
	}

	Scene::Scene() : device(GDeviceFactory::GetDevice())
	{
	}

	Scene::~Scene()
	{
		sceneLights.clear();
		sceneMaterials.clear();
		frameResources.clear();
		objects.clear();
	}

	void Scene::AddGameObject(std::shared_ptr<GameObject> gameObject)
	{
		objects.push_back(gameObject);

		UpdateGameObjects(gameObject);
	}

	void Scene::UpdateGameObjects(std::shared_ptr<GameObject> gameObject)
	{
		if (gameObject == nullptr) return;

		auto light = gameObject->GetComponent<Light>();
		if (light != nullptr)
		{
			auto contains = sceneLights.find(light.get()) != sceneLights.end();

			if (!contains)
			{
				sceneLights.insert(light.get());
			}
		}

		auto renderer = gameObject->GetComponent<Renderer>();
		if (renderer != nullptr)
		{
			for (auto&& material : renderer->GetSharedMaterials())
			{
				if (material != nullptr)
				{
					auto contains = sceneMaterials.find(material.get()) != sceneMaterials.end();

					auto typedContains = typedRenderer[material->GetRenderMode()].find(renderer.get()) != typedRenderer[material->GetRenderMode()].end();
					

					if (!contains)
					{
						sceneMaterials.insert(material.get());
					}

					if(!typedContains)
					{
						typedRenderer[material->GetRenderMode()].insert(renderer.get());
					}
				}
			}
		}
		auto ai = gameObject->GetComponent<AIComponent>();
		if(ai != nullptr)
		{
			ai->addGlobalState(objects);
		}
	}

	FrameResource* Scene::GetCurrentFrameResource() const
	{
		return currentFrameResource.get();
	}
}
