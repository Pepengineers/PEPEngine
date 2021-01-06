#include "pch.h"
#include "ModelRenderer.h"

#include "AssetDatabase.h"
#include "GameObject.h"
#include "GCommandList.h"
#include "GMesh.h"
#include "GModel.h"
#include "Transform.h"

namespace PEPEngine::Common
{
	void ModelRenderer::Serialize(json& j)
	{
		j["Type"] = ComponentID;

		auto jPos = json(); 
		jPos["ModelID"] = model->GetName().size();

		j["RendererData"] = jPos;
	};

	void ModelRenderer::Deserialize(json& j)
	{
		auto jPos =  j["RendererData"];

		UINT64 name = jPos["ModelID"];

		auto asset = AssetDatabase::FindAssetByID(name);

		//SetModel(asset);		
	};
	
	void ModelRenderer::PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList, UINT meshIndex)
	{
		cmdList->SetRootConstantBufferView(ObjectWorldDataBuffer, *modelDataBuffer.get(), meshIndex);
		
		model->Render(cmdList, meshIndex);		
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

	void ModelRenderer::SetMaterial(std::shared_ptr<Material> material, UINT slot)
	{
		assert(slot < materials.size());
		materials[slot] = material;
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
			materials = asset->GetMaterials();			
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

	std::vector<std::shared_ptr<Material>>& ModelRenderer::GetSharedMaterials()
	{
		return materials;
	}
}
