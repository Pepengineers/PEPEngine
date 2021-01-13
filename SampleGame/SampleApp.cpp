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

namespace SimpleRender
{
	SampleApp::SampleApp(HINSTANCE hInstance) : D3DApp(hInstance)
	                                            
	{
	}

	inline static std::vector<ParticleEmitter*> emitters;

	
	void SampleApp::Pick(const MousePoint& mouse_point)
	{
		auto P = Camera::mainCamera->GetProjectionMatrix();

		// Compute picking ray in view space.
		float vx = (+2.0f * mouse_point.x / MainWindow->GetClientWidth() - 1.0f) / P(0, 0);
		float vy = (-2.0f * mouse_point.y / MainWindow->GetClientHeight() + 1.0f) / P(1, 1);
	
		// Ray definition in view space.
		Vector3 rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		Vector3 rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

		auto  V = Camera::mainCamera->GetViewMatrix();

		V = XMMatrixInverse(&XMMatrixDeterminant(V), V);


		auto pickedGO = level->GetScene()->TryToPickObject(rayOrigin, rayDir, V);

		if(pickedGO != nullptr)
		{
			if(pickedGO->GetName() == "Atlas") scorecount++;
			if (pickedGO->GetName() == "P-body") scorecount += 5;
			killcount++;
			

			auto newGoEmmiter = std::make_shared<GameObject>("Emmiter");
			newGoEmmiter->GetTransform()->SetPosition(pickedGO->GetTransform()->GetWorldPosition());
			auto emitter = std::make_shared<ParticleEmitter>(250);
			newGoEmmiter->AddComponent(emitter);

			level->GetScene()->AddGameObject(std::move(newGoEmmiter));

			emitters.push_back(emitter.get());

			level->GetScene()->RemoveGameObject(pickedGO);
		}
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

		
		 atlas = AssetDatabase::Get<AModel>(L"Atlas");
		if(atlas == nullptr)
		{
			atlas = AssetDatabase::AddModel("Data\\Objects\\Atlas\\Atlas.obj");
		}	
		
		
		level = AssetDatabase::Get<AScene>(L"DefaultScene");
		ShowCursor(false);
		RECT a = { 0,0,MainWindow->GetClientWidth(),MainWindow->GetClientHeight() };
		//ClipCursor(&a);

		
	
	

		if(level == nullptr)
		{
			level = AssetDatabase::CreateAsset<AScene>(L"DefaultScene");

			auto scene = level->GetScene();


			auto cameraGO = std::make_unique<GameObject>("MainCamera");
			cameraGO->GetTransform()->SetEulerRotate(Vector3(0, 120, 0));
			cameraGO->GetTransform()->SetPosition(Vector3(0, 5, 0));

			cameraGO->AddComponent(std::make_shared<Camera>(AspectRatio()));
			cameraGO->AddComponent(std::make_shared<CameraController>());

			scene->AddGameObject(std::move(cameraGO));

			for (int i = 0; i < 6; ++i)
			{
				
				for (int j = 0; j < 3; ++j)
				{
					auto rModel = std::make_unique<GameObject>("Atlas");
					auto renderer = std::make_shared<ModelRenderer>(atlas);
					rModel->AddComponent(renderer);

					rModel->GetTransform()->SetPosition(
						Vector3(Camera::mainCamera->gameObject->GetTransform()->GetWorldPosition().x + (static_cast<float>(rand()) / static_cast<float>((RAND_MAX)) * 2) - 1, 0, Camera::mainCamera->gameObject->GetTransform()->GetWorldPosition().z + (static_cast<float>(rand()) / static_cast<float>((RAND_MAX)) * 2) - 1));


						auto ai = std::make_shared<AIComponent>();
						rModel->AddComponent(ai);

					auto light = std::make_shared<Light>();
					light->Range = 7;
					light->Color = Vector4(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF(), 1);
					light->Intensity =1;
					 light->Type = Point;
					rModel->AddComponent(light);
				
				

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

		pbody = AssetDatabase::Get<AModel>(L"P-Body");
		if (pbody == nullptr)
		{
			pbody = AssetDatabase::AddModel("Data\\Objects\\P-Body\\P-Body.obj");
		}
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
		static float next = false;
	
		uiPass->setCount(scorecount, timeElapsed);
		if((int)killcount % 18 == 0 && killcount > next)
		{
			next = killcount;
			for (int i = 0; i < 6; ++i)
			{
				
				for (int j = 0; j < 3; ++j)
				{
					if(j < killcount / 18)
					{
						auto rModel = std::make_unique<GameObject>("P-body");
						auto renderer = std::make_shared<ModelRenderer>(pbody);

						rModel->AddComponent(renderer);

						rModel->GetTransform()->SetPosition(
							Vector3(Camera::mainCamera->gameObject->GetTransform()->GetWorldPosition().x + (static_cast<float>(rand()) / static_cast<float>((RAND_MAX)) * 2) - 1, 0, Camera::mainCamera->gameObject->GetTransform()->GetWorldPosition().z + (static_cast<float>(rand()) / static_cast<float>((RAND_MAX)) * 2) - 1));

						auto light = std::make_shared<Light>();
						light->Color = Vector4(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF(), 1);
						light->Intensity = 0.9;
						light->Range = 7;
						light->Type = Point;
						rModel->AddComponent(light);
						auto ai = std::make_shared<AIComponent>();
						ai->gotaGoFast();
						rModel->AddComponent(ai);
						level->GetScene()->AddGameObject(std::move(rModel));
					}
					else {
						auto rModel = std::make_unique<GameObject>("Atlas");
						auto renderer = std::make_shared<ModelRenderer>(atlas);

						rModel->AddComponent(renderer);

						rModel->GetTransform()->SetPosition(
							Vector3(Camera::mainCamera->gameObject->GetTransform()->GetWorldPosition().x + (static_cast<float>(rand()) / static_cast<float>((RAND_MAX)) * 2) - 1, 0, Camera::mainCamera->gameObject->GetTransform()->GetWorldPosition().z + (static_cast<float>(rand()) / static_cast<float>((RAND_MAX)) * 2) - 1));

						auto light = std::make_shared<Light>();
						light->Color = Vector4(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF(), 1);
						light->Intensity = 0.9;
						light->Range = 7;
						light->Type = Point;
						rModel->AddComponent(light);
						auto ai = std::make_shared<AIComponent>();
						rModel->AddComponent(ai);
						level->GetScene()->AddGameObject(std::move(rModel));
					}
				}
			}
		}	
		


		static bool picked = false;
		
		if (mouse.IsLeftDown() && timeElapsed < 60)
		{
			if (!picked)
			{
				MousePoint ye = { MainWindow->GetClientWidth()/2 ,MainWindow->GetClientHeight()/2 };
				Pick(ye);
				picked = true;
			}
		}
		else picked = false;


		for(int i =0; i < emitters.size(); ++i)
		{
			auto emitter = emitters[i];

			if(emitter->isWorked)
			{
				if(emitter->ActiveTime > 0.7f)
				{
					emitter->isWorked = false;
				}
			}					
		}		
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
