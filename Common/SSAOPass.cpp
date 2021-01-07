#include "SSAOPass.h"

#include "GPass.h"
#include "Scene.h"
#include "GTexture.h"

namespace PEPEngine::Common
{
	void SSAOPass::BuildRandomVectorTexture(std::shared_ptr<GCommandList> cmdList)
	{
		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.Width = 256;
		texDesc.Height = 256;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		randomVectorMap = GTexture(device, texDesc, L"SSAO Random Vector Map", TextureUsage::Normalmap);

		std::vector<Vector4> data;
		data.resize(256 * 256);

		for (int i = 0; i < 256; ++i)
		{
			for (int j = 0; j < 256; ++j)
			{
				// Random vector in [0,1].  We will decompress in shader to [-1,1].
				Vector3 v(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF());
				data[i * 256 + j] = Vector4(v.x, v.y, v.z, 0.0f);
			}
		}

		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = data.data();
		subResourceData.RowPitch = 256 * sizeof(Vector4);
		subResourceData.SlicePitch = subResourceData.RowPitch * 256;

		cmdList->TransitionBarrier(randomVectorMap.GetD3D12Resource(), D3D12_RESOURCE_STATE_COPY_DEST);
		cmdList->FlushResourceBarriers();

		cmdList->UpdateSubresource(randomVectorMap, &subResourceData, 1);

		cmdList->TransitionBarrier(randomVectorMap.GetD3D12Resource(), D3D12_RESOURCE_STATE_GENERIC_READ);
		cmdList->FlushResourceBarriers();
	}

	void SSAOPass::BuildOffsetVectors()
	{
		// Start with 14 uniformly distributed vectors.  We choose the 8 corners of the cube
		// and the 6 center points along each cube face.  We always alternate the points on 
		// opposites sides of the cubes.  This way we still get the vectors spread out even
		// if we choose to use less than 14 samples.

		// 8 cube corners
		offsets[0] = Vector4(+1.0f, +1.0f, +1.0f, 0.0f);
		offsets[1] = Vector4(-1.0f, -1.0f, -1.0f, 0.0f);

		offsets[2] = Vector4(-1.0f, +1.0f, +1.0f, 0.0f);
		offsets[3] = Vector4(+1.0f, -1.0f, -1.0f, 0.0f);

		offsets[4] = Vector4(+1.0f, +1.0f, -1.0f, 0.0f);
		offsets[5] = Vector4(-1.0f, -1.0f, +1.0f, 0.0f);

		offsets[6] = Vector4(-1.0f, +1.0f, -1.0f, 0.0f);
		offsets[7] = Vector4(+1.0f, -1.0f, +1.0f, 0.0f);

		// 6 centers of cube faces
		offsets[8] = Vector4(-1.0f, 0.0f, 0.0f, 0.0f);
		offsets[9] = Vector4(+1.0f, 0.0f, 0.0f, 0.0f);

		offsets[10] = Vector4(0.0f, -1.0f, 0.0f, 0.0f);
		offsets[11] = Vector4(0.0f, +1.0f, 0.0f, 0.0f);

		offsets[12] = Vector4(0.0f, 0.0f, -1.0f, 0.0f);
		offsets[13] = Vector4(0.0f, 0.0f, +1.0f, 0.0f);

		for (auto& mOffset : offsets)
		{
			// Create random lengths in [0.25, 1.0].
			float s = MathHelper::RandF(0.25f, 1.0f);
			mOffset.Normalize();
			mOffset = s * mOffset;
		}
	}

	GTexture SSAOPass::CreateAmbientMap() const
	{
		D3D12_RESOURCE_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Alignment = 0;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		texDesc.Width = renderTargetWidth;
		texDesc.Height = renderTargetHeight;
		texDesc.Format = AmbientMapFormat;

		float ambientClearColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
		auto optClear = CD3DX12_CLEAR_VALUE(AmbientMapFormat, ambientClearColor);

		return GTexture(device, texDesc, L"SSAO AmbientMap", TextureUsage::RenderTarget, &optClear);
	}

	void SSAOPass::CreateSSAORS()
	{
		ssaoPrimeRootSignature = std::make_shared<GRootSignature>();

		CD3DX12_DESCRIPTOR_RANGE texTable0[2];
		texTable0[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
		texTable0[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);

		CD3DX12_DESCRIPTOR_RANGE texTable1;
		texTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);

		ssaoPrimeRootSignature->AddConstantBufferParameter(0);
		ssaoPrimeRootSignature->AddConstantParameter(1, 1);
		ssaoPrimeRootSignature->AddDescriptorParameter(&texTable0[0], 1, D3D12_SHADER_VISIBILITY_PIXEL);
		ssaoPrimeRootSignature->AddDescriptorParameter(&texTable0[1], 1, D3D12_SHADER_VISIBILITY_PIXEL);
		ssaoPrimeRootSignature->AddDescriptorParameter(&texTable1, 1, D3D12_SHADER_VISIBILITY_PIXEL);

		const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
			0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
			1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP, // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC depthMapSam(
			2, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_BORDER, // addressU
			D3D12_TEXTURE_ADDRESS_MODE_BORDER, // addressV
			D3D12_TEXTURE_ADDRESS_MODE_BORDER, // addressW
			0.0f,
			0,
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

		const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
			3, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		std::array<CD3DX12_STATIC_SAMPLER_DESC, 4> staticSamplers =
		{
			pointClamp, linearClamp, depthMapSam, linearWrap
		};

		for (auto&& sampler : staticSamplers)
		{
			ssaoPrimeRootSignature->AddStaticSampler(sampler);
		}

		ssaoPrimeRootSignature->Initialize(device);
	}

	void SSAOPass::LoadAndCompileShaders()
	{
		ssaoVertexShader = std::move(
			GShader(L"Shaders\\Ssao.hlsl", VertexShader, nullptr, "VS", "vs_5_1"));
		ssaoVertexShader.LoadAndCompile();
		ssaoPixelShader = std::move(
			GShader(L"Shaders\\Ssao.hlsl", PixelShader, nullptr, "PS", "ps_5_1"));
		ssaoPixelShader.LoadAndCompile();
		blurVertexShader = std::move(
			GShader(L"Shaders\\SsaoBlur.hlsl", VertexShader, nullptr, "VS", "vs_5_1"));
		blurVertexShader.LoadAndCompile();
		blurPixelShader = std::move(
			GShader(L"Shaders\\SsaoBlur.hlsl", PixelShader, nullptr, "PS", "ps_5_1"));
		blurPixelShader.LoadAndCompile();
	}

	void SSAOPass::CreatePSO()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC basePsoDesc;
		ZeroMemory(&basePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		basePsoDesc.pRootSignature = ssaoPrimeRootSignature->GetRootSignature().Get();
		basePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		basePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		basePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		basePsoDesc.SampleMask = UINT_MAX;
		basePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		basePsoDesc.NumRenderTargets = 1;
		basePsoDesc.SampleDesc.Count = 1;
		basePsoDesc.SampleDesc.Quality = 0;

		ssaoPSO.SetPsoDesc(basePsoDesc);
		ssaoPSO.SetInputLayout({nullptr, 0});
		ssaoPSO.SetShader(&ssaoPixelShader);
		ssaoPSO.SetShader(&ssaoVertexShader);
		ssaoPSO.SetRTVFormat(0, AmbientMapFormat);
		ssaoPSO.SetSampleCount(1);
		ssaoPSO.SetSampleQuality(0);
		ssaoPSO.SetDSVFormat(DXGI_FORMAT_UNKNOWN);
		auto depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		depthStencilDesc.DepthEnable = false;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		ssaoPSO.SetDepthStencilState(depthStencilDesc);

		ssaoPSO.Initialize(device);


		blurPSO.SetPsoDesc(ssaoPSO.GetPsoDescription());
		blurPSO.SetShader(&blurPixelShader);
		blurPSO.SetShader(&blurVertexShader);
		blurPSO.Initialize(device);
	}

	SSAOPass::SSAOPass(float width, float height, GPass& pass) : RenderPass(width, height), gpass(pass)
	{
		randomVectorSrvMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
		ambientMapMapSrvMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2);
		ambientMapRtvMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2);

		CreateSSAORS();

		LoadAndCompileShaders();

		CreatePSO();


		auto queue = device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
		auto cmdList = queue->GetCommandList();

		BuildRandomVectorTexture(cmdList);

		queue->WaitForFenceValue(queue->ExecuteCommandList(cmdList));


		BuildOffsetVectors();
		ChangeResolutionSize(width, height);

		for (int i = 0; i < globalCountFrameResources; ++i)
		{
			SsaoConstantUploadBuffers[i] = std::make_shared<ConstantUploadBuffer<SsaoConstants>
			>(device, 1, L"SSAO Constant Buffer" + std::to_wstring(i));
		}
	}


	void SSAOPass::ChangeResolutionSize(UINT newWidth, UINT newHeight)
	{
		if (renderTargetWidth != newWidth || renderTargetHeight != newHeight)
		{
			renderTargetWidth = newWidth;
			renderTargetHeight = newHeight;

			viewport.TopLeftX = 0.0f;
			viewport.TopLeftY = 0.0f;
			viewport.Width = renderTargetWidth;
			viewport.Height = renderTargetHeight;
			viewport.MinDepth = 0.0f;
			viewport.MaxDepth = 1.0f;

			scissorRect = {0, 0, static_cast<int>(renderTargetWidth), static_cast<int>(renderTargetHeight)};


			if (ambientMap0.GetD3D12Resource() == nullptr)
			{
				ambientMap0 = CreateAmbientMap();
			}
			else
			{
				GTexture::Resize(ambientMap0, renderTargetWidth, renderTargetHeight, 1);
			}

			if (ambientMap1.GetD3D12Resource() == nullptr)
			{
				ambientMap1 = CreateAmbientMap();
			}
			else
			{
				GTexture::Resize(ambientMap1, renderTargetWidth, renderTargetHeight, 1);
			}

			RebuildDescriptors();
		}
	}

	void SSAOPass::RebuildDescriptors()
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		randomVectorMap.CreateShaderResourceView(&srvDesc, &randomVectorSrvMemory);

		srvDesc.Format = AmbientMapFormat;
		ambientMap0.CreateShaderResourceView(&srvDesc, &ambientMapMapSrvMemory);
		ambientMap1.CreateShaderResourceView(&srvDesc, &ambientMapMapSrvMemory, 1);

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;
		rtvDesc.Texture2D.PlaneSlice = 0;

		rtvDesc.Format = AmbientMapFormat;
		ambientMap0.CreateRenderTargetView(&rtvDesc, &ambientMapRtvMemory);
		ambientMap1.CreateRenderTargetView(&rtvDesc, &ambientMapRtvMemory, 1);
	}

	UINT SSAOPass::SsaoMapWidth() const
	{
		return renderTargetWidth;
	}

	UINT SSAOPass::SsaoMapHeight() const
	{
		return renderTargetHeight;
	}

	GTexture& SSAOPass::AmbientMap()
	{
		return ambientMap0;
	}

	GDescriptor* SSAOPass::AmbientMapSrv()
	{
		return &ambientMapMapSrvMemory;
	}

	void SSAOPass::GetOffsetVectors(Vector4 offsets[])
	{
		std::copy(&this->offsets[0], &this->offsets[14], &offsets[0]);
	}

	std::vector<float> SSAOPass::CalcGaussWeights(float sigma) const
	{
		float twoSigma2 = 2.0f * sigma * sigma;

		// Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
		// For example, for sigma = 3, the width of the bell curve is 
		int blurRadius = static_cast<int>(ceil(2.0f * sigma));

		assert(blurRadius <= MaxBlurRadius);

		std::vector<float> weights;
		weights.resize(2 * blurRadius + 1);

		float weightSum = 0.0f;

		for (int i = -blurRadius; i <= blurRadius; ++i)
		{
			float x = static_cast<float>(i);

			weights[i + blurRadius] = expf(-x * x / twoSigma2);

			weightSum += weights[i + blurRadius];
		}

		// Divide by the sum so all the weights add up to 1.0.
		for (int i = 0; i < weights.size(); ++i)
		{
			weights[i] /= weightSum;
		}

		return weights;
	}

	void SSAOPass::Render(std::shared_ptr<GCommandList> cmdList)
	{
		cmdList->SetRootSignature(ssaoPrimeRootSignature.get());

		cmdList->SetViewports(&viewport, 1);
		cmdList->SetScissorRects(&scissorRect, 1);

		cmdList->TransitionBarrier(ambientMap0, D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmdList->FlushResourceBarriers();


		float clearValue[] = {1.0f, 1.0f, 1.0f, 1.0f};
		cmdList->ClearRenderTarget(&ambientMapRtvMemory, 0, clearValue);

		cmdList->SetRenderTargets(1, &ambientMapRtvMemory, 0);

		cmdList->SetPipelineState(ssaoPSO);


		cmdList->SetRootConstantBufferView(0, *SsaoConstantUploadBuffers[currentFrameResourceIndex].get());
		cmdList->SetRoot32BitConstant(1, 0, 0);

		cmdList->SetDescriptorsHeap(gpass.GetSRV());
		cmdList->SetRootDescriptorTable(2, gpass.GetSRV(), GPass::NormalMap);
		cmdList->SetRootDescriptorTable(3, gpass.GetSRV(), GPass::DepthMap);
		cmdList->SetRootDescriptorTable(4, &randomVectorSrvMemory);


		cmdList->SetVBuffer(0, 0, nullptr);
		cmdList->SetIBuffer(nullptr);
		cmdList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->Draw(6, 1, 0, 0);


		cmdList->TransitionBarrier(ambientMap0, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		cmdList->FlushResourceBarriers();

		BlurAmbientMap(cmdList, 3);
	}

	void SSAOPass::BlurAmbientMap(std::shared_ptr<GCommandList> cmdList,
	                              int blurCount)
	{
		cmdList->SetPipelineState(blurPSO);

		cmdList->SetRootConstantBufferView(0, *SsaoConstantUploadBuffers[currentFrameResourceIndex].get());

		for (int i = 0; i < blurCount; ++i)
		{
			BlurAmbientMap(cmdList, true);
			BlurAmbientMap(cmdList, false);
		}
	}


	void SSAOPass::BlurAmbientMap(std::shared_ptr<GCommandList> cmdList, bool horzBlur)
	{
		GTexture output;
		size_t inputSrv;
		size_t outputRtv;

		if (horzBlur == true)
		{
			output = ambientMap1;
			inputSrv = 0;
			outputRtv = 1;
			cmdList->SetRoot32BitConstant(1, 1, 0);
		}
		else
		{
			output = ambientMap0;
			inputSrv = 1;
			outputRtv = 0;
			cmdList->SetRoot32BitConstant(1, 0, 0);
		}

		cmdList->TransitionBarrier(output, D3D12_RESOURCE_STATE_RENDER_TARGET);
		cmdList->FlushResourceBarriers();

		float clearValue[] = {1.0f, 1.0f, 1.0f, 1.0f};
		cmdList->ClearRenderTarget(&ambientMapRtvMemory, outputRtv, clearValue);

		cmdList->SetRenderTargets(1, &ambientMapRtvMemory, outputRtv);

		cmdList->SetDescriptorsHeap(gpass.GetSRV());

		cmdList->SetRootDescriptorTable(2, gpass.GetSRV(), GPass::NormalMap);
		cmdList->SetRootDescriptorTable(3, gpass.GetSRV(), GPass::DepthMap);

		cmdList->SetDescriptorsHeap(&ambientMapMapSrvMemory);
		
		cmdList->SetRootDescriptorTable(4, &ambientMapMapSrvMemory, inputSrv);

		// Draw fullscreen quad.
		cmdList->SetVBuffer(0, 0, nullptr);
		cmdList->SetIBuffer(nullptr);
		cmdList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmdList->Draw(6, 1, 0, 0);

		cmdList->TransitionBarrier(output, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		cmdList->FlushResourceBarriers();
	}

	void SSAOPass::Update()
	{
		currentFrameResourceIndex = (currentFrameResourceIndex + 1) % globalCountFrameResources;

		SsaoConstants ssaoCB;

		auto camera = Camera::mainCamera;

		auto cameraData = camera->GetCameraData();

		auto P = camera->GetProjectionMatrix();

		// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
		Matrix T(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		ssaoCB.Proj = cameraData.Proj;
		ssaoCB.InvProj = cameraData.InvProj;
		XMStoreFloat4x4(&ssaoCB.ProjTex, XMMatrixTranspose(P * T));

		{
			GetOffsetVectors(ssaoCB.OffsetVectors);

			auto blurWeights = CalcGaussWeights(2.5f);
			ssaoCB.BlurWeights[0] = Vector4(&blurWeights[0]);
			ssaoCB.BlurWeights[1] = Vector4(&blurWeights[4]);
			ssaoCB.BlurWeights[2] = Vector4(&blurWeights[8]);

			ssaoCB.InvRenderTargetSize = Vector2(1.0f / SsaoMapWidth(),
			                                     1.0f / SsaoMapHeight());

			// Coordinates given in view space.
			ssaoCB.OcclusionRadius = 0.5f;
			ssaoCB.OcclusionFadeStart = 0.2f;
			ssaoCB.OcclusionFadeEnd = 1.0f;
			ssaoCB.SurfaceEpsilon = 0.05f;

			SsaoConstantUploadBuffers[currentFrameResourceIndex]->CopyData(0, ssaoCB);
		}
	}

	void SSAOPass::ChangeRenderTargetSize(float width, float height)
	{
	}
}
