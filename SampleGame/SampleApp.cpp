#include "SampleApp.h"

#include "AIComponent.h"
#include "CameraController.h"
#include "GameObject.h"
#include "GDeviceFactory.h"
#include "GModel.h"
#include "GPass.h"
#include "GraphicPSO.h"
#include "Light.h"
#include "LightPass.h"
#include "MathHelper.h"
#include "ModelRenderer.h"
#include "ShadowPass.h"
#include "SSAOPass.h"
#include "Transform.h"
#include "Window.h"
#include "AssetDatabase.h"
#include "AScene.h"
#include "ParticleEmitter.h"
#include "SpawnController.h"

namespace SimpleRender
{
	SampleApp::SampleApp(HINSTANCE hInstance) : D3DApp(hInstance)
	                                            
	{
	}
	
	bool SampleApp::Initialize()
	{
		device = GDeviceFactory::GetDevice();		
		
		if (!D3DApp::Initialize())
		{
			return false;
		}

		MainWindow->SetVSync(true);		
		
		uiPass = std::make_shared<UILayer>(MainWindow->GetClientWidth(), MainWindow->GetClientHeight(), MainWindow->GetWindowHandle());
		

		pbody = AssetDatabase::Get<AModel>(L"P-Body");
		if (pbody == nullptr)
		{
			pbody = AssetDatabase::AddModel("Data\\Objects\\P-Body\\P-Body.obj");
		}

		level = AssetDatabase::Get<AScene>(L"DefaultScene");

		if(level == nullptr)
		{
			level = AssetDatabase::CreateAsset<AScene>(L"DefaultScene");

			auto scene = level->GetScene();

			auto spawner = std::make_unique<GameObject>("Spawner");
			spawner->AddComponent(std::make_shared<SpawnController>(pbody, Vector3::Left * 120 + Vector3::Up * 75 + Vector3::Left * 500, Vector3::Right * 120 + Vector3::Up * 75 + Vector3::Left * 500, Vector3::One * 0.25f));
			scene->AddGameObject(std::move(spawner));
			

			auto cameraGO = std::make_unique<GameObject>("MainCamera");
			cameraGO->GetTransform()->SetEulerRotate(Vector3(0, 180, 0));
			cameraGO->GetTransform()->SetPosition(Vector3::Forward * 135 + Vector3::Left * 500);

			cameraGO->AddComponent(std::make_shared<Camera>(AspectRatio()));

			scene->AddGameObject(std::move(cameraGO));

			auto sun = std::make_unique<GameObject>("Directional Light");
			auto light = std::make_shared<Light>();
			light->Type = LightType::Directional;
			light->Direction = Vector3::Forward;
			light->Intensity = 0.1f;
			sun->GetTransform()->SetEulerRotate(Vector3(0, 180, 0));
			sun->GetTransform()->SetPosition(Vector3::Forward * 135 + Vector3::Left * 500);
			sun->AddComponent(light);
			scene->AddGameObject(std::move(sun));			
		}
		level->GetScene()->Prepare();
		level->GetScene()->Update();
		AssetDatabase::UpdateAsset(level.get());

		
	}


	void SampleApp::Update(const GameTimer& gt)
	{
		auto renderQueue = device->GetCommandQueue();

		auto values = renderFenceValues[currentFrameResourceIndex];

		if (values != 0 && !renderQueue->IsFinish(values))
		{
			renderQueue->WaitForFenceValue(values);
		}

		level->GetScene()->Update();
		

		static bool spawned = false;

		if(keyboard.KeyIsPressed('P'))
		{
			if(!spawned)
			{
				auto rModel = std::make_shared<GameObject>();
				auto renderer = std::make_shared<ModelRenderer>(pbody);
				rModel->AddComponent(renderer);
				rModel->GetTransform()->SetPosition(Camera::mainCamera->gameObject->GetTransform()->GetWorldPosition());

				level->GetScene()->AddGameObject(std::move(rModel));

				spawned = true;
			}
		}
		else
		{
			spawned = false;
		}

		static bool sceneSaved = false;
		if (keyboard.KeyIsPressed('I'))
		{
			if(!sceneSaved)
			{
				sceneSaved = true;
				AssetDatabase::UpdateAsset(level.get());
			}
		}
		else sceneSaved = false;

	
		
	}

	void SampleApp::Draw(const GameTimer& gt)
	{
		if (isResizing) return;

		auto renderQueue = device->GetCommandQueue();
		
		auto computeQueue = device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		auto cmdList = computeQueue->GetCommandList();

		level->GetScene()->Dispatch(cmdList);

		computeQueue->Wait(renderQueue);
		
		computeFenceValues[currentFrameResourceIndex] = computeQueue->ExecuteCommandList(cmdList);
		

		
		
		cmdList = renderQueue->GetCommandList();

		
		level->GetScene()->Render(cmdList);

		uiPass->Render(cmdList);

		cmdList->TransitionBarrier(MainWindow->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
		cmdList->FlushResourceBarriers();

		renderQueue->Wait(computeQueue);
		renderFenceValues[currentFrameResourceIndex] = renderQueue->ExecuteCommandList(cmdList);

		currentFrameResourceIndex = MainWindow->Present();
	}

	void SampleApp::OnResize()
	{
		D3DApp::OnResize();

		currentFrameResourceIndex = MainWnd()->GetCurrentBackBufferIndex();

		if (Camera::mainCamera)
		{
			Camera::mainCamera->SetAspectRatio(AspectRatio());

			Camera::mainCamera->ChangeRenderSize(MainWindow->GetClientHeight(), MainWindow->GetClientWidth());
		}

		
		if (uiPass)
			uiPass->ChangeRenderTargetSize(MainWindow->GetClientWidth(), MainWindow->GetClientHeight());
		
	}

	LRESULT SampleApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (uiPass)
			uiPass->MsgProc(hwnd, msg, wParam, lParam);		
		
		return D3DApp::MsgProc(hwnd, msg, wParam, lParam);
	}
}
