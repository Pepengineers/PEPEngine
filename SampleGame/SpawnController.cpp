#include "SpawnController.h"
#include "AssetDatabase.h"
#include "d3dApp.h"
#include "MathHelper.h"
#include "GameObject.h"
#include "ModelRenderer.h"
#include "Transform.h"
#include "Scene.h"
#include "Window.h"
#include "imgui.h"

static inline std::vector<std::shared_ptr<GameObject>> spawnedObjects;

SpawnController::SpawnController(const std::shared_ptr<AModel> spawnModel, const Vector3 lp, const Vector3 rp,
                                 const Vector3 modelScale): leftPoint(lp), rightPoint(rp), spawnModelScale(modelScale),
                                                            spawnObjectModel(spawnModel)
{
}

void SpawnController::OnGUI()
{
	// Exceptionally add an extra assert here for people confused about initial Dear ImGui setup
	// Most ImGui functions would normally just crash if the context is missing.
	IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");
	
	// We specify a default position/size in case there's no data in the .ini file.
	// We only do it to make the demo applications a little more welcoming, but typically this isn't required.
	ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(main_viewport->GetWorkPos().x, main_viewport->GetWorkPos().y ), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(250, 55), ImGuiCond_Always);

	// Main body of the Demo window starts here.
	if (!ImGui::Begin("Show Game Stats",nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize  | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}
	
	ImGui::Text("Total killed objects: (%d)", count);
	
	ImGui::End();
}

void SpawnController::Update()
{
	auto timer = D3DApp::GetApp().GetTimer();

	currentTime += timer->DeltaTime();
	if(currentTime >= tickTime)
	{
		for (int i = 0; i < spawnModelPerSecondCount; ++i)
		{
			auto x = MathHelper::RandF(leftPoint.x, rightPoint.x);
			auto y = MathHelper::RandF(leftPoint.y, rightPoint.y);
			auto z = MathHelper::RandF(leftPoint.z, rightPoint.z);

			auto go = std::make_shared<GameObject>();
			go->GetTransform()->SetPosition(Vector3(x,y,z));
			go->AddComponent(std::make_shared<ModelRenderer>(spawnObjectModel));

			Scene::currentScene->AddGameObject(go);

			spawnedObjects.push_back(std::move(go));		
			
		}		
		currentTime = 0;
	}

	auto mouse = D3DApp::GetApp().GetMouse();
	
	static bool isClicked = false;

	if (mouse->IsLeftDown())
	{
		if (!isClicked)
		{
			auto mousePos = mouse->GetPos();
			auto MainWindow = D3DApp::GetApp().GetMainWindow();
			auto P = Camera::mainCamera->GetProjectionMatrix();

			float vx = (+2.0f * mousePos.x / MainWindow->GetClientWidth() - 1.0f) / P(0, 0);
			float vy = (-2.0f * mousePos.y / MainWindow->GetClientHeight() + 1.0f) / P(1, 1);

			Vector3 originRay(0.0f, 0.0f, 0.0f);
			Vector3 dirRay(vx, vy, 1.0f);

			auto  V = Camera::mainCamera->GetViewMatrix();

			V = XMMatrixInverse(&XMMatrixDeterminant(V), V);

			for (auto it = spawnedObjects.begin(); it != spawnedObjects.end(); ++it)
			{
				auto object = *it;

				auto renderer = object->GetComponent<Renderer>();

				auto aModel = renderer->GetModel();

				auto model = aModel->GetGModel();

				auto W = (renderer->gameObject->GetTransform()->GetWorldMatrix());

				auto invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

				// Tranform ray to vi space of Mesh.
				Matrix toLocal = XMMatrixMultiply(V, invWorld);

				auto rayOrigin = XMVector3TransformCoord(originRay, toLocal);
				auto rayDir = XMVector3TransformNormal(dirRay, toLocal);
				rayDir = XMVector3Normalize(rayDir);

				dirRay = XMVector3Normalize(dirRay);

				static float tmin = 0.0f;
				if (model->Bounds.Intersects(rayOrigin, rayDir, tmin))
				{
					Scene::currentScene->RemoveGameObject(object);
					spawnedObjects.erase(it);
					count++;

					spawnModelPerSecondCount = (count / 50) + 1;
					
					break;
				}
			}

			isClicked = true;
		}
	}
	else isClicked = false;

	for (auto && object : spawnedObjects)
	{
		object->GetTransform()->AdjustPosition(Vector3::Down * 10.0f * timer->DeltaTime());
	}
}

void SpawnController::Serialize(json& j)
{
	j["Type"] = ComponentID;


	auto jLPOINT = json();
	jLPOINT["x"] = leftPoint.x;
	jLPOINT["y"] = leftPoint.y;
	jLPOINT["z"] = leftPoint.z;

	auto jRPOINT = json();
	jRPOINT["x"] = rightPoint.x;
	jRPOINT["y"] = rightPoint.y;
	jRPOINT["z"] = rightPoint.z;

	auto jModelScale = json();
	jModelScale["x"] = spawnModelScale.x;
	jModelScale["y"] = spawnModelScale.y;
	jModelScale["z"] = spawnModelScale.z;


	j["RightPoint"] = jRPOINT;
	j["LeftPoint"] = jLPOINT;
	j["ModelScale"] = jModelScale;

	assert(spawnObjectModel);

	j["ModelID"] = spawnObjectModel->GetID();
}

void SpawnController::Deserialize(json& j)
{
	float x, y, z;

	auto rPoint = j["RightPoint"];

	Asset::TryReadVariable(rPoint, "x", &x);
	Asset::TryReadVariable(rPoint, "y", &y);
	Asset::TryReadVariable(rPoint, "z", &z);
	rightPoint = Vector3(x, y, z);

	auto lPoint = j["LeftPoint"];
	Asset::TryReadVariable(lPoint, "x", &x);
	Asset::TryReadVariable(lPoint, "y", &y);
	Asset::TryReadVariable(lPoint, "z", &z);

	leftPoint = Vector3(x, y, z);

	lPoint = j["ModelScale"];
	Asset::TryReadVariable(lPoint, "x", &x);
	Asset::TryReadVariable(lPoint, "y", &y);
	Asset::TryReadVariable(lPoint, "z", &z);

	spawnModelScale = Vector3(x, y, z);

	UINT64 id;
	Asset::TryReadVariable(j, "ModelID", &id);

	spawnObjectModel = AssetDatabase::FindAssetByID<AModel>(id);

	assert(spawnObjectModel);
}
