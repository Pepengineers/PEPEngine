#pragma once
#include <DirectXBuffers.h>
#include <GDevice.h>
#include "ShaderBuffersData.h"

namespace SimpleRender
{
	using namespace PEPEngine;
	using namespace Graphics;
	using namespace Utils;
	using namespace Allocator;
	using namespace Common;

	struct FrameResource
	{
		FrameResource(std::shared_ptr<GDevice> device, UINT passCount, UINT materials, UINT lights);
		FrameResource(const FrameResource& rhs) = delete;
		FrameResource& operator=(const FrameResource& rhs) = delete;

		~FrameResource() {};

		UINT64 FenceValue = 0;

		std::shared_ptr<ConstantBuffer<PassConstants>> PassConstantBuffer = nullptr;
		std::shared_ptr<UploadBuffer<MaterialData>> MaterialsBuffer = nullptr;
		std::shared_ptr<UploadBuffer<LightData>> LightsBuffer = nullptr;
	};
}