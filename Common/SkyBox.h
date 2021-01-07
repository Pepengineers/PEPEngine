#pragma once
#include "ModelRenderer.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;
		using namespace Utils;
		using namespace Graphics;

		class SkyBox : public ModelRenderer
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE gpuTextureHandle{};
			CD3DX12_CPU_DESCRIPTOR_HANDLE cpuTextureHandle{};

		public:
			SkyBox(const std::shared_ptr<AModel>& model,
			       GTexture& skyMapTexture,
			       GDescriptor* srvMemory, UINT offset = 0);


			void Render(std::shared_ptr<GCommandList> cmdList);
			
		protected:
			void Update() override;;;
		};
	}
}
