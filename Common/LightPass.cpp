#include "LightPass.h"
#include "AssetsLoader.h"
#include "d3dApp.h"
#include "GDeviceFactory.h"
#include "GModel.h"
#include "GPass.h"
#include "Scene.h"
#include "Window.h"
#include "SSAOPass.h"
namespace PEPEngine::Common
{
	void LightPass::LoadAndCompileShaders()
	{
		vertexShader = std::move(
			GShader(L"Shaders\\Quad.hlsl", VertexShader, nullptr, "VS", "vs_5_1"));
		vertexShader.LoadAndCompile();
		pixelShader = std::move(GShader(L"Shaders\\Quad.hlsl", PixelShader, nullptr, "PS", "ps_5_1"));
		pixelShader.LoadAndCompile();
	}

	void LightPass::CreateAndInitializeRS()
	{
		CD3DX12_DESCRIPTOR_RANGE texParam[6];
		texParam[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 2);
		texParam[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 2);
		texParam[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 2);
		texParam[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 2);
		texParam[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 2);
		texParam[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, MaxMaterialTexturesMaps, 0, 0); //Material Maps

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

	void LightPass::InitializePSO()
	{
		pso.SetInputLayout({VertexInputLayout, static_cast<UINT>(_countof(VertexInputLayout))});
		pso.SetRootSignature(rootSignature.GetRootSignature().Get());
		pso.SetPrimitiveType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		pso.SetRenderTargetsCount(1);
		pso.SetRTVFormat(0, GetSRGBFormat(DXGI_FORMAT_R8G8B8A8_UNORM));
		pso.SetSampleCount(1);
		pso.SetSampleQuality(0);
		pso.SetDSVFormat(DXGI_FORMAT_UNKNOWN);
		pso.SetSampleMask(UINT_MAX);
		pso.SetShader(&vertexShader);
		pso.SetShader(&pixelShader);
		pso.SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		

		auto depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		depthStencilDesc.DepthEnable = false;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

		pso.SetDepthStencilState(depthStencilDesc);
		pso.SetRasterizationState(CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT));
		pso.Initialize(GDeviceFactory::GetDevice());
	}

	void LightPass::ChangeRenderTargetSize(float width, float height)
	{
	}

	LightPass::LightPass(float width, float height,GPass& pass, SSAOPass& ssao): RenderPass(width, height), gpass(pass), ssaoPass(ssao)
	{
		LoadAndCompileShaders();

		CreateAndInitializeRS();

		InitializePSO();

		auto copyQueue = device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
		const auto cmdList = copyQueue->GetCommandList();

		quadModel = AssetsLoader::GenerateQuad(cmdList);

		copyQueue->WaitForFenceValue(copyQueue->ExecuteCommandList(cmdList));;
	}

	void PEPEngine::Common::LightPass::Render(std::shared_ptr<GCommandList> cmdList)
	{
		auto* const camera = Camera::mainCamera;

		const auto* target = camera->GetRenderTarget();
		auto* rtv = camera->GetRTV();
		UINT offset = 0;

		if (target == nullptr)
		{
			target = &D3DApp::GetApp().GetMainWindow()->GetCurrentBackBuffer();
			rtv = D3DApp::GetApp().GetMainWindow()->GetBackBuffersRTV();
			offset = D3DApp::GetApp().GetMainWindow()->GetCurrentBackBufferIndex();
		}


		cmdList->TransitionBarrier(target->GetD3D12Resource(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmdList->FlushResourceBarriers();

		cmdList->SetViewports(&camera->GetViewPort(), 1);
		cmdList->SetScissorRects(&camera->GetRect(), 1);

		cmdList->SetRootSignature(&rootSignature);
		cmdList->SetPipelineState(pso);

		const auto currentFrameResource = Scene::currentScene->GetCurrentFrameResource();

		cmdList->SetRootShaderResourceView(MaterialsBuffer, *currentFrameResource->MaterialsBuffer);
		cmdList->SetRootConstantBufferView(WorldDataBuffer, *currentFrameResource->WorldBuffer);
		cmdList->SetRootConstantBufferView(CameraDataBuffer, Camera::mainCamera->GetCameraDataBuffer());
		cmdList->SetRootShaderResourceView(LightBuffer, *currentFrameResource->LightsBuffer);

		cmdList->SetDescriptorsHeap(gpass.GetSRV());
		cmdList->SetRootDescriptorTable(NormalMap, gpass.GetSRV(), GPass::NormalMap);
		cmdList->SetRootDescriptorTable(BaseColorMap, gpass.GetSRV(), GPass::ColorMap);
		cmdList->SetRootDescriptorTable(PositionMap, gpass.GetSRV(), GPass::PositionMap);		
		cmdList->SetDescriptorsHeap(ssaoPass.AmbientMapSrv());
		cmdList->SetRootDescriptorTable(AmbientMap, ssaoPass.AmbientMapSrv());

		cmdList->ClearRenderTarget(rtv, offset, DirectX::Colors::Black);
		cmdList->SetRenderTargets(1, rtv, offset);


		quadModel->Draw(cmdList);

		cmdList->TransitionBarrier(target->GetD3D12Resource(), D3D12_RESOURCE_STATE_COMMON);
		cmdList->FlushResourceBarriers();
	}
}