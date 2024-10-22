#include "Scene.h"

#include "AssetDatabase.h"
#include "Light.h"
#include "Renderer.h"
#include "Material.h"
#include "GDeviceFactory.h"
#include "Camera.h"
#include "AModel.h"
#include "AMaterial.h"
#include "Emitter.h"


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
				UpdateGameObject(go);
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

	void Scene::Dispatch(std::shared_ptr<GCommandList> cmdList)
	{
		for (auto&& emitter : emitters)
		{
			emitter->Dispatch(cmdList);
		}
	}

	void Scene::UpdateGameObject(std::shared_ptr<GameObject> gameObject)
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
		auto emitter = gameObject->GetComponent<Emitter>();
		if (emitter != nullptr)
		{
			emitters.insert(emitter.get());
		}

		const auto camera = gameObject->GetComponent<Camera>();
		if (camera != nullptr)
		{
			cameras.insert(camera.get());
		}

		auto ai = gameObject->GetComponent<AIComponent>();
		if (ai != nullptr)
		{
			ai->addGlobalState(&objects);
		}
	}
	

	void Scene::DeleteOldGO()
	{
		std::shared_ptr<GameObject> go = nullptr;

		while (gameObjectsToRemove.TryPop(go))
		{
			auto pCamera = go->GetComponent<Camera>();
			auto pRenderer = go->GetComponent<Renderer>();
			auto pLight = go->GetComponent<Light>();


			if (pCamera != nullptr && cameras.find(pCamera.get()) != cameras.end())
			{
				cameras.erase(pCamera.get());
			}

			auto emitter = go->GetComponent<Emitter>();
			if (emitter != nullptr && emitters.find(emitter.get()) != emitters.end())
			{
				emitters.insert(emitter.get());
			}

			if (pRenderer != nullptr)
			{
				auto sharedMaterials = pRenderer->GetSharedMaterials();
				for (uint32_t i = 0u; i < sharedMaterials.size(); ++i)
				{
					auto sharedMaterial = sharedMaterials[i];
					if (sharedMaterial != nullptr)
					{
						auto material = sharedMaterial->GetMaterial();

						const std::set<Material*>::iterator it = typedMaterials[material->GetRenderMode()].find(
							material.get());

						if (it != typedMaterials[material->GetRenderMode()].end())
						{
							typedRenderers[material.get()].erase(pRenderer.get());

							if(typedRenderers[material.get()].size() <= 0)
							{
								typedRenderers.erase(material.get());
								typedMaterials[material->GetRenderMode()].erase(material.get());
							}
						}
					}
				}
			}

			if (pLight != nullptr && sceneLights.find(pLight.get()) != sceneLights.end())
			{
				sceneLights.erase(pLight.get());
			}

			const auto it = std::find(objects.begin(), objects.end(), go);

			if (it != objects.end())
			{
				objects.erase(it);
			}
		}
	}

	void Scene::Update()
	{
		DeleteOldGO();
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

	void Scene::RenderParticle(std::shared_ptr<GCommandList> cmdList)
	{
		for (auto&& emitter : emitters)
		{
			emitter->PopulateDrawCommand(cmdList);
		}
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
			if (it != typedRenderers.end())
			{
				material->SetRenderMaterialData(cmdList);
				for (auto&& renderPair : it->second)
				{
					for (auto&& meshIndex : renderPair.second)
					{
						renderPair.first->PopulateDrawCommand(cmdList, meshIndex);
					}
				}
			}
		}
	}

	std::shared_ptr<GameObject> Scene::TryToPickObject(const Vector3& originRay, const Vector3& dirRay,
	                                                   const Matrix& invertViewMatrix)
	{
		for (auto && typedPair : typedRenderers)
		{
			for (auto&& typedRederers : typedPair.second)
			{
				auto renderer = typedRederers.first;
				auto aModel = renderer->GetModel();

				auto model = aModel->GetGModel();

				auto W = (renderer->gameObject->GetTransform()->GetWorldMatrix());
				
				auto invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

				// Tranform ray to vi space of Mesh.
				XMMATRIX toLocal = XMMatrixMultiply(invertViewMatrix, invWorld);

				auto rayOrigin = XMVector3TransformCoord(originRay, toLocal);
				auto rayDir = XMVector3TransformNormal(dirRay, toLocal);

				rayDir = XMVector3Normalize(rayDir);

				static float tmin = 0.0f;
				if (model->Bounds.Intersects(rayOrigin, rayDir, tmin))
				{
					for (auto && object : objects)
					{
						if (object.get() == renderer->gameObject)
							return object;
					}
				}				
			}
		}

		return nullptr;
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

	

	void Scene::RemoveGameObject(std::shared_ptr<GameObject> gameObject)
	{
		if (gameObject == nullptr)
		{
			return;
		}

		gameObjectsToRemove.Push(gameObject);
	}

	void Scene::Serialize(json& j)
	{
		j["GameObjectsCount"] = objects.size();

		auto array = json::array();

		for (auto&& object : objects)
		{
			json element;
			object->Serialize(element);
			array.push_back(element);
		}
		j["GameObjects"] = array;
	}

	void Scene::Deserialize(json& j)
	{
		UINT count = 0;
		(Asset::TryReadVariable<UINT>(j, "GameObjectsCount", &count));

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
