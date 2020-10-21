#include "pch.h"
#include "Material.h"
#include "d3dApp.h"
#include "GDescriptor.h"
#include "GraphicPSO.h"

namespace PEPEngine
{
	namespace Common
	{
		UINT Material::materialIndexGlobal = 0;

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
			this->type = pso;
		}

		RenderMode::Mode Material::GetRenderMode() const
		{
			return type;
		}

		void Material::SetMaterialMap(MaterialTypes type, std::shared_ptr<GTexture> texture)
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
		}	

		Material::Material(std::wstring name, RenderMode::Mode pso) : Name(std::move(name)), type(pso)
		{
			materialIndex = materialIndexGlobal++;
		}


		void Material::InitMaterial(std::shared_ptr<GDevice> device)
		{
			if(textureMapsSRVMemory.IsNull())
			{
				textureMapsSRVMemory = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, MaxMaterialTexturesMaps);
			}

			
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			for (auto && slotPair : slots)
			{
				auto map = materialMaps[slotPair.second];
				auto desc = map->GetD3D12ResourceDesc();
				
				if(slotPair.first == BaseColor)
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

		void Material::Draw(std::shared_ptr<GCommandList> cmdList) const
		{
			cmdList->SetDescriptorsHeap(&textureMapsSRVMemory);
			cmdList->SetRootDescriptorTable(DefferedPassRSSlots::MaterialTextures, &textureMapsSRVMemory);
		}

		void Material::Update()
		{
			if (NumFramesDirty > 0)
			{
				matConstants.GlobalAmbient = GlobalAmbient;
				matConstants.AmbientColor = AmbientColor;
				matConstants.EmissiveColor = EmissiveColor;
				matConstants.DiffuseColor = DiffuseColor;
				matConstants.SpecularColor = SpecularColor;
				matConstants.Reflectance = Reflectance;
				matConstants.Opacity = Opacity;
				matConstants.SpecularPower = SpecularPower;
				matConstants.IndexOfRefraction = IndexOfRefraction;
				matConstants.BumpIntensity = BumpIntensity;
				matConstants.SpecularScale = SpecularScale;
				matConstants.AlphaThreshold = AlphaThreshold;
				
				for (auto& [type, Index] : slots)
				{
					switch (type)
					{
					case BaseColor: matConstants.DiffuseMapIndex = Index;
						break;
					case NormalMap: matConstants.NormalMapIndex = Index;
						break;
					case HeightMap: matConstants.HeightMapIndex = Index;
						break;
					case MetallicMap: matConstants.MetallicMapIndex = Index;
						break;
					case RoughnessMap: matConstants.RounghessMapIndex = Index;
						break;
					case AOMap: matConstants.AOMapIndex = Index;
						break;
					default: assert("WTF? Is it Material?");
					}
				}	

				NumFramesDirty--;
			}
		}

		std::wstring& Material::GetName()
		{
			return Name;
		}
	}
}
