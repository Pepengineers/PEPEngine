#pragma once
#include <d3d12.h>
#include <string>

#include "GBuffer.h"
#include "GCommandList.h"
#include "Lazy.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace Utils;
		using namespace Allocator;
		using namespace Graphics;
		class NativeMesh;
		class GModel;

		class GMesh
		{
			friend GModel;

			std::shared_ptr<NativeMesh> mesh;


			std::shared_ptr<GBuffer> vertexBuffer = nullptr;
			std::shared_ptr<GBuffer> indexBuffer = nullptr;
		public:

			std::shared_ptr<NativeMesh> GetMeshData() const;

			D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveType() const;


			GMesh(std::shared_ptr<NativeMesh> meshData, std::shared_ptr<GCommandList>& cmdList);

			GMesh(const GMesh& copy);

			void Draw(std::shared_ptr<GCommandList> cmdList) const;

			std::wstring GetName() const;
		};
	}
}
