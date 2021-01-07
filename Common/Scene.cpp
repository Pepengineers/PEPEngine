#include "Scene.h"

#include "AssetDatabase.h"
#include "Light.h"
#include "Renderer.h"
#include "Material.h"
#include "GDeviceFactory.h"
#include "Camera.h"
#include "AModel.h"
#include "AMaterial.h"


namespace PEPEngine::Common
{
	void Scene::UpdateSceneMaterialBuffer()
	{
		auto currentMaterialBuffer = currentFrameResource->MaterialsBuffer.get();
		UINT count = 0;
		for (auto&& materials : typedMaterials)
		{
			for (auto&& material : materials)
			{
				assert(count < TotalMaterialCount);

				material->Update();
				auto& constantData = material->GetMaterialConstantData();
				currentMaterialBuffer->CopyData(count++, constantData);
			}
		}
	}

	void Scene::UpdateSceneLightBuffer()
	{
		UINT count = 0;
		auto currentLightsBuffer = currentFrameResource->LightsBuffer.get();

		for (auto&& light : sceneLights)
		{
			auto& lightData = light->GetData();
			currentLightsBuffer->CopyData(count++, lightData);
		}
	}

	void Scene::Prepare()
	{
		currentFrameResource = frameResources[currentFrameResourceIndex];

		currentScene = this;
	}


	void Scene::SpawnNewGO()
	{
		if (addedGameObjects.Size() > 0)
		{
			std::shared_ptr<GameObject> go;
			while (addedGameObjects.TryPop(go))
			{
				objects.push_back(go);
				UpdateGameObjects(go);
			}

			if (currentFrameResource->LightsBuffer->GetElementCount() < sceneLights.size())
			{
				for (int i = 0; i < globalCountFrameResources; ++i)
				{
					frameResources[i]->UpdateLightBufferSize(sceneLights.size() + 5);
				}
			}

			if (currentFrameResource->MaterialsBuffer->GetElementCount() < TotalMaterialCount)
			{
				for (int i = 0; i < globalCountFrameResources; ++i)
				{
					frameResources[i]->UpdateMaterialBufferSizer(TotalMaterialCount + 5);
				}
			}
		}
	}

	void Scene::Update()
	{
		SpawnNewGO();

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

	void Scene::Render(std::shared_ptr<GCommandList> cmdList)
	{
		for (auto camera : cameras)
		{
			camera->Render(cmdList);
		}
	}

	void Scene::RenderTypedObjects(RenderMode::Mode mode, std::shared_ptr<GCommandList> cmdList)
	{
		auto materials = typedMaterials[mode];

		for (auto&& material : materials)
		{
			auto it = typedRenderers.find(material);
			if(it != typedRenderers.end())
			{
				material->SetRenderMaterialData(cmdList);
				for (auto && renderPair : it->second)
				{
					for (auto && meshIndex : renderPair.second)
					{
						renderPair.first->PopulateDrawCommand(cmdList, meshIndex);
					}
				}
			}
		}
	}

	Scene::Scene() : device(GDeviceFactory::GetDevice())
	{
		for (int i = 0; i < globalCountFrameResources; ++i)
		{
			frameResources.push_back(
				std::make_shared<FrameResource>(device, 5, 5));
		}
	}


	void Scene::AddGameObject(std::shared_ptr<GameObject> gameObject)
	{
		addedGameObjects.Push(gameObject);
	}

	void Scene::UpdateGameObjects(std::shared_ptr<GameObject> gameObject)
	{
		if (gameObject == nullptr) return;


		auto light = gameObject->GetComponent<Light>();
		if (light != nullptr)
		{
			sceneLights.insert(light.get());
		}
		auto ai = gameObject->GetComponent<AIComponent>();
		if(ai != nullptr)
		{
			ai->addGlobalState(objects);
		}

		auto renderer = gameObject->GetComponent<Renderer>();
		if (renderer != nullptr)
		{
			auto sharedMaterials = renderer->GetSharedMaterials();

			for (int i = 0; i < sharedMaterials.size(); ++i)
			{
				auto material = sharedMaterials[i];
				
				if (material != nullptr)
				{
					auto gMaterial = material->GetMaterial();
					
					const auto it = typedMaterials[gMaterial->GetRenderMode()].insert(gMaterial.get());

					if (it.second)
					{
						gMaterial->Init(device);
						gMaterial->SetMaterialIndex(TotalMaterialCount++);
					}

					typedRenderers[gMaterial.get()][renderer.get()].push_back(i);
				}
			}			
		}

		const auto camera = gameObject->GetComponent<Camera>();
		if (camera != nullptr)
		{
			cameras.insert(camera.get());
		}
	}

	void Scene::Serialize(json& j)
	{
		j["GameObjectsCount"] = objects.size();

		auto array = json::array();

		for (auto && object : objects)
		{
			json element;
			object->Serialize(element);
			array.push_back(element);
		}
		j["GameObjects"] = array;		
	}

	void Scene::Deserialize(json& j)
	{
		UINT count;
		assert(Asset::TryReadVariable<UINT>(j, "GameObjectsCount", &count));

		json array = j["GameObjects"];
		
		for (int i = 0; i < count; ++i)
		{
			auto GO = std::make_shared<GameObject>();
			GO->Deserialize(array[i]);
			AddGameObject(GO);
		}		
	}

	FrameResource* Scene::GetCurrentFrameResource() const
	{
		return currentFrameResource.get();
	}
}
