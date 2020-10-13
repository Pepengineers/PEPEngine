#include "FrameResource.h"


FrameResource::FrameResource(std::shared_ptr<DX::Graphics::GDevice> primeDevice, UINT passCount, UINT objectCount,
                             UINT materialCount)
{
	PassConstantBuffer = std::make_unique<DX::Graphics::ConstantBuffer<DX::Common::PassConstants>>(
		primeDevice, passCount, L"Forward Path Data");
	SsaoConstantBuffer = std::make_unique<DX::Graphics::ConstantBuffer<DX::Common::SsaoConstants>>(
		primeDevice, 1, L"SSAO Path Data");
	MaterialBuffer = std::make_unique<DX::Graphics::UploadBuffer<DX::Common::MaterialConstants>>(
		primeDevice, materialCount, L"Materials Data");
}

FrameResource::~FrameResource()
{
}
