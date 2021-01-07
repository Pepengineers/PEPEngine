#include "pch.h"
#include "ModelRenderer.h"

#include "AssetDatabase.h"
#include "GameObject.h"
#include "GCommandList.h"
#include "GMesh.h"
#include "GModel.h"
#include "Transform.h"
#include "AModel.h"
#include "AMaterial.h"
#include "Material.h"

namespace PEPEngine::Common
{
	void ModelRenderer::Serialize(json& j)
	{
		j["Type"] = ComponentID;

		auto jPos = json(); 
		jPos["ModelID"] = model->GetID();

		j["RendererData"] = jPos;
	};

	void ModelRenderer::Deserialize(json& j)
	{
		auto jPos =  j["RendererData"];

		const UINT64 id = jPos["ModelID"];

		const auto asset = AssetDatabase::FindAssetByID<AModel>(id);

		SetModel(asset);		
	};
	
	void ModelRenderer::PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList, UINT meshIndex)
	{
		cmdList->SetRootConstantBufferView(ObjectWorldDataBuffer, *modelDataBuffer.get(), meshIndex);
		
		model->GetGModel()->Render(cmdList, meshIndex);		
	}

	void ModelRenderer::Update()
	{
		const auto transform = gameObject->GetTransform();

		if (transform->IsDirty())
		{
			modelWorldData.TextureTransform = transform->TextureTransform.Transpose();
			modelWorldData.World = (transform->GetWorldMatrix() * model->GetGModel()->scaleMatrix).Transpose();
						
			for (int i = 0; i < materials.size(); ++i)
			{
				auto material = materials[i];

				if (material != nullptr)
				{
					modelWorldData.MaterialIndex = material->GetMaterial()->GetMaterialIndex();
					modelDataBuffer->CopyData(i, modelWorldData);
				}
			}
		}
	}

	void ModelRenderer::SetMaterial(std::shared_ptr<AMaterial> material, UINT slot)
	{
		assert(slot < materials.size());
		materials[slot] = material;
	}

	ModelRenderer::ModelRenderer(std::shared_ptr<AModel> model) : Renderer()
	{
		SetModel(model);
	}

	void ModelRenderer::SetModel(std::shared_ptr<AModel> asset)
	{
		assert(asset != nullptr);
		
		if (modelDataBuffer == nullptr || modelDataBuffer->GetElementCount() < asset->GetGModel()->GetMeshesCount())
		{
			modelDataBuffer.reset();
			modelDataBuffer = std::make_shared<ConstantUploadBuffer<ObjectConstants>>(asset->GetGModel()->GetDevice(), asset->GetGModel()->GetMeshesCount(), AnsiToWString( asset->GetGModel()->GetName()));
			materials = asset->GetGModel()->GetMaterials();	
		}

		model = asset;
	}

	UINT ModelRenderer::GetMeshCount()
	{
		return model->GetGModel()->GetMeshesCount();
	}

	std::shared_ptr<GMesh> ModelRenderer::GetMesh(UINT index)
	{
		return model->GetGModel()->GetMesh(index);
	}

	std::vector<std::shared_ptr<AMaterial>>& ModelRenderer::GetSharedMaterials()
	{
		
		return materials;
	}
}
