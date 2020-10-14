#include "pch.h"
#include "Material.h"
#include "d3dApp.h"
#include "GMemory.h"
#include "GraphicPSO.h"

namespace DX
{
	namespace Common
	{
		UINT Material::materialIndexGlobal = 0;

		UINT Material::GetMaterialIndex()
		{
			return materialIndex;
		}

		void Material::SetMaterialIndex(UINT index)
		{
			materialIndex = index;
		}

		std::shared_ptr<GTexture> Material::GetDiffuseTexture() const
		{
			return diffuseMap;
		}

		std::shared_ptr<GTexture> Material::GetNormalTexture() const
		{
			return normalMap;
		}

		UINT Material::GetDiffuseMapIndex() const
		{
			return DiffuseMapIndex;
		}

		UINT Material::GetNormalMapDiffuseIndex() const
		{
			return NormalMapIndex;
		}

		MaterialData& Material::GetMaterialConstantData()
		{
			return matConstants;
		}

		UINT Material::GetIndex() const
		{
			return materialIndex;
		}

		void Material::SetDirty()
		{
			NumFramesDirty = globalCountFrameResources;
		}

		void Material::SetNormalMap(std::shared_ptr<GTexture> texture, UINT index)
		{
			normalMap = texture;
			NormalMapIndex = index;
		}

		void Material::SetType(PsoType::Type pso)
		{
			this->type = pso;
		}

		PsoType::Type Material::GetPSO() const
		{
			return type;
		}

		void Material::SetDiffuseTexture(std::shared_ptr<GTexture> texture, UINT index)
		{
			diffuseMap = texture;
			DiffuseMapIndex = index;
		}

		Material::Material(std::wstring name, PsoType::Type pso) : Name(std::move(name)), type(pso)
		{
			materialIndex = materialIndexGlobal++;
		}


		void Material::InitMaterial(GMemory* textureHeap)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			//TODO: Подумать как можно от этого избавиться, и работать всегда только с индексами
			if (diffuseMap)
			{
				auto desc = diffuseMap->GetD3D12Resource()->GetDesc();

				if (diffuseMap)
				{
					srvDesc.Format = GetSRGBFormat(desc.Format);
				}
				else
				{
					srvDesc.Format = (desc.Format);
				}


				switch (type)
				{
				case PsoType::AlphaSprites:
					srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
					srvDesc.Texture2DArray.MostDetailedMip = 0;
					srvDesc.Texture2DArray.MipLevels = -1;
					srvDesc.Texture2DArray.FirstArraySlice = 0;
					srvDesc.Texture2DArray.ArraySize = desc.DepthOrArraySize;
					break;
				default:
					{
						srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
						srvDesc.Texture2D.MostDetailedMip = 0;
						srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
						srvDesc.Texture2D.MipLevels = desc.MipLevels;
					}
				}
				diffuseMap->CreateShaderResourceView(&srvDesc, textureHeap, DiffuseMapIndex);
			}

			if (normalMap)
			{
				srvDesc.Format = normalMap->GetD3D12Resource()->GetDesc().Format;
				srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
				srvDesc.Texture2D.MipLevels = normalMap->GetD3D12Resource()->GetDesc().MipLevels;
				normalMap->CreateShaderResourceView(&srvDesc, textureHeap, NormalMapIndex);
			}
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
				matConstants.HasAmbientTexture = false;
				matConstants.HasEmissiveTexture = false;
				matConstants.HasDiffuseTexture  = diffuseMap != nullptr;
				matConstants.HasSpecularTexture = false;
				matConstants.HasSpecularPowerTexture = false;
				matConstants.HasNormalTexture = normalMap != nullptr;
				matConstants.HasBumpTexture = false;
				matConstants.HasOpacityTexture = false;
				matConstants.BumpIntensity = BumpIntensity;
				matConstants.SpecularScale = SpecularScale;
				matConstants.AlphaThreshold = AlphaThreshold;
				NumFramesDirty--;
			}
		}

		std::wstring& Material::GetName()
		{
			return Name;
		}
	}
}
