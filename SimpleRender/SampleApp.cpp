#include "SampleApp.h"

#include "AIComponent.h"
#include "CameraController.h"
#include "GameObject.h"
#include "GDeviceFactory.h"
#include "GModel.h"
#include "GPass.h"
#include "GraphicPSO.h"
#include "Light.h"
#include "MathHelper.h"
#include "ModelRenderer.h"
#include "Transform.h"
#include "Window.h"
#include "LightPass.h"
#include "SSAOPass.h"

namespace SimpleRender
{
	SampleApp::SampleApp(HINSTANCE hInstance) : D3DApp(hInstance),
	                                            assetLoader(AssetsLoader(GDeviceFactory::GetDevice()))
	{
	}

	bool SampleApp::Initialize()
	{
		device = GDeviceFactory::GetDevice();		

		auto gPass = std::make_shared<GPass>(device);
		auto ambiantPass = std::make_shared<SSAOPass>(*gPass.get(), 1920, 1080);
		auto lightPass = std::make_shared<LightPass>(*gPass.get(), *ambiantPass.get());
		
		passes.push_back(gPass);
		passes.push_back(ambiantPass);
		passes.push_back(lightPass);

		if (!D3DApp::Initialize())
		{
			return false;
		}

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
				//light->Color = Vector4(1,0,0, 1);
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

		for (auto && render_pass : passes)
		{
			render_pass->Update();
		}
	}

	void SampleApp::Draw(const GameTimer& gt)
	{
		if (isResizing) return;

		auto renderQueue = device->GetCommandQueue();
		auto cmdList = renderQueue->GetCommandList();
		
		for (auto&& pass : passes)
		{
			pass->Render(cmdList);
		}		

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

		for (auto&& pass : passes)
		{
			pass->OnResize();
		}
	}

	LRESULT SampleApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_INPUT:
			{
				UINT dataSize;
				GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &dataSize,
				                sizeof(RAWINPUTHEADER));
				//Need to populate data size first

				if (dataSize > 0)
				{
					std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(dataSize);
					if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawdata.get(), &dataSize,
					                    sizeof(RAWINPUTHEADER)) == dataSize)
					{
						RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());
						if (raw->header.dwType == RIM_TYPEMOUSE)
						{
							mouse.OnMouseMoveRaw(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
						}
					}
				}

				return DefWindowProc(hwnd, msg, wParam, lParam);
			}
			//Mouse Messages
		case WM_MOUSEMOVE:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				mouse.OnMouseMove(x, y);
				return 0;
			}
		case WM_LBUTTONDOWN:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				mouse.OnLeftPressed(x, y);
				return 0;
			}
		case WM_RBUTTONDOWN:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				mouse.OnRightPressed(x, y);
				return 0;
			}
		case WM_MBUTTONDOWN:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				mouse.OnMiddlePressed(x, y);
				return 0;
			}
		case WM_LBUTTONUP:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				mouse.OnLeftReleased(x, y);
				return 0;
			}
		case WM_RBUTTONUP:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				mouse.OnRightReleased(x, y);
				return 0;
			}
		case WM_MBUTTONUP:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				mouse.OnMiddleReleased(x, y);
				return 0;
			}
		case WM_MOUSEWHEEL:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
				{
					mouse.OnWheelUp(x, y);
				}
				else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
				{
					mouse.OnWheelDown(x, y);
				}
				return 0;
			}
		case WM_KEYUP:

			{
				unsigned char keycode = static_cast<unsigned char>(wParam);
				keyboard.OnKeyReleased(keycode);


				return 0;
			}
		case WM_KEYDOWN:
			{
				{
					unsigned char keycode = static_cast<unsigned char>(wParam);
					if (keyboard.IsKeysAutoRepeat())
					{
						keyboard.OnKeyPressed(keycode);
					}
					else
					{
						const bool wasPressed = lParam & 0x40000000;
						if (!wasPressed)
						{
							keyboard.OnKeyPressed(keycode);
						}
					}					
				}
			}

		case WM_CHAR:
			{
				unsigned char ch = static_cast<unsigned char>(wParam);
				if (keyboard.IsCharsAutoRepeat())
				{
					keyboard.OnChar(ch);
				}
				else
				{
					const bool wasPressed = lParam & 0x40000000;
					if (!wasPressed)
					{
						keyboard.OnChar(ch);
					}
				}
				return 0;
			}
		}

		return D3DApp::MsgProc(hwnd, msg, wParam, lParam);
	}
}
