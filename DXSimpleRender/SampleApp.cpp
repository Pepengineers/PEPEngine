#include "SampleApp.h"


#include "CameraController.h"
#include "GameObject.h"
#include "GDeviceFactory.h"
#include "GraphicPSO.h"
#include "ModelRenderer.h"
#include "Transform.h"
#include "Window.h"


SampleApp::SampleApp(HINSTANCE hInstance): D3DApp(hInstance), assetLoader(AssetsLoader(GDeviceFactory::GetDevice()))
{
}

bool SampleApp::Initialize()
{
	device = GDeviceFactory::GetDevice();

	dsvMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	rtvMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, globalCountFrameResources);
	defferedRTVMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV,  4);
	srvMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, assetLoader.GetLoadTexturesCount());
	defferedSRVMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4);

	if (!D3DApp::Initialize())
	{
		return false;
	}

	auto copyQueue = device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);

	auto cmdList = copyQueue->GetCommandList();

	auto atlas = assetLoader.CreateModelFromFile(cmdList, "Data\\Objects\\Atlas\\Atlas.obj");
	models[L"atlas"] = std::move(atlas);

	copyQueue->ExecuteCommandList(cmdList);
	copyQueue->Flush();


	rootSignature = std::make_shared<GRootSignature>();
	

	rootSignature->AddConstantBufferParameter(0);
	rootSignature->AddConstantBufferParameter(1);
	rootSignature->AddShaderResourceView(0, 1);
	rootSignature->AddShaderResourceView(0, 2);

	rootSignature->Initialize(device);
		

	shaders[L"StandardVertex"] = std::move(
		std::make_unique<GShader>(L"Shaders\\Default.hlsl", VertexShader, nullptr, "VS", "vs_5_1"));

	shaders[L"OpaquePixel"] = std::move(
		std::make_unique<GShader>(L"Shaders\\Default.hlsl", PixelShader, nullptr, "PS", "ps_5_1"));

	for (auto&& sh : shaders)
	{
		sh.second->LoadAndCompile();
	}

	defaultInputLayout =
	{
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC basePsoDesc;

	ZeroMemory(&basePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	basePsoDesc.InputLayout = {defaultInputLayout.data(), static_cast<UINT>(defaultInputLayout.size())};
	basePsoDesc.pRootSignature = rootSignature->GetRootSignature().Get();
	basePsoDesc.VS = shaders[L"StandardVertex"]->GetShaderResource();
	basePsoDesc.PS = shaders[L"OpaquePixel"]->GetShaderResource();
	basePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	basePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	basePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	basePsoDesc.SampleMask = UINT_MAX;
	basePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	basePsoDesc.NumRenderTargets = 4;
	basePsoDesc.RTVFormats[0] = GetSRGBFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	basePsoDesc.RTVFormats[1] = GetSRGBFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	basePsoDesc.RTVFormats[2] = GetSRGBFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	basePsoDesc.RTVFormats[3] = GetSRGBFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
	basePsoDesc.SampleDesc.Count = isM4xMsaa ? 4 : 1;
	basePsoDesc.SampleDesc.Quality = isM4xMsaa ? (m4xMsaaQuality - 1) : 0;
	basePsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	opaque = std::make_shared<GraphicPSO>();
	opaque->SetPsoDesc(basePsoDesc);
	opaque->Initialize(device);

	auto sun1 = std::make_unique<GameObject>("Directional Light");
	auto light = std::make_shared<Light>();
	sun1->AddComponent(light);
	gameObjects.push_back(std::move(sun1));


	auto cameraGO = std::make_unique<GameObject>("MainCamera");
	cameraGO->GetTransform()->SetEulerRotate(Vector3(-30, 270, 0));
	cameraGO->GetTransform()->SetPosition(Vector3(-1000, 190, -32));
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
				Vector3::Right * -60 + Vector3::Right * -30 * j + Vector3::Up * 11 + Vector3::Forward * 10 * i);
			gameObjects.push_back(std::move(rModel));
		}
	}

	auto materials = assetLoader.GetMaterials();

	for (auto&& pair : materials)
	{
		pair->InitMaterial(&srvMemory);
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
		frameResources.push_back(std::make_shared<FrameResource>(device, 1, assetLoader.GetMaterials().size(), lights.size()));
	}
}


void SampleApp::Update(const GameTimer& gt)
{
	auto renderQueue = device->GetCommandQueue();

	currentFrameResourceIndex = (currentFrameResourceIndex + 1) % globalCountFrameResources;
	currentFrameResource = frameResources[currentFrameResourceIndex];

	if (currentFrameResource->FenceValue != 0 && !renderQueue->IsFinish(currentFrameResource->FenceValue))
	{
		renderQueue->WaitForFenceValue(currentFrameResource->FenceValue);
	}

	for (auto&& object : gameObjects)
	{
		object->Update();
	}


	auto view = camera->GetViewMatrix();
	auto proj = camera->GetProjectionMatrix();

	auto viewProj = (view * proj);
	auto invView = view.Invert();
	auto invProj = proj.Invert();
	auto invViewProj = viewProj.Invert();

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	Matrix T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);
	Matrix viewProjTex = XMMatrixMultiply(viewProj, T);
	mainPassCB.View = view.Transpose();
	mainPassCB.InvView = invView.Transpose();
	mainPassCB.Proj = proj.Transpose();
	mainPassCB.InvProj = invProj.Transpose();
	mainPassCB.ViewProj = viewProj.Transpose();
	mainPassCB.InvViewProj = invViewProj.Transpose();
	mainPassCB.ViewProjTex = viewProjTex.Transpose();
	mainPassCB.EyePosW = camera->gameObject->GetTransform()->GetWorldPosition();
	mainPassCB.RenderTargetSize = Vector2(static_cast<float>(MainWindow->GetClientWidth()),
	                                      static_cast<float>(MainWindow->GetClientHeight()));
	mainPassCB.InvRenderTargetSize = Vector2(1.0f / mainPassCB.RenderTargetSize.x,
	                                         1.0f / mainPassCB.RenderTargetSize.y);
	mainPassCB.NearZ = 1.0f;
	mainPassCB.FarZ = 1000.0f;
	mainPassCB.TotalTime = gt.TotalTime();
	mainPassCB.DeltaTime = gt.DeltaTime();
	mainPassCB.AmbientLight = Vector4{0.25f, 0.25f, 0.35f, 1.0f};
	
	auto currentPassCB = currentFrameResource->PassConstantBuffer.get();
	currentPassCB->CopyData(0, mainPassCB);

	auto currentMaterialBuffer = currentFrameResource->MaterialsBuffer.get();
	for (auto&& material : assetLoader.GetMaterials())
	{
		material->Update();
		auto constantData = material->GetMaterialConstantData();
		currentMaterialBuffer->CopyData(material->GetIndex(), constantData);
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

	cmdList->SetViewports(&viewport, 1);
	cmdList->SetScissorRects(&rect, 1);

	cmdList->TransitionBarrier(MainWindow->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	for (auto && texture : defferedGBufferTextures)
	{
		cmdList->TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}
	
	cmdList->TransitionBarrier(depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdList->FlushResourceBarriers();

	cmdList->SetRenderTargets(defferedGBufferTextures.size(), &defferedRTVMemory, 0, &dsvMemory, 0, true);
	
	cmdList->ClearRenderTarget(&rtvMemory, currentFrameResourceIndex, DirectX::Colors::Yellow);

	for (int i = 0; i < defferedGBufferTextures.size(); ++i)
	{
		cmdList->ClearRenderTarget(&defferedRTVMemory, i, DirectX::Colors::Green);
	}
	
	cmdList->ClearDepthStencil(&dsvMemory, 0);

	cmdList->SetRootSignature(rootSignature.get());
	cmdList->SetRootConstantBufferView(1, *currentFrameResource->PassConstantBuffer);
	cmdList->SetRootShaderResourceView(2, *currentFrameResource->MaterialsBuffer);
	cmdList->SetRootShaderResourceView(3, *currentFrameResource->LightsBuffer);
	
	cmdList->SetPipelineState(*opaque.get());	
	for (auto&& object : gameObjects)
	{
		object->Draw(cmdList);
	}

	cmdList->CopyResource(MainWindow->GetCurrentBackBuffer(), defferedGBufferTextures[0]);

	cmdList->TransitionBarrier(MainWindow->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
	for (auto&& texture : defferedGBufferTextures)
	{
		cmdList->TransitionBarrier(texture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	cmdList->TransitionBarrier(depthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
	cmdList->FlushResourceBarriers();

	currentFrameResource->FenceValue = renderQueue->ExecuteCommandList(cmdList);

	MainWindow->Present();
}

void SampleApp::OnResize()
{
	D3DApp::OnResize();

	currentFrameResourceIndex = MainWnd()->GetCurrentBackBufferIndex() - 1;
	
	viewport.Height = static_cast<float>(MainWindow->GetClientHeight());
	viewport.Width = static_cast<float>(MainWindow->GetClientWidth());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	rect = {0, 0, MainWindow->GetClientWidth(), MainWindow->GetClientHeight()};
	
	if (!depthBuffer.IsValid())
	{
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;
		depthStencilDesc.Width = MainWindow->GetClientWidth();
		depthStencilDesc.Height = MainWindow->GetClientHeight();
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		depthBuffer = GTexture(device, depthStencilDesc, L"Depth Map", TextureUsage::Depth, &optClear);
	}
	else
	{
		GTexture::Resize(depthBuffer, MainWindow->GetClientWidth(), MainWindow->GetClientHeight(), 1);
	}

	auto backBufferDesc = MainWindow->GetCurrentBackBuffer().GetD3D12ResourceDesc();	
	
	if (defferedGBufferTextures.size() < 1)
	{
		auto desc = backBufferDesc;	
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		defferedGBufferTextures.push_back((GTexture(device, desc, L"Deffered LightAccumulation Map", TextureUsage::RenderTarget)));

		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		defferedGBufferTextures.push_back((GTexture(device, desc, L"Deffered Diffuse Map", TextureUsage::RenderTarget)));

		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		defferedGBufferTextures.push_back((GTexture(device, desc, L"Deffered Specular Map", TextureUsage::RenderTarget)));

		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		defferedGBufferTextures.push_back((GTexture(device, desc, L"Deffered Normal Map", TextureUsage::RenderTarget)));
	}
	else
	{
		for (auto&& texture : defferedGBufferTextures)
		{
			GTexture::Resize(texture, MainWindow->GetClientWidth(), MainWindow->GetClientHeight(), 1);
		}
	}

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = GetSRGBFormat(backBufferDesc.Format);

	for (int i = 0; i < globalCountFrameResources; ++i)
	{
		MainWindow->GetBackBuffer(i).CreateRenderTargetView(&rtvDesc, &rtvMemory, i);
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	depthBuffer.CreateDepthStencilView(&dsvDesc, &dsvMemory);

	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	for (int i = 0; i < defferedGBufferTextures.size(); ++i)
	{
		auto desc = defferedGBufferTextures[i].GetD3D12ResourceDesc();
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Format = desc.Format;
		rtvDesc.Format = desc.Format;

		defferedGBufferTextures[i].CreateShaderResourceView(&srvDesc, &defferedSRVMemory, i);
		defferedGBufferTextures[i].CreateRenderTargetView(&rtvDesc, &defferedRTVMemory, i);
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
			/*if ((int)wParam == VK_F2)
				Set4xMsaaState(!isM4xMsaa);*/
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
