#include "pch.h"
#include "ModelRenderer.h"
#include "GameObject.h"
#include "GCommandList.h"
#include "GDeviceFactory.h"
#include "Transform.h"
#include "Material.h"
#include "GModel.h"

void ModelRenderer::Draw(std::shared_ptr<GCommandList> cmdList)
{
	if (material != nullptr)
		material->Draw(cmdList);

	if(model != nullptr)
	{
		for (int i = 0; i < model->GetMeshesCount(); ++i)
		{
			const auto mesh = model->GetMesh(i);
			cmdList->SetRootConstantBufferView(StandardShaderSlot::ObjectData,
				*modelDataBuffer, i);
			cmdList->SetRootConstantBufferView(StandardShaderSlot::MaterialData,
				*materialsDataBuffer, i);
			cmdList->SetVBuffer(0, 1, mesh->GetVertexView());
			cmdList->SetIBuffer(mesh->GetIndexView());
			cmdList->SetPrimitiveTopology(mesh->GetPrimitiveType());
			cmdList->DrawIndexed(mesh->GetIndexCount());
		}		
	}
}

void ModelRenderer::Update()
{
	auto transform = gameObject->GetTransform();

	if (transform->IsDirty())
	{
		if (model == nullptr) return;

		constantData.TextureTransform = transform->TextureTransform.Transpose();
		constantData.World = (transform->GetWorldMatrix() * model->scaleMatrix).Transpose();
		for (int i = 0; i < model->GetMeshesCount(); ++i)
		{
			constantData.MaterialIndex = meshesMaterials[i]->GetIndex();
			modelDataBuffer->CopyData(i, constantData);
			materialsDataBuffer->CopyData(i, meshesMaterials[i]->GetMaterialConstantData());
		}		
	}
}

void ModelRenderer::SetModel(std::shared_ptr<GModel> asset)
{
	if(meshesMaterials.size() < asset->GetMeshesCount())
	{
		meshesMaterials.resize(asset->GetMeshesCount());
	}
	
	if(modelDataBuffer == nullptr || modelDataBuffer->GetElementCount() < asset->GetMeshesCount() )
	{
		modelDataBuffer.reset();
		materialsDataBuffer.reset();
		modelDataBuffer = std::make_unique<ConstantBuffer<ObjectConstants>>(GDeviceFactory::GetDevice() , asset->GetMeshesCount(), asset->GetName());
		materialsDataBuffer = std::make_unique<ConstantBuffer<MaterialConstants>>(GDeviceFactory::GetDevice(), asset->GetMeshesCount(), asset->GetName() + L"Materials");
	}
	
	model = asset;
}

UINT ModelRenderer::GetMeshesCount() const
{
	return model->GetMeshesCount();
}

void ModelRenderer::SetMeshMaterial(UINT index, const std::shared_ptr<Material> material)
{
	meshesMaterials[index] = material;	
}
