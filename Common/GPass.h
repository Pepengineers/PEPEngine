#pragma once
#include "Camera.h"
#include "RenderPass.h"

namespace PEPEngine::Common
{
	class GPass :
		public RenderPass
	{
		static const UINT GBufferMapsCount = 4;
		static const DXGI_FORMAT NormalMapFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		static const DXGI_FORMAT BaseColorMapFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		static const DXGI_FORMAT PositionMapFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		static const DXGI_FORMAT DepthMapFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		custom_vector<GTexture> gbuffers = MemoryAllocator::CreateVector<GTexture>();
		GShader gpassVertexShader;
		GShader gpassPixelShader;


		GraphicPSO deferredPSO;
		GDescriptor deferredRTVDescriptor;
		GDescriptor deferredSRVDescriptor;
		GDescriptor deferredDSVDescriptor;

		std::shared_ptr<GDevice> device;
		GRootSignature rootSign;

		void AllocateDescriptors();

		void InitRootSignature();

		void InitPSO();


	public:

		enum DeferredPassRTVSlot: UINT
		{
			NormalMap = 0,
			ColorMap,
			PositionMap,
			DepthMap
		};

		GPass(std::shared_ptr<GDevice> renderDevice);

		GTexture& GetGTexture(DeferredPassRTVSlot slot);

		GDescriptor& GetSRV();


		void Render(std::shared_ptr<GCommandList> cmdList) override;

		void OnResize() override;;
	};
}
