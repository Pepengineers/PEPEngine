#include "pch.h"
#include "ModelRenderer.h"
#include "GameObject.h"
#include "GCommandList.h"
#include "GMesh.h"
#include "GModel.h"
#include "Transform.h"

namespace PEPEngine::Common
{
	void ModelRenderer::PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList, UINT meshIndex)
	{
		cmdList->SetRootConstantBufferView(ObjectDataBuffer, *modelDataBuffer.get(), meshIndex);
		const auto mesh = model->GetMesh(meshIndex);
		mesh->Render(cmdList);
	}

	void ModelRenderer::Update()
	{
		const auto transform = gameObject->GetTransform();

		if (transform->IsDirty())
		{
			modelWorldData.TextureTransform = transform->TextureTransform.Transpose();
			modelWorldData.World = (transform->GetWorldMatrix() * model->scaleMatrix).Transpose();
						
			for (int i = 0; i < materials.size(); ++i)
			{
				auto material = materials[i];

				if (material != nullptr)
				{
					modelWorldData.MaterialIndex = material->GetMaterialIndex();
					modelDataBuffer->CopyData(i, modelWorldData);
				}
			}
		}
	}

	ModelRenderer::ModelRenderer(std::shared_ptr<GModel> model) : Renderer()
	{
		SetModel(model);
	}

	void ModelRenderer::SetModel(std::shared_ptr<GModel> asset)
	{
		assert(asset != nullptr);
		
		if (modelDataBuffer == nullptr || modelDataBuffer->GetElementCount() < asset->GetMeshesCount())
		{
			modelDataBuffer.reset();
			modelDataBuffer = std::make_shared<ConstantUploadBuffer<ObjectConstants>>(asset->GetDevice(), asset->GetMeshesCount(), asset->GetName());

			if (materials.size() < asset->GetMeshesCount())
			{
				materials.resize(asset->GetMeshesCount());
			}
		}

		model = asset;
	}

	UINT ModelRenderer::GetMeshCount()
	{
		return model->GetMeshesCount();
	}

	std::shared_ptr<GMesh> ModelRenderer::GetMesh(UINT index)
	{
		return model->GetMesh(index);
	}
}
