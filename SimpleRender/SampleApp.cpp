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

		
		

		auto atlas = AssetDatabase::Get<AModel>(L"Atlas");
		if(atlas == nullptr)
		{
			atlas = AssetDatabase::AddModel("Data\\Objects\\Atlas\\Atlas.obj");
		}	
		

		level = AssetDatabase::Get<AScene>(L"DefaultScene");

		if(level == nullptr)
		{
			level = AssetDatabase::CreateAsset<AScene>(L"DefaultScene");

			auto scene = level->GetScene();


			auto cameraGO = std::make_unique<GameObject>("MainCamera");
			cameraGO->GetTransform()->SetEulerRotate(Vector3(-30, 120, 0));
			cameraGO->GetTransform()->SetPosition(Vector3(30, 20, -130));

			cameraGO->AddComponent(std::make_shared<Camera>(AspectRatio()));
			cameraGO->AddComponent(std::make_shared<CameraController>());

			scene->AddGameObject(std::move(cameraGO));

			auto sun = std::make_unique<GameObject>("Directional Light");
			auto light = std::make_shared<Light>();
			light->Type = LightType::Directional;
			light->Direction = Vector3(0.57735f, -0.57735f, 0.57735f);
			light->Intensity = 0.1f;
			sun->AddComponent(light);
			scene->AddGameObject(std::move(sun));

			for (int i = 0; i < 12; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					auto rModel = std::make_unique<GameObject>();
					auto renderer = std::make_shared<ModelRenderer>(atlas);
					rModel->AddComponent(renderer);
					rModel->GetTransform()->SetPosition(
						Vector3::Right * -30 * j + Vector3::Forward * 10 * i);

					/*	auto ai = std::make_shared<AIComponent>();
						rModel->AddComponent(ai);*/


					auto pos = rModel->GetTransform()->GetWorldPosition() + (Vector3::Up * 1 * 10);

					auto sun1 = std::make_unique<GameObject>("Light");
					sun1->GetTransform()->SetPosition(pos);
					auto light = std::make_shared<Light>();
					light->Color = Vector4(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF(), 1);
					light->Intensity = 1;
					(i + j) % 2 == 1 ? light->Type = Spot : light->Type = Point;

					sun1->AddComponent(light);

					scene->AddGameObject(std::move(sun1));

					scene->AddGameObject(std::move(rModel));
				}
			}
			
			scene->Prepare();
			scene->Update();
			
			AssetDatabase::UpdateAsset(level);
		}
		else
		{
			auto scene = level->GetScene();
			scene->Prepare();
			scene->Update();
		}		
	}


	void SampleApp::Update(const GameTimer& gt)
	{
		auto renderQueue = device->GetCommandQueue();

		auto values = fenceValues[currentFrameResourceIndex];

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
			/*	auto renderer = std::make_shared<ModelRenderer>(models[L"PBody"]);
				rModel->AddComponent(renderer);*/
				rModel->GetTransform()->SetPosition(Camera::mainCamera->gameObject->GetTransform()->GetWorldPosition());

				level->GetScene()->AddGameObject(std::move(rModel));

				spawned = true;
			}
		}
		else
		{
			spawned = false;
		}
		
	}

	void SampleApp::Draw(const GameTimer& gt)
	{
		if (isResizing) return;

		auto renderQueue = device->GetCommandQueue();
		auto cmdList = renderQueue->GetCommandList();

		
		level->GetScene()->Render(cmdList);

		uiPass->Render(cmdList);

		cmdList->TransitionBarrier(MainWindow->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
		cmdList->FlushResourceBarriers();

		fenceValues[currentFrameResourceIndex] = renderQueue->ExecuteCommandList(cmdList);

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
