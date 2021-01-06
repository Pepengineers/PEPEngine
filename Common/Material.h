#pragma once

#include <DirectXColors.h>
#include "d3dUtil.h"
#include "d3dx12.h"
#include "GDescriptor.h"
#include "GraphicPSO.h"
#include "ShaderBuffersData.h"
#include "GTexture.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace Microsoft::WRL;
		using namespace Allocator;
		using namespace Utils;
		using namespace Graphics;


		class Material
		{
		public:
			enum MaterialTypes : UINT
			{
				DiffuseMap = 0,
				BaseColor = DiffuseMap,
				NormalMap = BaseColor + 1,
				RoughnessMap = NormalMap + 1
			};

		private:
		

			UINT materialIndex = -1;

			std::wstring Name;

			RenderMode::Mode type = RenderMode::Opaque;

			MaterialData matConstants{};

			UINT NumFramesDirty = globalCountFrameResources;

			std::vector<std::shared_ptr<GTexture>> materialMaps;

			std::unordered_map<MaterialTypes, UINT> slots;

			GDescriptor textureMapsSRVMemory;

			bool IsInited = false;

		public:

			static const UINT MaxMaterialTexturesMaps = 3;
			
			Vector4 DiffuseColor;
			Vector4 SpecularColor;			
			float AlphaThreshold;

			UINT GetMaterialIndex() const;

			void SetMaterialIndex(UINT index);

			MaterialData& GetMaterialConstantData();

			void SetDirty();

			RenderMode::Mode GetRenderMode() const;

			void SetMaterialMap(MaterialTypes type, std::shared_ptr<GTexture> texture);

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
