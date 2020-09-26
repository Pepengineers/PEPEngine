#include "SampleApp.h"


#include "CameraController.h"
#include "GDeviceFactory.h"
#include "Material.h"
#include "Window.h"
#include "GameObject.h"
#include "GCommandList.h"
#include "Light.h"
#include "ModelRenderer.h"
#include "Transform.h"

SampleApp::SampleApp(HINSTANCE hInstance): D3DApp(hInstance), assetLoader(AssetsLoader(GDeviceFactory::GetDevice()))
{
	
}

bool SampleApp::Initialize()
{
	device = GDeviceFactory::GetDevice();
	
	dsvMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	rtvMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3);
	srvMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, assetLoader.GetLoadTexturesCount());
	
	
	if(!D3DApp::Initialize())
	{
		return false;
	}

	auto copyQueue = device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);

	auto atlas = assetLoader.GetOrCreateModelFromFile(copyQueue, "Data\\Objects\\Atlas\\Atlas.obj");
	models[L"atlas"] = std::move(atlas);
	copyQueue->Flush();


	rootSignature = std::make_shared<GRootSignature>();

	CD3DX12_DESCRIPTOR_RANGE texParam[4];
	texParam[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, StandardShaderSlot::SkyMap - 3, 0); //SkyMap
	texParam[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, StandardShaderSlot::ShadowMap - 3, 0); //ShadowMap
	texParam[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, StandardShaderSlot::AmbientMap - 3, 0); //SsaoMap
	texParam[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, assetLoader.GetLoadTexturesCount(), StandardShaderSlot::TexturesMap - 3, 0);


	rootSignature->AddConstantBufferParameter(0);
	rootSignature->AddConstantBufferParameter(1);
	rootSignature->AddConstantBufferParameter(0, 1);
	rootSignature->AddDescriptorParameter(&texParam[0], 1, D3D12_SHADER_VISIBILITY_PIXEL);
	rootSignature->AddDescriptorParameter(&texParam[1], 1, D3D12_SHADER_VISIBILITY_PIXEL);
	rootSignature->AddDescriptorParameter(&texParam[2], 1, D3D12_SHADER_VISIBILITY_PIXEL);
	rootSignature->AddDescriptorParameter(&texParam[3], 1, D3D12_SHADER_VISIBILITY_PIXEL);

	rootSignature->Initialize(device);


	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		nullptr, nullptr
	};

	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		nullptr, nullptr
	};

	shaders[L"StandardVertex"] = std::move(
		std::make_unique<GShader>(L"Shaders\\Default.hlsl", VertexShader, nullptr, "VS", "vs_5_1"));
		
	shaders[L"OpaquePixel"] = std::move(
		std::make_unique<GShader>(L"Shaders\\Default.hlsl", PixelShader, defines, "PS", "ps_5_1"));

	for (auto && sh : shaders)
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
	basePsoDesc.InputLayout = { defaultInputLayout.data(), static_cast<UINT>(defaultInputLayout.size()) };
	basePsoDesc.pRootSignature = rootSignature->GetRootSignature().Get();
	basePsoDesc.VS = shaders[L"StandardVertex"]->GetShaderResource();
	basePsoDesc.PS = shaders[L"OpaquePixel"]->GetShaderResource();
	basePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	basePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	basePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	basePsoDesc.SampleMask = UINT_MAX;
	basePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	basePsoDesc.NumRenderTargets = 1;
	basePsoDesc.RTVFormats[0] = GetSRGBFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	basePsoDesc.SampleDesc.Count = isM4xMsaa ? 4 : 1;
	basePsoDesc.SampleDesc.Quality = isM4xMsaa ? (m4xMsaaQuality - 1) : 0;
	basePsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	opaque = std::make_shared<GraphicPSO>();
	opaque->SetPsoDesc(basePsoDesc);
	opaque->Initialize(device);

	auto sun1 = std::make_unique<GameObject>("Directional Light");
	auto light = new Light(Directional);
	light->Direction({ 0.57735f, -0.57735f, 0.57735f });
	light->Strength({ 0.8f, 0.8f, 0.8f });
	sun1->AddComponent(light);
	gameObjects.push_back(std::move(sun1));

	
	auto cameraGO = std::make_unique<GameObject>("MainCamera");
	cameraGO->GetTransform()->SetEulerRotate(Vector3(-30, 270, 0));
	cameraGO->GetTransform()->SetPosition(Vector3(-1000, 190, -32));
	cameraGO->AddComponent(new Camera(AspectRatio()));
	cameraGO->AddComponent(new CameraController());
	gameObjects.push_back(std::move(cameraGO));

	for (int i = 0; i < 12; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			auto atlas = std::make_unique<GameObject>();
			auto renderer = new ModelRenderer();
			atlas->AddComponent(renderer);
			renderer->SetModel(assetLoader.GetModelByName(L"Data\\Objects\\Atlas\\Atlas.obj"));
			for (int k = 0; k < renderer->model->GetMeshesCount(); ++k)
			{
				renderer->SetMeshMaterial(k, assetLoader.GetDefaultMaterial(renderer->model->GetMesh(k)));
			}				
			atlas->GetTransform()->SetPosition(Vector3::Right * -60 + Vector3::Right * -30 * j + Vector3::Up * 11 + Vector3::Forward * 10 * i);
			gameObjects.push_back(std::move(atlas));
		}
	}

	auto materials = assetLoader.GetDefaultMaterialForMeshes();

	for (auto && pair : materials)
	{
		pair.second->InitMaterial(&srvMemory);
	}

	for (int i = 0; i < globalCountFrameResources; ++i)
	{
		frameResources.push_back(std::make_shared<FrameResource>(device, 1));
	}

	for (auto&& item : gameObjects)
	{
		auto lightComponent = item->GetComponent<Light>();
		if (lightComponent != nullptr)
		{
			lights.push_back(lightComponent);
		}
		
		auto cam = item->GetComponent<Camera>();
		if (cam != nullptr)
		{
			camera = std::unique_ptr<Camera>(cam);
		}
	}
}


void SampleApp::Update(const GameTimer& gt)
{
	auto renderQueue = device->GetCommandQueue();
		
	currentFrameResource = frameResources[MainWindow->GetCurrentBackBufferIndex()];

	if(currentFrameResource->FenceValue != 0 && renderQueue->IsFinish(currentFrameResource->FenceValue))
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
	mainPassCB.AmbientLight = Vector4{ 0.25f, 0.25f, 0.35f, 1.0f };

	for (int i = 0; i < MaxLights; ++i)
	{
		if (i < lights.size())
		{
			mainPassCB.Lights[i] = lights[i]->GetData();
		}
		else
		{
			break;
		}
	}

	auto currentPassCB = currentFrameResource->PassConstantBuffer.get();
	currentPassCB->CopyData(0, mainPassCB);
}

void SampleApp::Draw(const GameTimer& gt)
{
	if(isResizing) return;

	auto renderQueue = device->GetCommandQueue();
	auto cmdList = renderQueue->GetCommandList();

	cmdList->SetGMemory(&srvMemory);
	cmdList->SetViewports(&viewport, 1);
	cmdList->SetScissorRects(&rect, 1);

	cmdList->TransitionBarrier(MainWindow->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdList->TransitionBarrier(depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	cmdList->FlushResourceBarriers();

	
	cmdList->SetRenderTargets(1, &rtvMemory, MainWindow->GetCurrentBackBufferIndex(), &dsvMemory);
	cmdList->ClearRenderTarget(&rtvMemory, MainWindow->GetCurrentBackBufferIndex(), DirectX::Colors::Yellow);
	cmdList->ClearDepthStencil(&dsvMemory, 0);

	cmdList->SetRootSignature(rootSignature.get());
	cmdList->SetRootConstantBufferView(1, *currentFrameResource->PassConstantBuffer);
	cmdList->SetPipelineState(*opaque.get());
	for (auto && object : gameObjects)
	{
		object->Draw(cmdList);
	}

	

	
	cmdList->TransitionBarrier(MainWindow->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
	cmdList->TransitionBarrier(depthBuffer, D3D12_RESOURCE_STATE_DEPTH_READ);
	cmdList->FlushResourceBarriers();

	currentFrameResource->FenceValue = renderQueue->ExecuteCommandList(cmdList);
	
	MainWindow->Present();	
}

void SampleApp::OnResize()
{
	D3DApp::OnResize();

	viewport.Height = static_cast<float>(MainWindow->GetClientHeight());
	viewport.Width = static_cast<float>(MainWindow->GetClientWidth());
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	rect = { 0,0, MainWindow->GetClientWidth() , MainWindow->GetClientHeight() };


	auto backBufferDesc = MainWindow->GetCurrentBackBuffer().GetD3D12ResourceDesc();
	
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Format = GetSRGBFormat(backBufferDesc.Format);

	for (int i = 0; i < globalCountFrameResources; ++i)
	{
		MainWindow->GetBackBuffer(i).CreateRenderTargetView(&rtvDesc, &rtvMemory, i);
	}
	
	if(!depthBuffer.IsValid())
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


	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.Texture2D.MipSlice = 0;
	
	depthBuffer.CreateDepthStencilView(&dsvDesc, &dsvMemory);	

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
