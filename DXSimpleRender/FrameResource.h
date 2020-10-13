#pragma once
#include "d3dUtil.h"
#include "DirectXBuffers.h"
#include "ShaderBuffersData.h"

struct FrameResource
{
	FrameResource(std::shared_ptr<DX::Graphics::GDevice> device, UINT passCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource() {};
	
	UINT64 FenceValue = 0;

	std::shared_ptr<DX::Graphics::ConstantBuffer<DX::Common::PassConstants>> PassConstantBuffer = nullptr;
};

