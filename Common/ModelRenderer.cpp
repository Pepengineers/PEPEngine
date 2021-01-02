#include "pch.h"
#include "ModelRenderer.h"
#include "GameObject.h"
#include "GCommandList.h"
#include "GMesh.h"
#include "GModel.h"
#include "Transform.h"

namespace PEPEngine::Common
{
	void ModelRenderer::PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList)
	{
		for (int i = 0; i < model->GetMeshesCount(); ++i)
		{
			model->meshesMaterials[i]->Draw(cmdList);

			cmdList->SetRootConstantBufferView(ObjectDataBuffer,
			                                   *modelDataBuffer, i);

			const auto mesh = model->GetMesh(i);
			mesh->Draw(cmdList);
		}
	}

	void ModelRenderer::Update()
	{
		const auto transform = gameObject->GetTransform();

		if (transform->IsDirty())
		{
			objectWorldData.TextureTransform = transform->TextureTransform.Transpose();
			objectWorldData.World = (transform->GetWorldMatrix() * model->scaleMatrix).Transpose();
			for (int i = 0; i < model->GetMeshesCount(); ++i)
			{
				objectWorldData.MaterialIndex = model->GetMeshMaterial(i)->GetMaterialIndex();
				modelDataBuffer->CopyData(i, objectWorldData);
			}
		}
	}

	ModelRenderer::ModelRenderer(const std::shared_ptr<GDevice> device,
	                             std::shared_ptr<GModel> model) : Renderer(), device(device), model(model)
	{
		SetModel(model);
	}

	void ModelRenderer::SetModel(std::shared_ptr<GModel> asset)
	{
		if (modelDataBuffer == nullptr || modelDataBuffer->GetElementCount() < asset->GetMeshesCount())
		{
			modelDataBuffer.reset();
			modelDataBuffer = std::make_shared<ConstantUploadBuffer<ObjectConstants>>(
				device, asset->GetMeshesCount(), asset->GetName());
		}

		model = asset;
	}
}
