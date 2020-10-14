#include "FrameResource.h"

FrameResource::FrameResource(std::shared_ptr<DX::Graphics::GDevice> device, UINT passCount,  UINT materials, UINT lights)
{
	PassConstantBuffer = std::make_shared<DX::Graphics::ConstantBuffer<DX::Common::PassConstants>>(
		device, passCount, L"Frame Path Constnats");

	MaterialsBuffer = std::make_shared<DX::Graphics::UploadBuffer<DX::Common::MaterialData>>(
		device, materials, L"Materials Data UploadBuffer");

	LightsBuffer = std::make_shared<DX::Graphics::UploadBuffer<DX::Common::LightData>>(
		device, lights, L"lights dat upload buffer");
}
