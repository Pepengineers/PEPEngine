#include "GPass.h"
#include "Scene.h"
#include "d3dApp.h"
#include "Window.h"
namespace PEPEngine::Common
{
	void GPass::AllocateDescriptors()
	{
		deferredRTVDescriptor = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, GBufferMapsCount);

		//+1 для SRV Depth Map
		deferredSRVDescriptor = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			GBufferMapsCount);

		deferredDSVDescriptor = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
	}

	void GPass::InitRootSignature()
	{
		CD3DX12_DESCRIPTOR_RANGE texParam[6];
		texParam[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2);
		texParam[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 2);
		texParam[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 2);
		texParam[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 2);
		texParam[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 2);
		texParam[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MaxMaterialTexturesMaps, 0, 0); //Material Maps

		rootSign.AddConstantBufferParameter(0); // ObjectData
		rootSign.AddDescriptorParameter(&texParam[5], 1, D3D12_SHADER_VISIBILITY_PIXEL); //MaterialsTexture
		rootSign.AddConstantBufferParameter(1); // CameraData
		rootSign.AddConstantBufferParameter(2); // WorldData
		rootSign.AddShaderResourceView(0, 1); // MaterialData				

		rootSign.Initialize(device);
	}

	void GPass::InitPSO()
	{
		gpassVertexShader = std::move(GShader(L"Shaders\\Default.hlsl", VertexShader, nullptr, "VS", "vs_5_1"));
		gpassVertexShader.LoadAndCompile();

		gpassPixelShader = std::move(GShader(L"Shaders\\Default.hlsl", PixelShader, nullptr, "PS", "ps_5_1"));
		gpassPixelShader.LoadAndCompile();

		deferredPSO.SetInputLayout({ VertexInputLayout, static_cast<UINT>(_countof(VertexInputLayout)) });
		deferredPSO.SetRootSignature(rootSign.GetRootSignature().Get());
		deferredPSO.SetPrimitiveType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		deferredPSO.SetRenderTargetsCount(GBufferMapsCount - 1);
		deferredPSO.SetRTVFormat(0, GetSRGBFormat(NormalMapFormat));
		deferredPSO.SetRTVFormat(1, GetSRGBFormat(BaseColorMapFormat));
		deferredPSO.SetRTVFormat(2, GetSRGBFormat(PositionMapFormat));
		deferredPSO.SetSampleCount(1);
		deferredPSO.SetSampleQuality(0);
		deferredPSO.SetDSVFormat(DepthMapFormat);
		deferredPSO.SetSampleMask(UINT_MAX);
		deferredPSO.SetShader(&gpassVertexShader);
		deferredPSO.SetShader(&gpassPixelShader);
		deferredPSO.SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		deferredPSO.SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
		deferredPSO.SetRasterizationState(CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT));
		deferredPSO.Initialize(device);
	}

	GPass::GPass(std::shared_ptr<GDevice> renderDevice) : device(renderDevice)
	{
		AllocateDescriptors();

		InitRootSignature();

		InitPSO();
	}

	GTexture& GPass::GetGTexture(DeferredPassRTVSlot slot)
	{
		return gbuffers[slot];
	}

	const GDescriptor* GPass::GetSRV() const
	{
		return &deferredSRVDescriptor;
	}

	void GPass::Render(std::shared_ptr<GCommandList> cmdList)
	{
		const auto camera = Camera::mainCamera;

		cmdList->SetViewports(&camera->GetViewPort(), 1);
		cmdList->SetScissorRects(&camera->GetRect(), 1);

		for (int i = 0; i < GBufferMapsCount - 1; ++i)
		{
			cmdList->TransitionBarrier(gbuffers[i], D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
		cmdList->TransitionBarrier(gbuffers[DepthMap], D3D12_RESOURCE_STATE_DEPTH_WRITE);
		cmdList->FlushResourceBarriers();

		cmdList->ClearRenderTarget(&deferredRTVDescriptor, NormalMap, DirectX::Colors::Blue);
		cmdList->ClearRenderTarget(&deferredRTVDescriptor, ColorMap, DirectX::Colors::Black);
		cmdList->ClearRenderTarget(&deferredRTVDescriptor, PositionMap, DirectX::Colors::Blue);
		cmdList->ClearDepthStencil(&deferredDSVDescriptor, 0);


		cmdList->SetRenderTargets(GBufferMapsCount - 1, &deferredRTVDescriptor, 0, &deferredDSVDescriptor);

		cmdList->SetRootSignature(&rootSign);
		cmdList->SetPipelineState(deferredPSO);
		
		const auto currentScene = Scene::currentScene;

		cmdList->SetRootConstantBufferView(WorldDataBuffer, *currentScene->GetCurrentFrameResource()->WorldBuffer.get());
		cmdList->SetRootConstantBufferView(CameraDataBuffer, Camera::mainCamera->GetCameraDataBuffer());
		cmdList->SetRootShaderResourceView(MaterialsBuffer, *currentScene->GetCurrentFrameResource()->MaterialsBuffer.get());
		
		currentScene->Render(RenderMode::Opaque, cmdList);
		

		for (int i = 0; i < GBufferMapsCount; ++i)
		{
			cmdList->TransitionBarrier(gbuffers[i], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
		
		cmdList->FlushResourceBarriers();
	}

	void GPass::OnResize()
	{
		const auto* window = D3DApp::GetApp().GetMainWindow();

		if (gbuffers.size() < GBufferMapsCount)
		{
			auto desc = CD3DX12_RESOURCE_DESC(
				CD3DX12_RESOURCE_DESC::Tex2D(NormalMapFormat, window->GetClientWidth(),
					window->GetClientHeight()));
			desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			desc.MipLevels = 1;

			CD3DX12_CLEAR_VALUE optClear(NormalMapFormat, DirectX::Colors::Blue);
			
			gbuffers.push_back((GTexture(device, desc, L"Normal Roughness GMap", TextureUsage::RenderTarget, &optClear)));

			desc.Format = BaseColorMapFormat;

			optClear = CD3DX12_CLEAR_VALUE(desc.Format, DirectX::Colors::Black);
			
			gbuffers.push_back((GTexture(device, desc, L"BaseColor Metalnes GMAP", TextureUsage::RenderTarget, &optClear)));

			desc.Format = PositionMapFormat;

			optClear = CD3DX12_CLEAR_VALUE(desc.Format, DirectX::Colors::Black);
			
			gbuffers
				.push_back((GTexture(device, desc, L"Position GMAP", TextureUsage::RenderTarget, &optClear)));

			
			optClear.Format = DepthMapFormat;
			optClear.DepthStencil.Depth = 1.0f;
			optClear.DepthStencil.Stencil = 0;

			desc.Format = DepthMapFormat;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			gbuffers.push_back(GTexture(device, desc, L"Depth Map", TextureUsage::Depth, &optClear));
		}
		else
		{
			for (auto&& texture : gbuffers)
			{
				GTexture::Resize(texture, window->GetClientWidth(),
					window->GetClientHeight(), 1);
			}
		}


		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;
		gbuffers[DepthMap].CreateDepthStencilView(&dsvDesc, &deferredDSVDescriptor);


		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Texture2D.MipLevels = 1;

		for (int i = 0; i < gbuffers.size() - 1; ++i)
		{
			auto desc = gbuffers[i].GetD3D12ResourceDesc();
			srvDesc.Format = desc.Format;

			gbuffers[i].CreateShaderResourceView(&srvDesc, &deferredSRVDescriptor, i);
			gbuffers[i].CreateRenderTargetView(&rtvDesc, &deferredRTVDescriptor, i);
		}

		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		gbuffers[DepthMap].CreateShaderResourceView(&srvDesc, &deferredSRVDescriptor, DepthMap);
	}
}