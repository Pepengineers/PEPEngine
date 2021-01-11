#include "ShadowPass.h"



#include "Camera.h"
#include "Material.h"
#include "Transform.h"

namespace PEPEngine::Common
{
	void ShadowPass::InitRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE texParam[6];
		texParam[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2);
		texParam[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 2);
		texParam[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 2);
		texParam[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 2);
		texParam[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 2);
		texParam[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Material::MaxMaterialTexturesMaps, 0, 0); //Material Maps

		rootSignature.AddConstantBufferParameter(0); // ObjectData
		rootSignature.AddDescriptorParameter(&texParam[5], 1, D3D12_SHADER_VISIBILITY_PIXEL); //MaterialsTexture
		rootSignature.AddConstantBufferParameter(1); // CameraData
		rootSignature.AddConstantBufferParameter(2); // WorldData
		rootSignature.AddShaderResourceView(0, 1); // MaterialData
		rootSignature.AddShaderResourceView(1, 1); // LightData		

		rootSignature.AddDescriptorParameter(&texParam[0], 1, D3D12_SHADER_VISIBILITY_PIXEL); //NormalRoughness
		rootSignature.AddDescriptorParameter(&texParam[1], 1, D3D12_SHADER_VISIBILITY_PIXEL); //BaseColorMetalness
		rootSignature.AddDescriptorParameter(&texParam[2], 1, D3D12_SHADER_VISIBILITY_PIXEL); //Position
		rootSignature.AddDescriptorParameter(&texParam[3], 1, D3D12_SHADER_VISIBILITY_PIXEL); //Ambient
		rootSignature.AddDescriptorParameter(&texParam[4], 1, D3D12_SHADER_VISIBILITY_PIXEL); //Depth

		rootSignature.Initialize(GDeviceFactory::GetDevice());
	}

	void ShadowPass::LoadAndCompileShaders()
	{
		vertexShader = std::move(
			GShader(L"Shaders\\Shadows.hlsl", VertexShader, nullptr, "VS", "vs_5_1"));
		vertexShader.LoadAndCompile();
		pixelShader = std::move(
			GShader(L"Shaders\\Shadows.hlsl", PixelShader, nullptr, "PS", "ps_5_1"));
		pixelShader.LoadAndCompile();
	}

	void ShadowPass::InitPSO()
	{
		pso.SetInputLayout({VertexInputLayout, static_cast<UINT>(_countof(VertexInputLayout))});
		pso.SetRootSignature(rootSignature.GetRootSignature().Get());
		pso.SetPrimitiveType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		pso.SetRenderTargetsCount(1);
		pso.SetRTVFormat(0, GetSRGBFormat(DXGI_FORMAT_D24_UNORM_S8_UINT));
		pso.SetSampleCount(1);
		pso.SetSampleQuality(0);
		pso.SetDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
		pso.SetSampleMask(UINT_MAX);
		pso.SetShader(&vertexShader);
		pso.SetShader(&pixelShader);
		pso.SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		pso.SetRTVFormat(0, DXGI_FORMAT_UNKNOWN);
		pso.SetRenderTargetsCount(0);

		auto rasterizedDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		rasterizedDesc.DepthBias = 100000;
		rasterizedDesc.DepthBiasClamp = 0.0f;
		rasterizedDesc.SlopeScaledDepthBias = 1.0f;
		pso.SetRasterizationState(rasterizedDesc);

		pso.Initialize(device);
	}

	ShadowPass::ShadowPass(float width, float height) : RenderPass(width, height)
	{
		viewPort = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
		rect = {0, 0, static_cast<int>(width), static_cast<int>(height)};

		srvMemory = this->device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
		dsvMemory = this->device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);

		projectionMatrix = DirectX::XMMatrixOrthographicLH(width, height, 0.1f, 1000);

		for (int i = 0; i < globalCountFrameResources; ++i)
		{
			LightCameraBuffer[i] = std::make_shared<ConstantUploadBuffer<CameraConstants>>(
				device, 1, L" Shadow Map Light Buffer");
		}
		
	

		BuildResource();


		InitRootSignature();

		LoadAndCompileShaders();

		InitPSO();

		currentBuffer = LightCameraBuffer[currentIndex];
	}

	GDescriptor* ShadowPass::GetSrvMemory()
	{
		return &srvMemory;
	}

	GDescriptor* ShadowPass::GetDsvMemory()
	{
		return &dsvMemory;
	}

	void ShadowPass::BuildResource()
	{
		if (shadowMap.GetD3D12Resource() == nullptr)
		{
			D3D12_RESOURCE_DESC texDesc;
			ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
			texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			texDesc.Alignment = 0;
			texDesc.Width = width;
			texDesc.Height = height;
			texDesc.DepthOrArraySize = 1;
			texDesc.MipLevels = 1;
			texDesc.Format = mFormat;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			D3D12_CLEAR_VALUE optClear;
			optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;

			shadowMap = GTexture(device, texDesc, std::wstring(L"Shadow Map"), TextureUsage::Depth, &optClear);
		}
		else
		{
			GTexture::Resize(shadowMap, width, height, 1);
		}

		BuildDescriptors();
	}

	void ShadowPass::BuildDescriptors()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Texture2D.PlaneSlice = 0;
		shadowMap.CreateShaderResourceView(&srvDesc, &srvMemory);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;
		shadowMap.CreateDepthStencilView(&dsvDesc, &dsvMemory);
	}

	void ShadowPass::Render(std::shared_ptr<GCommandList> cmdList)
	{
		cmdList->SetRootSignature(&rootSignature);
		cmdList->SetPipelineState(pso);

		const auto scene = Scene::currentScene;
		const auto currentFrameResource = Scene::currentScene->GetCurrentFrameResource();

		cmdList->SetRootShaderResourceView(MaterialsDataBuffer, *currentFrameResource->MaterialsBuffer);
		cmdList->SetRootConstantBufferView(WorldDataBuffer, *currentFrameResource->WorldBuffer);
		cmdList->SetRootConstantBufferView(CameraDataBuffer, *currentBuffer.get());

		cmdList->SetViewports(&viewPort, 1);
		cmdList->SetScissorRects(&rect, 1);

		cmdList->TransitionBarrier(shadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		cmdList->FlushResourceBarriers();

		cmdList->SetRenderTargets(0, nullptr, 0, &dsvMemory, 0);
		cmdList->ClearDepthStencil(&dsvMemory, 0,
		                           D3D12_CLEAR_FLAG_DEPTH);

		scene->RenderTypedObjects(RenderMode::Opaque, cmdList);
		scene->RenderTypedObjects(RenderMode::OpaqueAlphaDrop, cmdList);

		cmdList->TransitionBarrier(shadowMap, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		cmdList->FlushResourceBarriers();
	}

	void ShadowPass::Update()
	{		
		if (mainDirectionalLight == nullptr)
		{
			auto* scene = Scene::currentScene;
			for (auto&& light : scene->sceneLights)
			{
				if (light->Type == Directional)
				{
					mainDirectionalLight = light;
					break;
				}
			}
		}

		if (mainDirectionalLight != nullptr)
		{
			auto view = mainDirectionalLight->GetViewMatrix();
			auto proj = projectionMatrix;

			auto viewProj = (view * proj);
			auto invView = view.Invert();
			auto invProj = proj.Invert();
			auto invViewProj = viewProj.Invert();

			shadowMapConstants.View = view.Transpose();
			shadowMapConstants.InvView = invView.Transpose();
			shadowMapConstants.Proj = proj.Transpose();
			shadowMapConstants.InvProj = invProj.Transpose();
			shadowMapConstants.ViewProj = viewProj.Transpose();
			shadowMapConstants.InvViewProj = invViewProj.Transpose();
			shadowMapConstants.EyePosW = mainDirectionalLight->gameObject->GetTransform()->GetWorldPosition();
			shadowMapConstants.NearZ = 0.1;
			shadowMapConstants.FarZ = 1000;
			shadowMapConstants.RenderTargetSize = Vector2(static_cast<float>(width), static_cast<float>(height));
			shadowMapConstants.InvRenderTargetSize = Vector2(1.0f / width, 1.0f / height);

			// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
			static Matrix T(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, -0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.5f, 0.5f, 0.0f, 1.0f);

			Matrix S = view * proj * T;
			shadowMapConstants.ShadowTransform = S;

			Camera::mainCamera->SetShadowTransform(S);
			
			currentBuffer->CopyData(0, shadowMapConstants);
		}

		currentIndex = (currentIndex + 1) % globalCountFrameResources;
		currentBuffer = LightCameraBuffer[currentIndex];
	}

	void ShadowPass::ChangeRenderTargetSize(float newWidth, float newHeight)
	{
		if ((width != newWidth) || (height != newHeight))
		{
			width = newWidth;
			height = newHeight;

			projectionMatrix = DirectX::XMMatrixOrthographicLH(width, height, 0.1f, 1000);

			BuildResource();
		}
	}
}
