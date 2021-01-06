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
				HeightMap = NormalMap + 1,
				MetallicMap = HeightMap + 1,
				RoughnessMap = MetallicMap + 1,
				AOMap = RoughnessMap + 1
			};

		private:

			static UINT materialIndexGlobal;

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

			Vector4 AmbientColor;
			Vector4 EmissiveColor;
			Vector4 DiffuseColor;
			Vector4 SpecularColor;
			Vector4 Reflectance;
			float Opacity;
			float SpecularPower;
			float IndexOfRefraction;
			float BumpIntensity;
			float SpecularScale;
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

			void InitMaterial(std::shared_ptr<GDevice> device);

			void Draw(std::shared_ptr<GCommandList> cmdList) const;


			void Update();

			std::wstring& GetName();
		};
	}
}
