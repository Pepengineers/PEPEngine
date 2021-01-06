#pragma once

#include <DirectXColors.h>
#include "d3dUtil.h"
#include "d3dx12.h"
#include "GDescriptor.h"
#include "GraphicPSO.h"
#include "ShaderBuffersData.h"
#include "GCommandList.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace PEPEngine
{
	namespace Common
	{
		using namespace Microsoft::WRL;
		using namespace Utils;
		using namespace Graphics;

		class ATexture;
		class Material
		{
		public:
			Material() = default;
			enum MaterialSlotTypes : UINT
			{
				DiffuseMap = 0,
				BaseColor = DiffuseMap,
				NormalMap = BaseColor + 1,
				RoughnessMap = NormalMap + 1
			};

		public:
			void Serialize(json& j);
			void Deserialize(json& j);
		private:
		

			UINT materialIndex = -1;

			std::wstring Name;

			RenderMode::Mode renderMode = RenderMode::Opaque;

			MaterialData matConstants{};

			UINT NumFramesDirty = globalCountFrameResources;

			std::vector<std::shared_ptr<ATexture>> materialMaps;


			std::unordered_map<MaterialSlotTypes, UINT> slots;

			GDescriptor textureMapsSRVMemory;

			bool IsInited = false;

		public:

			static const UINT MaxMaterialTexturesMaps = 3;
			
			Vector4 DiffuseColor;		
			float AlphaThreshold;
			float SpecularPower = 1.0f;

			UINT GetMaterialIndex() const;

			void SetMaterialIndex(UINT index);

			MaterialData& GetMaterialConstantData();

			void SetDirty();

			RenderMode::Mode GetRenderMode() const;

			void SetMaterialMap(MaterialSlotTypes type, std::shared_ptr<ATexture> texture);

			void SetRenderMode(RenderMode::Mode pso);

			Material(std::wstring name, RenderMode::Mode pso = RenderMode::Opaque);
			void UpdateDescriptors();

			void Init(std::shared_ptr<GDevice> device);

			void SetRenderMaterialData(std::shared_ptr<GCommandList> cmdList) const;


			void Update();

			std::wstring& GetName();
		};
	}
}
