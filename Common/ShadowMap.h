#pragma once

#include "d3dUtil.h"
#include "GMemory.h"
#include "GTexture.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;
		using namespace Utils;
		using namespace Graphics;
		using namespace Microsoft::WRL;

		class ShadowMap
		{
		public:
			ShadowMap(std::shared_ptr<GDevice> device, UINT width, UINT height);

			ShadowMap(const ShadowMap& rhs) = delete;
			ShadowMap& operator=(const ShadowMap& rhs) = delete;
			~ShadowMap() = default;

			UINT Width() const;
			UINT Height() const;
			GTexture& GetTexture();

			GMemory* GetSrvMemory()
			{
				return &srvMemory;
			}

			GMemory* GetDsvMemory()
			{
				return &dsvMemory;
			}

			D3D12_VIEWPORT Viewport() const;
			D3D12_RECT ScissorRect() const;

			void BuildDescriptors();

			void OnResize(UINT newWidth, UINT newHeight);

		private:

			void BuildResource();

		private:
			std::shared_ptr<GDevice> device;

			D3D12_VIEWPORT mViewport;
			D3D12_RECT mScissorRect;

			UINT mWidth = 0;
			UINT mHeight = 0;
			DXGI_FORMAT mFormat = DXGI_FORMAT_R24G8_TYPELESS;

			GMemory srvMemory;

			GMemory dsvMemory;

			GTexture shadowMap;
		};
	}
}
