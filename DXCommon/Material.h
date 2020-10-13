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

			MaterialConstants matConstants{};

			UINT NumFramesDirty = globalCountFrameResources;

			std::shared_ptr<GTexture> diffuseMap = nullptr;
			std::shared_ptr<GTexture> normalMap = nullptr;


			UINT DiffuseMapIndex = -1;
			UINT NormalMapIndex = -1;

			CD3DX12_GPU_DESCRIPTOR_HANDLE gpuTextureHandle;
			CD3DX12_CPU_DESCRIPTOR_HANDLE cpuTextureHandle;

		public:

			UINT GetMaterialIndex();

			void SetMaterialIndex(UINT index);

			std::shared_ptr<GTexture> GetDiffuseTexture() const;

			std::shared_ptr<GTexture> GetNormalTexture() const;

			UINT GetDiffuseMapIndex() const;

			UINT GetNormalMapDiffuseIndex() const;

			MaterialConstants& GetMaterialConstantData();

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

			Vector4 DiffuseAlbedo = DirectX::XMFLOAT4(DirectX::Colors::White);
			Vector3 FresnelR0 = {0.01f, 0.01f, 0.01f};
			float Roughness = .25f;
			Matrix MatTransform = Matrix::Identity;
		};
	}
}
