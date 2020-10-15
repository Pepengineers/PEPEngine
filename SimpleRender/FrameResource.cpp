#include "FrameResource.h"


SimpleRender::FrameResource::FrameResource(std::shared_ptr<GDevice> device, UINT passCount, UINT materials, UINT lights)
{
	PassConstantBuffer = std::make_shared<ConstantBuffer<PassConstants>>(device, passCount, L"World Data Buffer");

	MaterialsBuffer = std::make_shared<PEPEngine::Graphics::UploadBuffer<PEPEngine::Common::MaterialData>>(
		device, materials, L"Materials Data UploadBuffer");

	LightsBuffer = std::make_shared<PEPEngine::Graphics::UploadBuffer<PEPEngine::Common::LightData>>(
		device, lights, L"lights dat upload buffer");
}
