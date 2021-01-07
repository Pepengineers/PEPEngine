#pragma once
#include <d3d12.h>
#include <string>

#include "GBuffer.h"
#include "GCommandList.h"
#include "GMeshBuffer.h"
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

			std::shared_ptr<GMeshBuffer> vertexBuffer = nullptr;
			std::shared_ptr<GMeshBuffer> indexBuffer = nullptr;
		public:

			std::shared_ptr<NativeMesh> GetMeshData() const;

			D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveType() const;

			D3D12_VERTEX_BUFFER_VIEW* GetVertexView() const;

			D3D12_INDEX_BUFFER_VIEW* GetIndexView() const;

			GMesh(const std::shared_ptr<NativeMesh>& data, std::shared_ptr<GCommandList>& cmdList);

			void Render(std::shared_ptr<GCommandList> cmdList) const;

			std::string GetName() const;
		};
	}
}
