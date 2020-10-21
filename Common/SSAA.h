#pragma once
#include "GDescriptor.h"
#include "GTexture.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace Utils;
		using namespace Allocator;
		using namespace Graphics;

		class SSAA
		{
			GTexture renderTarget;
			GTexture depthMap;

			GDescriptor srvMemory;
			GDescriptor rtvMemory;
			GDescriptor dsvMemory;

			UINT ResolutionMultiplier = 1;
			const DXGI_FORMAT rtvFormat = GetSRGBFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
			const DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

			D3D12_VIEWPORT viewport{};
			D3D12_RECT scissorRect{};

			std::shared_ptr<GDevice> device;

		public:

			D3D12_VIEWPORT GetViewPort() const;

			D3D12_RECT GetRect() const;


			GTexture& GetRenderTarget();

			GTexture& GetDepthMap();

			GDescriptor* GetRTV();

			GDescriptor* GetSRV();

			GDescriptor* GetDSV();

			void OnResize(UINT newWidth, UINT newHeight);

			SSAA(std::shared_ptr<GDevice> device, UINT multiplier, UINT width, UINT height);
		};
	}
}
