#include "FrameResource.h"


PEPEngine::Common::FrameResource::FrameResource(std::shared_ptr<GDevice> device, UINT materials, UINT lights)
{
	WorldBuffer = std::make_shared<ConstantUploadBuffer<WorldData>>(device, 1, L"World Data Buffer");

	MaterialsBuffer = std::make_shared<StructuredUploadBuffer<MaterialData>>(
		device, materials, L"Materials Data UploadBuffer");

	LightsBuffer = std::make_shared<StructuredUploadBuffer<LightData>>(
		device, lights, L"lights dat upload buffer");
}