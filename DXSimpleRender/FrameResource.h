#pragma once
#include "d3dUtil.h"
#include "DirectXBuffers.h"
#include "ShaderBuffersData.h"

struct LightIndex
{
	UINT index;
};

struct FrameResource
{
	FrameResource(std::shared_ptr<DX::Graphics::GDevice> device, UINT passCount, UINT materials, UINT lights);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;

	~FrameResource()
	{
	};

	UINT64 FenceValue = 0;

	std::shared_ptr<DX::Graphics::ConstantBuffer<DX::Common::PassConstants>> PassConstantBuffer = nullptr;
	std::shared_ptr<DX::Graphics::UploadBuffer<DX::Common::MaterialData>> MaterialsBuffer = nullptr;
	std::shared_ptr<DX::Graphics::UploadBuffer<DX::Common::LightData>> LightsBuffer = nullptr;
};
