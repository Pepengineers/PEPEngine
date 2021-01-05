#include "Scene.h"
#include "Light.h"
#include "Renderer.h"
#include "Material.h"
#include "GDeviceFactory.h"
#include "Camera.h"

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


	void Scene::Update()
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
		for (auto&& renderer : typedRenderers[mode])
		{
			renderer->PopulateDrawCommand(cmdList);
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

		auto renderer = gameObject->GetComponent<Renderer>();
		if (renderer != nullptr)
		{
			for (auto&& material : renderer->GetSharedMaterials())
			{
				if (material != nullptr)
				{
					const auto it = typedMaterials[material->GetRenderMode()].insert(material.get());

					if (it.second)
					{
						material->InitMaterial(device);
						material->SetMaterialIndex(TotalMaterialCount++);
					}


					typedRenderers[material->GetRenderMode()].push_back(renderer.get());
				}
			}
		}

		auto camera = gameObject->GetComponent<Camera>();
		if (camera != nullptr)
		{
			cameras.insert(camera.get());
		}
	}

	FrameResource* Scene::GetCurrentFrameResource() const
	{
		return currentFrameResource.get();
	}
}
