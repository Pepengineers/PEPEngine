#include "FrameResource.h"

FrameResource::FrameResource(std::shared_ptr<GDevice> device, UINT passCount)
{
	PassConstantBuffer = std::make_shared<ConstantBuffer<PassConstants>>(device, passCount, L"Frame Path Constnats");
}