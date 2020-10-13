#pragma once

#include "DirectXBuffers.h"
#include "GDeviceFactory.h"
#include "ShaderBuffersData.h"


struct FrameResource
{
public:

	FrameResource(std::shared_ptr<DX::Graphics::GDevice> primeDevice, UINT passCount, UINT objectCount, UINT materialCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	ComPtr<ID3D12Resource> crossAdapterResources[DX::Graphics::GraphicAdapterCount][DX::Utils::globalCountFrameResources];
	
	std::unique_ptr<DX::Graphics::ConstantBuffer<DX::Common::PassConstants>> PassConstantBuffer = nullptr;
	std::unique_ptr<DX::Graphics::ConstantBuffer<DX::Common::SsaoConstants>> SsaoConstantBuffer = nullptr;
	std::unique_ptr<DX::Graphics::UploadBuffer<DX::Common::MaterialConstants>> MaterialBuffer = nullptr;

	UINT64 FenceValue = 0;
};
