#include "FrameResource.h"

FrameResource::FrameResource(std::shared_ptr<DX::Graphics::GDevice> device, UINT passCount)
{
	PassConstantBuffer = std::make_shared<DX::Graphics::ConstantBuffer<DX::Common::PassConstants>>(
		device, passCount, L"Frame Path Constnats");
}
