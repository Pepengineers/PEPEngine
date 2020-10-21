#include "pch.h"
#include "GMesh.h"
#include <DirectXMesh.h>
#include "GBuffer.h"
#include "GCommandList.h"
#include "NativeModel.h"

namespace PEPEngine
{
	namespace Common
	{
		std::shared_ptr<NativeMesh> GMesh::GetMeshData() const
		{
			return mesh;
		}

		D3D12_PRIMITIVE_TOPOLOGY GMesh::GetPrimitiveType() const
		{
			return mesh->topology;
		}


		GMesh::GMesh(std::shared_ptr<NativeMesh> meshData, std::shared_ptr<GCommandList>& cmdList) : mesh(meshData)
		{
			indexBuffer = std::make_shared<GBuffer>(std::move(GBuffer::CreateBuffer(
				cmdList, mesh->GetIndexes().data(), mesh->GetIndexSize(), mesh->GetIndexes().size(),
				mesh->GetName() + L" Indexes")));

			vertexBuffer = std::make_shared<GBuffer>(std::move(GBuffer::CreateBuffer(
				cmdList, mesh->GetVertexes().data(), mesh->GetVertexSize(), mesh->GetVertexes().size(),
				mesh->GetName() + L" Vertexes")));

			
		}


		GMesh::GMesh(const GMesh& copy) : mesh(copy.mesh),
		                                  vertexBuffer(copy.vertexBuffer), indexBuffer(copy.indexBuffer)
		{
		}

		void GMesh::Draw(std::shared_ptr<GCommandList> cmdList) const
		{
			cmdList->SetVBuffer(0, 1, vertexBuffer->VertexBufferView());
			cmdList->SetIBuffer(indexBuffer->IndexBufferView());
			cmdList->SetPrimitiveTopology(mesh->topology);
			cmdList->DrawIndexed(mesh->GetIndexCount());
		}

		std::wstring GMesh::GetName() const
		{
			return mesh->GetName();
		}
	}
}
