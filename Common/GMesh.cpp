#include "pch.h"
#include "GMesh.h"
#include <DirectXMesh.h>

#include "d3dUtil.h"
#include "GBuffer.h"
#include "GCommandList.h"
#include "GMeshBuffer.h"
#include "NativeModel.h"
#include "Utils.h"

using namespace PEPEngine::Utils;

namespace PEPEngine::Common
{
	std::shared_ptr<NativeMesh> GMesh::GetMeshData() const
	{
		return mesh;
	}

	D3D12_PRIMITIVE_TOPOLOGY GMesh::GetPrimitiveType() const
	{
		return mesh->topology;
	}

	D3D12_VERTEX_BUFFER_VIEW* GMesh::GetVertexView() const
	{
		return vertexBuffer->VertexBufferView();
	}

	D3D12_INDEX_BUFFER_VIEW* GMesh::GetIndexView() const
	{
		return indexBuffer->IndexBufferView();
	}

	GMesh::GMesh(const std::shared_ptr<NativeMesh>& data, std::shared_ptr<GCommandList>& cmdList) : mesh(
		std::move(data))
	{
		indexBuffer = std::make_shared<GMeshBuffer>(std::move(GMeshBuffer::CreateBuffer(
			cmdList, mesh->GetIndexes().data(), mesh->GetIndexSize(), mesh->GetIndexes().size(), AnsiToWString(
			  mesh->GetName() + " Indexes"))));

		vertexBuffer = std::make_shared<GMeshBuffer>(std::move(GMeshBuffer::CreateBuffer(
			cmdList, mesh->GetVertexes().data(), mesh->GetVertexSize(), mesh->GetVertexes().size(), AnsiToWString(
			mesh->GetName() + " Vertexes"))));
	}


	void GMesh::Render(std::shared_ptr<GCommandList> cmdList) const
	{
		cmdList->SetVBuffer(0, 1, vertexBuffer->VertexBufferView());
		cmdList->SetIBuffer(indexBuffer->IndexBufferView());
		cmdList->SetPrimitiveTopology(mesh->topology);
		cmdList->DrawIndexed(mesh->GetIndexCount());
	}

	std::string GMesh::GetName() const
	{
		return mesh->GetName();
	}
}
