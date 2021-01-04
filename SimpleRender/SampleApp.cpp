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

namespace SimpleRender
{
	SampleApp::SampleApp(HINSTANCE hInstance) : D3DApp(hInstance),
	                                            assetLoader(AssetsLoader(GDeviceFactory::GetDevice()))
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
		
		gpass = std::make_shared<GPass>(MainWindow->GetClientWidth(), MainWindow->GetClientHeight());
		ambiantPass = std::make_shared<SSAOPass>( 1920, 1080, *gpass.get());
		shadowPass = std::make_shared<ShadowPass>(2048, 2048);
		lightPass = std::make_shared<LightPass>(MainWindow->GetClientWidth(), MainWindow->GetClientHeight() , *gpass.get(), *ambiantPass.get(), *shadowPass.get());
		uiPass = std::make_shared<UILayer>(MainWindow->GetClientWidth(), MainWindow->GetClientHeight(), MainWindow->GetWindowHandle());
		
		auto copyQueue = device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);

		auto cmdList = copyQueue->GetCommandList();

		auto atlas = assetLoader.CreateModelFromFile(cmdList, "Data\\Objects\\Atlas\\Atlas.obj");
		models[L"atlas"] = std::move(atlas);

		auto quad = assetLoader.GenerateQuad(cmdList);
		models[L"quad"] = std::move(quad);

		auto cube = assetLoader.GenerateSphere(cmdList);
		models[L"cube"] = std::move(cube);

		auto seamlessTex = GTexture::LoadTextureFromFile(L"Data\\Textures\\seamless_grass.jpg", cmdList);
		seamlessTex->SetName(L"seamless");
		assetLoader.AddTexture(seamlessTex);


		std::vector<std::wstring> texNormalNames =
		{
			L"bricksNormalMap",
			L"tileNormalMap",
			L"defaultNormalMap"
		};

		std::vector<std::wstring> texNormalFilenames =
		{
			L"Data\\Textures\\bricks2_nmap.dds",
			L"Data\\Textures\\tile_nmap.dds",
			L"Data\\Textures\\default_nmap.dds"
		};

		for (int i = 0; i < texNormalNames.size(); ++i)
		{
			auto texture = GTexture::LoadTextureFromFile(texNormalFilenames[i], cmdList, TextureUsage::Normalmap);
			texture->SetName(texNormalNames[i]);
			assetLoader.AddTexture(texture);
		}

		copyQueue->ExecuteCommandList(cmdList);
		copyQueue->Flush();


		std::vector<GTexture*> noMipMapsTextures;

		auto allTextures = assetLoader.GetTextures();

		for (auto&& texture : allTextures)
		{
			texture->ClearTrack();

			if (texture->GetD3D12Resource()->GetDesc().Flags != D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
				continue;

			if (!texture->HasMipMap)
			{
				noMipMapsTextures.push_back(texture.get());
			}
		}


		const auto computeQueue = device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		auto computeList = computeQueue->GetCommandList();
		GTexture::GenerateMipMaps(computeList, noMipMapsTextures.data(), noMipMapsTextures.size());
		for (auto&& texture : noMipMapsTextures)
			computeList->TransitionBarrier(texture->GetD3D12Resource(), D3D12_RESOURCE_STATE_COMMON);
		computeList->FlushResourceBarriers();
		computeQueue->WaitForFenceValue(computeQueue->ExecuteCommandList(computeList));		
		
		scene = std::make_shared<Scene>();

		
		auto seamless = std::make_shared<Material>(L"seamless", RenderMode::Opaque);
		auto tex = assetLoader.GetTextureIndex(L"seamless");
		seamless->SetMaterialMap(Material::DiffuseMap, assetLoader.GetTexture(tex));
		tex = assetLoader.GetTextureIndex(L"defaultNormalMap");
		seamless->SetMaterialMap(Material::NormalMap, assetLoader.GetTexture(tex));
		assetLoader.AddMaterial(seamless);


		auto quadRitem = std::make_unique<GameObject>("Quad");
		auto renderer = std::make_shared<ModelRenderer>(GDeviceFactory::GetDevice(), models[L"quad"]);
		
		quadRitem->AddComponent(renderer);

		scene->AddGameObject(std::move(quadRitem));

		auto cameraGO = std::make_unique<GameObject>("MainCamera");
		cameraGO->GetTransform()->SetEulerRotate(Vector3(-30, 120, 0));
		cameraGO->GetTransform()->SetPosition(Vector3(30, 20, -130));

		cameraGO->AddComponent(std::make_shared<Camera>(AspectRatio()));
		cameraGO->AddComponent(std::make_shared<CameraController>());

		scene->AddGameObject(std::move(cameraGO));

		auto sun = std::make_unique<GameObject>("Directional Light");
		auto light = std::make_shared<Light>();
		light->Type = LightType::Directional;
		light->Direction = Vector3( 0.57735f, -0.57735f, 0.57735f );
		light->Intensity = 0.1f;
		sun->AddComponent(light);
		scene->AddGameObject(std::move(sun));

		for (int i = 0; i < 12; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				auto rModel = std::make_unique<GameObject>();
				auto renderer = std::make_shared<ModelRenderer>(device, models[L"atlas"]);
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
	}


	void SampleApp::Update(const GameTimer& gt)
	{
		auto renderQueue = device->GetCommandQueue();

		auto values = fenceValues[currentFrameResourceIndex];

		if (values != 0 && !renderQueue->IsFinish(values))
		{
			renderQueue->WaitForFenceValue(values);
		}

		scene->Update();

		gpass->Update();
		ambiantPass->Update();
		shadowPass->Update();
		lightPass->Update();
		
	}

	void SampleApp::Draw(const GameTimer& gt)
	{
		if (isResizing) return;

		auto renderQueue = device->GetCommandQueue();
		auto cmdList = renderQueue->GetCommandList();
		
		gpass->Render(cmdList);
		ambiantPass->Render(cmdList);
		shadowPass->Render(cmdList);
		lightPass->Render(cmdList);
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

		if(Camera::mainCamera)
			Camera::mainCamera->SetAspectRatio(AspectRatio());

		if (gpass)
			gpass->ChangeRenderTargetSize(MainWindow->GetClientWidth(), MainWindow->GetClientHeight());
		if (lightPass)
			lightPass->ChangeRenderTargetSize(MainWindow->GetClientWidth(), MainWindow->GetClientHeight());
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
