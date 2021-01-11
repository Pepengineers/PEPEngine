#include "pch.h"
#include "Material.h"
#include "d3dApp.h"
#include "GDescriptor.h"
#include "GraphicPSO.h"
#include "ATexture.h"
#include "AssetDatabase.h"

namespace PEPEngine::Common
{
	UINT Material::GetMaterialIndex() const
	{
		return materialIndex;
	}

	void Material::SetMaterialIndex(UINT index)
	{
		materialIndex = index;
	}

	MaterialData& Material::GetMaterialConstantData()
	{
		return matConstants;
	}

	void Material::SetDirty()
	{
		NumFramesDirty = globalCountFrameResources;
	}

	void Material::SetRenderMode(RenderMode::Mode pso)
	{
		this->renderMode = pso;
	}

	RenderMode::Mode Material::GetRenderMode() const
	{
		return renderMode;
	}

	void Material::SetMaterialMap(MaterialSlotTypes type, std::shared_ptr<ATexture> texture)
	{
		const auto it = slots.find(type);

		if (it == slots.end())
		{
			materialMaps.push_back(texture);
			slots[type] = materialMaps.size() - 1;
		}
		else
		{
			materialMaps[it->second] = texture;
		}

		UpdateDescriptors();

		SetDirty();
	}

	Material::Material(std::string name, RenderMode::Mode pso) : Name(std::move(name)), renderMode(pso), AlphaThreshold(0.1)
	{
	}


	void Material::UpdateDescriptors()
	{
		if (!IsInited) return;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		for (auto&& slotPair : slots)
		{
			auto map = materialMaps[slotPair.second]->GetGTexture();
			auto desc = map->GetD3D12ResourceDesc();

			if (slotPair.first == BaseColor)
			{
				srvDesc.Format = GetSRGBFormat(desc.Format);
			}
			else
			{
				srvDesc.Format = (desc.Format);
			}

			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;

			map->CreateShaderResourceView(&srvDesc, &textureMapsSRVMemory, slotPair.second);
		}
	}

	void Material::Init(std::shared_ptr<GDevice> device)
	{
		if (IsInited) return;

		if (textureMapsSRVMemory.IsNull())
		{
			textureMapsSRVMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,MaxMaterialTexturesMaps);
		}

		IsInited = true;

		UpdateDescriptors();
	}

	void Material::SetRenderMaterialData(std::shared_ptr<GCommandList> cmdList) const
	{
		cmdList->SetDescriptorsHeap(&textureMapsSRVMemory);
		cmdList->SetRootDescriptorTable(MaterialTextures, &textureMapsSRVMemory);
	}

	void Material::Update()
	{
		if (NumFramesDirty > 0)
		{
			matConstants.DiffuseColor = DiffuseColor;
			matConstants.AlphaThreshold = AlphaThreshold;
			matConstants.SpecularPower = SpecularPower;

			for (auto& [type, Index] : slots)
			{
				switch (type)
				{
				case BaseColor: matConstants.DiffuseMapIndex = Index;
					break;
				case NormalMap: matConstants.NormalMapIndex = Index;
					break;	
				default: assert("WTF? Is it Material?");
				}
			}

			NumFramesDirty--;
		}
	}

	std::string& Material::GetName()
	{
		return (Name);
	}

	void Material::Serialize(json& j)
	{
		j["Name"] = Name;
		j["Mode"] = renderMode;
		auto jDiffuseColor = json();
		jDiffuseColor["x"] = DiffuseColor.x;
		jDiffuseColor["y"] = DiffuseColor.y;
		jDiffuseColor["z"] = DiffuseColor.z;
		jDiffuseColor["w"] = DiffuseColor.w;

		j["DiffuseColor"] = jDiffuseColor;

		j["AlphaThreshold"] = AlphaThreshold;

		j["SpecularPower"] = SpecularPower;

		j["TexturesCount"] = materialMaps.size();

		auto jTextureArray = json::array();

		for(auto& materialMap : materialMaps){
			json jTexture;
			jTexture["id"] = materialMap->GetID();
			jTextureArray.push_back(jTexture);
		}

		j["Textures"] = jTextureArray;

		auto jTextureSlots = json::array();

		for(auto& slot : slots){
			json jSlot;
			jSlot["Index"] = slot.second;
			jSlot["SlotType"] = slot.first;
			jTextureSlots.push_back(jSlot);
		}

		j["MaterialMapSlots"] = jTextureSlots;
	}

	void Material::Deserialize(json& j)
	{		
		(Asset::TryReadVariable<RenderMode::Mode>(j, "Mode", &renderMode));
		(Asset::TryReadVariable<std::string>(j, "Name", &Name));
		
		float x, y, z, w;
		auto jcolor = j["DiffuseColor"];
		(Asset::TryReadVariable<float>(jcolor, "x", &x));
		(Asset::TryReadVariable<float>(jcolor, "y", &y));
		(Asset::TryReadVariable<float>(jcolor, "z", &z));
		(Asset::TryReadVariable<float>(jcolor, "w", &w));
		DiffuseColor = Vector4{ x, y, z, w };
		(Asset::TryReadVariable<float>(j, "AlphaThreshold", &AlphaThreshold));
		(Asset::TryReadVariable<float>(j, "SpecularPower", &SpecularPower));
		UINT count = 0u;
		(Asset::TryReadVariable<UINT>(j, "TexturesCount", &count));


		auto jTextureSlots = j["MaterialMapSlots"];

		std::unordered_map<UINT, MaterialSlotTypes> tempSlots;

		for(auto& jSlot : jTextureSlots){
			MaterialSlotTypes slotType;
			UINT index;
			(Asset::TryReadVariable<MaterialSlotTypes>(jSlot, "SlotType", &slotType));
			(Asset::TryReadVariable<UINT>(jSlot, "Index", &index));

			tempSlots[index] = slotType;
		}


		auto jTextures = j["Textures"];

		for(uint32_t i = 0u; i < count; ++i){
			uint64_t textureId;
			(Asset::TryReadVariable<uint64_t>(jTextures[i], "id", &textureId));
			auto textureAsset = std::static_pointer_cast<ATexture>(AssetDatabase::FindAssetByID(textureId));
			if(textureAsset){
				materialMaps.push_back(textureAsset);
			}
			else{
				tempSlots.erase(i);
			}
		}

		for(auto& slot : tempSlots){
			slots[slot.second] = slot.first;
		}

		tempSlots.clear();

	}

}
