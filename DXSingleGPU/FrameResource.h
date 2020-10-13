#pragma once
#include "DirectXBuffers.h"
#include "ShaderBuffersData.h"


struct FrameResource
{
public:

	FrameResource(std::shared_ptr<DX::Graphics::GDevice> primeDevice, UINT passCount, UINT objectCount,
	              UINT materialCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	std::shared_ptr<DX::Graphics::ConstantBuffer<DX::Common::PassConstants>> PassConstantBuffer = nullptr;
	std::shared_ptr<DX::Graphics::ConstantBuffer<DX::Common::SsaoConstants>> SsaoConstantBuffer = nullptr;
	std::shared_ptr<DX::Graphics::UploadBuffer<DX::Common::MaterialConstants>> MaterialBuffer = nullptr;

	UINT64 FenceValue = 0;
};
