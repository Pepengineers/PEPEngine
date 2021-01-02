#include "SampleApp.h"

#include "AIComponent.h"
#include "CameraController.h"
#include "GameObject.h"
#include "GDeviceFactory.h"
#include "GModel.h"
#include "GPass.h"
#include "GraphicPSO.h"
#include "MathHelper.h"
#include "ModelRenderer.h"
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

		rtvMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, globalCountFrameResources);

		passes.push_back(std::make_shared<GPass>(device));

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


		rootSignature = std::make_shared<GRootSignature>();

		CD3DX12_DESCRIPTOR_RANGE texParam[6];
		texParam[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2);
		texParam[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 2);
		texParam[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 2);
		texParam[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 2);
		texParam[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 2);
		texParam[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MaxMaterialTexturesMaps, 0, 0); //Material Maps

		rootSignature->AddConstantBufferParameter(0); // ObjectData
		rootSignature->AddDescriptorParameter(&texParam[5], 1, D3D12_SHADER_VISIBILITY_PIXEL); //MaterialsTexture
		rootSignature->AddConstantBufferParameter(1); // CameraData
		rootSignature->AddConstantBufferParameter(2); // WorldData
		rootSignature->AddShaderResourceView(0, 1); // MaterialData
		rootSignature->AddShaderResourceView(1, 1); // LightData		

		rootSignature->AddDescriptorParameter(&texParam[0], 1, D3D12_SHADER_VISIBILITY_PIXEL); //NormalRoughness
		rootSignature->AddDescriptorParameter(&texParam[1], 1, D3D12_SHADER_VISIBILITY_PIXEL); //BaseColorMetalness
		rootSignature->AddDescriptorParameter(&texParam[2], 1, D3D12_SHADER_VISIBILITY_PIXEL); //Position
		rootSignature->AddDescriptorParameter(&texParam[3], 1, D3D12_SHADER_VISIBILITY_PIXEL); //Ambient
		rootSignature->AddDescriptorParameter(&texParam[4], 1, D3D12_SHADER_VISIBILITY_PIXEL); //Depth

		rootSignature->Initialize(device);


		shaders[L"StandardVertex"] = std::move(
			std::make_unique<GShader>(L"Shaders\\Default.hlsl", VertexShader, nullptr, "VS", "vs_5_1"));

		shaders[L"OpaquePixel"] = std::move(
			std::make_unique<GShader>(L"Shaders\\Default.hlsl", PixelShader, nullptr, "PS", "ps_5_1"));

		shaders[L"Debug"] = std::move(
			std::make_unique<GShader>(L"Shaders\\Default.hlsl", PixelShader, nullptr, "PSDebug", "ps_5_1"));

		shaders[L"quadVS"] = std::move(
			std::make_unique<GShader>(L"Shaders\\Quad.hlsl", VertexShader, nullptr, "VS", "vs_5_1"));
		shaders[L"quadPS"] = std::move(
			std::make_unique<GShader>(L"Shaders\\Quad.hlsl", PixelShader, nullptr, "PS", "ps_5_1"));


		for (auto&& sh : shaders)
		{
			sh.second->LoadAndCompile();
		}


		D3D12_GRAPHICS_PIPELINE_STATE_DESC basePsoDesc;

		ZeroMemory(&basePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		basePsoDesc.InputLayout = {VertexInputLayout, static_cast<UINT>(_countof(VertexInputLayout))};
		basePsoDesc.pRootSignature = rootSignature->GetRootSignature().Get();
		basePsoDesc.VS = shaders[L"StandardVertex"]->GetShaderResource();
		basePsoDesc.PS = shaders[L"OpaquePixel"]->GetShaderResource();
		basePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		basePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		basePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		basePsoDesc.SampleMask = UINT_MAX;
		basePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		basePsoDesc.SampleDesc.Count = isM4xMsaa ? 4 : 1;
		basePsoDesc.SampleDesc.Quality = isM4xMsaa ? (m4xMsaaQuality - 1) : 0;
		basePsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		basePsoDesc.NumRenderTargets = 1;
		basePsoDesc.RTVFormats[0] = GetSRGBFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
		basePsoDesc.RTVFormats[1] = GetSRGBFormat(DXGI_FORMAT_UNKNOWN);
		basePsoDesc.RTVFormats[2] = GetSRGBFormat(DXGI_FORMAT_UNKNOWN);

		quadPso = std::make_unique<GraphicPSO>(RenderMode::Quad);
		quadPso->SetPsoDesc(basePsoDesc);
		quadPso->SetShader(shaders[L"quadVS"].get());
		quadPso->SetShader(shaders[L"quadPS"].get());
		quadPso->SetSampleCount(1);
		quadPso->SetSampleQuality(0);
		quadPso->SetDSVFormat(DXGI_FORMAT_UNKNOWN);
		auto depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		depthStencilDesc.DepthEnable = false;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		quadPso->SetDepthStencilState(depthStencilDesc);
		quadPso->Initialize(device);


		debugPso = std::make_shared<GraphicPSO>();
		debugPso->SetPsoDesc(quadPso->GetPsoDescription());
		debugPso->SetShader(shaders[L"StandardVertex"].get());
		debugPso->SetShader(shaders[L"Debug"].get());
		debugPso->Initialize(device);


		auto seamless = std::make_shared<Material>(L"seamless", RenderMode::Opaque);
		auto tex = assetLoader.GetTextureIndex(L"seamless");
		seamless->SetMaterialMap(Material::DiffuseMap, assetLoader.GetTexture(tex));
		tex = assetLoader.GetTextureIndex(L"defaultNormalMap");
		seamless->SetMaterialMap(Material::NormalMap, assetLoader.GetTexture(tex));
		assetLoader.AddMaterial(seamless);


		auto quadRitem = std::make_unique<GameObject>("Quad");
		auto renderer = std::make_shared<ModelRenderer>(GDeviceFactory::GetDevice(), models[L"quad"]);
		models[L"quad"]->SetMeshMaterial(0, assetLoader.GetMaterial(assetLoader.GetMaterialIndex(L"seamless")));
		quadRitem->AddComponent(renderer);
		typedGO[RenderMode::Quad].push_back(quadRitem.get());
		gameObjects.push_back(std::move(quadRitem));


		auto cameraGO = std::make_unique<GameObject>("MainCamera");
		cameraGO->GetTransform()->SetEulerRotate(Vector3(-30, 120, 0));
		cameraGO->GetTransform()->SetPosition(Vector3(30, 20, -130));

		cameraGO->AddComponent(std::make_shared<Camera>(AspectRatio()));
		cameraGO->AddComponent(std::make_shared<CameraController>());
		gameObjects.push_back(std::move(cameraGO));


		for (int i = 0; i < 12; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				auto rModel = std::make_unique<GameObject>();
				auto renderer = std::make_shared<ModelRenderer>(device, models[L"atlas"]);
				rModel->AddComponent(renderer);
				rModel->GetTransform()->SetPosition(
					Vector3::Right * -30 * j + Vector3::Forward * 10 * i);
				typedGO[RenderMode::Opaque].push_back(rModel.get());
				passes[0]->AddTarget(renderer.get());

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

				renderer = std::make_shared<ModelRenderer>(GDeviceFactory::GetDevice(), models[L"cube"]);
				models[L"cube"]->SetMeshMaterial(0, assetLoader.GetMaterial(assetLoader.GetMaterialIndex(L"seamless")));
				sun1->AddComponent(renderer);
				typedGO[RenderMode::Debug].push_back(sun1.get());

				gameObjects.push_back(std::move(sun1));

				gameObjects.push_back(std::move(rModel));
			}
		}


		auto materials = assetLoader.GetMaterials();

		for (auto&& pair : materials)
		{
			pair->InitMaterial(device);
		}


		for (auto&& item : gameObjects)
		{
			auto lightComponent = item->GetComponent<Light>();
			if (lightComponent != nullptr)
			{
				lights.push_back(lightComponent.get());
			}

			auto cam = item->GetComponent<Camera>();
			if (cam != nullptr)
			{
				camera = std::unique_ptr<Camera>(cam.get());
			}
		}

		for (int i = 0; i < globalCountFrameResources; ++i)
		{
			frameResources.push_back(
				std::make_shared<FrameResource>(device, assetLoader.GetMaterials().size(), lights.size()));
		}
	}


	void SampleApp::Update(const GameTimer& gt)
	{
		auto renderQueue = device->GetCommandQueue();

		currentFrameResource = frameResources[currentFrameResourceIndex];

		if (currentFrameResource->FenceValue != 0 && !renderQueue->IsFinish(currentFrameResource->FenceValue))
		{
			renderQueue->WaitForFenceValue(currentFrameResource->FenceValue);
		}

		for (auto&& object : gameObjects)
		{
			object->Update();
		}

		worldData.DeltaTime = gt.DeltaTime();
		worldData.TotalTime = gt.TotalTime();
		worldData.LightsCount = lights.size();

		currentFrameResource->WorldBuffer->CopyData(0, worldData);


		auto currentMaterialBuffer = currentFrameResource->MaterialsBuffer.get();
		for (auto&& material : assetLoader.GetMaterials())
		{
			material->Update();
			auto constantData = material->GetMaterialConstantData();
			currentMaterialBuffer->CopyData(material->GetMaterialIndex(), constantData);
		}

		auto currentLightsBuffer = currentFrameResource->LightsBuffer.get();
		for (int i = 0; i < lights.size(); ++i)
		{
			auto lightData = lights[i]->GetData();
			currentLightsBuffer->CopyData(i, lightData);
		}
	}

	void SampleApp::Draw(const GameTimer& gt)
	{
		if (isResizing) return;

		auto renderQueue = device->GetCommandQueue();
		auto cmdList = renderQueue->GetCommandList();

		cmdList->SetRootSignature(rootSignature.get());

		cmdList->SetRootShaderResourceView(MaterialsBuffer, *currentFrameResource->MaterialsBuffer);
		cmdList->SetRootConstantBufferView(CameraDataBuffer, Camera::mainCamera->GetCameraDataBuffer());
		for (auto&& pass : passes)
		{
			pass->Render(cmdList);
		}

		const auto camera = Camera::mainCamera;

		cmdList->SetViewports(&camera->GetViewPort(), 1);
		cmdList->SetScissorRects(&camera->GetRect(), 1);


		auto gpass = static_cast<GPass*>(passes[0].get());

		cmdList->SetRootSignature(rootSignature.get());
		cmdList->SetRootShaderResourceView(MaterialsBuffer, *currentFrameResource->MaterialsBuffer);
		cmdList->SetRootConstantBufferView(WorldDataBuffer, *currentFrameResource->WorldBuffer);
		cmdList->SetRootConstantBufferView(CameraDataBuffer, Camera::mainCamera->GetCameraDataBuffer());
		cmdList->SetRootShaderResourceView(LightBuffer, *currentFrameResource->LightsBuffer);


		cmdList->SetDescriptorsHeap(&gpass->GetSRV());
		cmdList->SetRootDescriptorTable(NormalMap, &gpass->GetSRV(), GPass::NormalMap);
		cmdList->SetRootDescriptorTable(BaseColorMap, &gpass->GetSRV(), GPass::ColorMap);
		cmdList->SetRootDescriptorTable(PositionMap, &gpass->GetSRV(), GPass::PositionMap);
		cmdList->SetRootDescriptorTable(DepthMap, &gpass->GetSRV(), GPass::DepthMap);


		cmdList->TransitionBarrier(MainWindow->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmdList->FlushResourceBarriers();

		cmdList->SetRenderTargets(1, &rtvMemory, currentFrameResourceIndex);
		cmdList->ClearRenderTarget(&rtvMemory, currentFrameResourceIndex, DirectX::Colors::Black);

		cmdList->SetPipelineState(*quadPso.get());
		for (auto&& object : typedGO[RenderMode::Quad])
			object->Draw(cmdList);


		cmdList->SetPipelineState(*debugPso.get());

		for (int i = 0; i < typedGO[RenderMode::Debug].size(); ++i)
		{
			auto object = typedGO[RenderMode::Debug][i];
			object->Draw(cmdList);
		}

		cmdList->TransitionBarrier(MainWindow->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
		cmdList->FlushResourceBarriers();

		currentFrameResource->FenceValue = renderQueue->ExecuteCommandList(cmdList);

		currentFrameResourceIndex = MainWindow->Present();
	}

	void SampleApp::OnResize()
	{
		D3DApp::OnResize();

		currentFrameResourceIndex = MainWnd()->GetCurrentBackBufferIndex();

		if(Camera::mainCamera)
			Camera::mainCamera->SetAspectRatio(AspectRatio());

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Format = GetSRGBFormat(MainWindow->GetCurrentBackBuffer().GetD3D12ResourceDesc().Format);


		for (int i = 0; i < globalCountFrameResources; ++i)
		{
			MainWindow->GetBackBuffer(i).CreateRenderTargetView(&rtvDesc, &rtvMemory, i);
		}

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

					Vector3 addPos = Vector3::Zero;
					const float speed = 50;

					if (keyboard.KeyIsPressed(VK_LEFT) && keycode == VK_LEFT)
					{
						addPos += Vector3::Left * 15 * timer.DeltaTime();
					}

					if (keyboard.KeyIsPressed(VK_RIGHT) && keycode == VK_RIGHT)
					{
						addPos += Vector3::Right * 15 * timer.DeltaTime();
					}

					if (keyboard.KeyIsPressed(VK_UP) && keycode == VK_UP)
					{
						addPos += Vector3::Up * 15 * timer.DeltaTime();
					}

					if (keyboard.KeyIsPressed(VK_DOWN) && keycode == VK_DOWN)
					{
						addPos += Vector3::Down * 15 * timer.DeltaTime();
					}

					if (keyboard.KeyIsPressed(VK_F2) && keycode == VK_F2)
					{
						addPos += Vector3::Forward * 15 * timer.DeltaTime();
					}

					if (keyboard.KeyIsPressed(VK_F3) && keycode == VK_F3)
					{
						addPos += Vector3::Backward * 15 * timer.DeltaTime();
					}


					for (auto&& object : typedGO[RenderMode::Debug])
					{
						object->GetTransform()->AdjustPosition(addPos * speed);
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
