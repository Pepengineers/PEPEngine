#include "pch.h"
#include "SkyBox.h"
#include "GameObject.h"
#include "GCommandList.h"
#include "GDescriptor.h"
#include "GMesh.h"
#include "GModel.h"
#include "Transform.h"

namespace PEPEngine::Common
{
	SkyBox::SkyBox(const std::shared_ptr<GModel>& model,
	               GTexture& skyMapTexture,
	               GDescriptor* srvMemory, UINT offset) : ModelRenderer(model)
	{
		gpuTextureHandle = srvMemory->GetGPUHandle(offset);
		cpuTextureHandle = srvMemory->GetCPUHandle(offset);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		auto desc = skyMapTexture.GetD3D12Resource()->GetDesc();
		srvDesc.Format = GetSRGBFormat(desc.Format);
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = desc.MipLevels;
		srvDesc.TextureCube.MostDetailedMip = 0;
		skyMapTexture.CreateShaderResourceView(&srvDesc, srvMemory, offset);
	}

	void SkyBox::Render(std::shared_ptr<GCommandList> cmdList)
	{
		cmdList->GetGraphicsCommandList()->SetGraphicsRootDescriptorTable(
			SkyMap, gpuTextureHandle);

		for (int i = 0; i < model->GetMeshesCount(); ++i)
		{
			const auto mesh = model->GetMesh(i);

			cmdList->SetRootConstantBufferView(ObjectDataBuffer,
			                                   *modelDataBuffer, i);
			mesh->Render(cmdList);
		}
	}

	void SkyBox::Update()
	{
		const auto transform = gameObject->GetTransform();

		if (transform->IsDirty())
		{
			modelWorldData.TextureTransform = transform->TextureTransform.Transpose();
			modelWorldData.World = (transform->GetWorldMatrix() * model->scaleMatrix).Transpose();
			for (int i = 0; i < model->GetMeshesCount(); ++i)
			{
				modelDataBuffer->CopyData(i, modelWorldData);
			}
		}
	}
}
