#include "pch.h"
#include "GModel.h"
#include "AMaterial.h"

#include "DirectXBuffers.h"
#include "GMesh.h"
#include "NativeModel.h"
#include "AssetDatabase.h"
#include "MathHelper.h"

namespace PEPEngine::Common
{
	void GModel::SetMaterial(std::shared_ptr<AMaterial> material, UINT slot)
	{
		materials[slot] = material;
	}

	std::vector<std::shared_ptr<AMaterial>>& GModel::GetMaterials()
	{
		return materials;
	}

	std::shared_ptr<GDevice> GModel::GetDevice() const
	{
		return device;
	}

	UINT GModel::GetMeshesCount() const
	{
		return model->GetMeshesCount();
	}

	std::shared_ptr<GMesh> GModel::GetMesh(UINT index)
	{
		return gmeshes[index];
	}

	std::vector<std::shared_ptr<GMesh>> GModel::GetMeshes() const
	{
		return gmeshes;
	}

	std::string GModel::GetName() const
	{
		return model->GetName();
	}

	GModel:: GModel(std::shared_ptr<NativeModel> model, std::shared_ptr<GCommandList> uploadCmdList): model(model)
	{
		Vector3 minPos(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
		Vector3 maxPos(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);
		
		for (int i = 0; i < model->GetMeshesCount(); ++i)
		{
			auto nativeMesh = model->GetMesh(i);
			for (auto&& vertex : nativeMesh->GetVertexes())
			{
				minPos = XMVectorMin(minPos, vertex.Position);
				maxPos = XMVectorMax(maxPos, vertex.Position);
			}
			
			gmeshes.push_back(std::make_shared<GMesh>(nativeMesh, uploadCmdList));
		}

		materials.resize(model->GetMeshesCount());		
		device = uploadCmdList->GetDevice();

		XMStoreFloat3(&Bounds.Center, 0.5f * (minPos + maxPos));
		XMStoreFloat3(&Bounds.Extents, 0.5f * (maxPos - minPos));
	}


	GModel::GModel(const GModel& copy) : model(copy.model), Bounds(copy.Bounds)
	{
		gmeshes.resize(copy.gmeshes.size());

		for (int i = 0; i < gmeshes.size(); ++i)
		{
			gmeshes[i] = std::move(std::make_shared<GMesh>(*copy.gmeshes[i]));
		}
	}

	GModel::~GModel() = default;

	void GModel::Render(std::shared_ptr<GCommandList> cmdList, UINT meshIndex)
	{		
		gmeshes[meshIndex]->Render(cmdList);		
	}

	std::shared_ptr<GModel> GModel::Dublicate(std::shared_ptr<GCommandList> otherDeviceCmdList) const
	{
		auto dublciate = std::make_shared<GModel>(model, otherDeviceCmdList);
		dublciate->scaleMatrix = scaleMatrix;
		return dublciate;
	}

	void GModel::Serialize(json& j)
	{
		j["MaterialCount"] = materials.size();
		auto jMaterials = json::array();
		for(auto& material : materials){
			json jMaterial;
			jMaterial["id"] = material->GetID();
			jMaterials.push_back(jMaterial);
		}
		j["Materials"] = jMaterials;
	}

	void GModel::Deserialize(json& j)
	{
		uint32_t materialCount;
		(Asset::TryReadVariable<uint32_t>(j, "MaterialCount", &materialCount));

		materials.resize(materialCount);
		
		auto jMaterials = j["Materials"];

		for(uint32_t i = 0u; i < materialCount; ++i){
			uint64_t materialId;
			(Asset::TryReadVariable<uint64_t>(jMaterials[i], "id", &materialId));

			auto aMaterial = std::static_pointer_cast<AMaterial>(AssetDatabase::FindAssetByID(materialId));

			if (aMaterial) {
				materials[i] = (aMaterial);
			}
			else{

			}
		}
	}

}
