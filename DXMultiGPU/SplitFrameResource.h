#pragma once

#include "DirectXBuffers.h"
#include "GCrossAdapterResource.h"
#include "ShaderBuffersData.h"
#include "GDeviceFactory.h"
#include "GMemory.h"
#include "GTexture.h"

struct SplitFrameResource
{
public:

	SplitFrameResource(std::shared_ptr<DX::Graphics::GDevice>* devices, UINT deviceCount, UINT passCount,
	                   UINT materialCount);
	SplitFrameResource(const SplitFrameResource& rhs) = delete;
	SplitFrameResource& operator=(const SplitFrameResource& rhs) = delete;
	~SplitFrameResource();


	std::shared_ptr<DX::Graphics::GCrossAdapterResource> CrossAdapterBackBuffer = nullptr;

	DX::Graphics::GTexture PrimeDeviceBackBuffer;

	DX::Allocator::custom_vector<DX::Graphics::GMemory> RenderTargetViewMemory =
		DX::Allocator::MemoryAllocator::CreateVector<DX::Graphics::GMemory>();

	DX::Allocator::custom_vector<std::shared_ptr<DX::Graphics::ConstantBuffer<DX::Common::PassConstants>>>
	PassConstantBuffers = DX::Allocator::MemoryAllocator::CreateVector<std::shared_ptr<DX::Graphics::ConstantBuffer<
		DX::Common::PassConstants>>>();

	DX::Allocator::custom_vector<std::shared_ptr<DX::Graphics::ConstantBuffer<DX::Common::SsaoConstants>>>
	SsaoConstantBuffers = DX::Allocator::MemoryAllocator::CreateVector<std::shared_ptr<DX::Graphics::ConstantBuffer<
		DX::Common::SsaoConstants>>>();

	DX::Allocator::custom_vector<std::shared_ptr<DX::Graphics::UploadBuffer<DX::Common::MaterialData>>>
	MaterialBuffers = DX::Allocator::MemoryAllocator::CreateVector<std::shared_ptr<DX::Graphics::UploadBuffer<
		DX::Common::MaterialData>>>();


	UINT64 FenceValue = 0;
};
