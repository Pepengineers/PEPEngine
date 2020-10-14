#pragma once

#include <DirectXColors.h>
#include "d3dUtil.h"
#include "d3dx12.h"
#include "GraphicPSO.h"
#include "ShaderBuffersData.h"
#include "GTexture.h"

namespace DX
{
	namespace Common
	{
		using namespace Microsoft::WRL;
		using namespace Allocator;
		using namespace Utils;
		using namespace Graphics;

		class Material
		{
			static UINT materialIndexGlobal;

			UINT materialIndex = -1;

			std::wstring Name;

			PsoType::Type type = PsoType::Opaque;

			MaterialData matConstants{};

			UINT NumFramesDirty = globalCountFrameResources;

			std::shared_ptr<GTexture> diffuseMap = nullptr;
			std::shared_ptr<GTexture> normalMap = nullptr;


			UINT DiffuseMapIndex = -1;
			UINT NormalMapIndex = -1;

			CD3DX12_GPU_DESCRIPTOR_HANDLE gpuTextureHandle;
			CD3DX12_CPU_DESCRIPTOR_HANDLE cpuTextureHandle;

		public:

			Vector4  GlobalAmbient;
			Vector4  AmbientColor;
			Vector4  EmissiveColor;
			Vector4  DiffuseColor;
			Vector4  SpecularColor;
			Vector4  Reflectance;
			float   Opacity;
			float   SpecularPower;
			float   IndexOfRefraction;
			bool    HasAmbientTexture;
			bool    HasEmissiveTexture;
			bool    HasDiffuseTexture;
			bool    HasSpecularTexture;
			bool    HasSpecularPowerTexture;
			bool    HasNormalTexture;
			bool    HasBumpTexture;
			bool    HasOpacityTexture;
			float   BumpIntensity;
			float   SpecularScale;
			float   AlphaThreshold;
			
			UINT GetMaterialIndex();

			void SetMaterialIndex(UINT index);

			std::shared_ptr<GTexture> GetDiffuseTexture() const;

			std::shared_ptr<GTexture> GetNormalTexture() const;

			UINT GetDiffuseMapIndex() const;

			UINT GetNormalMapDiffuseIndex() const;

			MaterialData& GetMaterialConstantData();

			UINT GetIndex() const;

			void SetDirty();

			PsoType::Type GetPSO() const;

			void SetNormalMap(std::shared_ptr<GTexture> texture, UINT index);

			void SetType(PsoType::Type pso);

			void SetDiffuseTexture(std::shared_ptr<GTexture> texture, UINT index);

			Material(std::wstring name, PsoType::Type pso = PsoType::Opaque);

			void InitMaterial(GMemory* textureHeap);

			void Update();

			std::wstring& GetName();
		};
	}
}
