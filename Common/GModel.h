#pragma once
#include "d3d12.h"
#include <SimpleMath.h>
#include "MemoryAllocator.h"
#include "GCommandList.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;
		using namespace Graphics;

		class NativeModel;
		class Material;
		class GMesh;

		class GModel
		{
			std::shared_ptr<NativeModel> model;

			std::vector<std::shared_ptr<GMesh>> gmeshes{};			

		public:

			custom_vector<std::shared_ptr<Material>> meshesMaterials = MemoryAllocator::CreateVector<std::shared_ptr<
				Material>>();
			
			DirectX::SimpleMath::Matrix scaleMatrix = DirectX::SimpleMath::Matrix::CreateScale(1);

			UINT GetMeshesCount() const;

			std::shared_ptr<Material> GetMeshMaterial(UINT index);

			std::shared_ptr<GMesh> GetMesh(UINT index);

			std::wstring GetName() const;;

			GModel(std::shared_ptr<NativeModel> model, std::shared_ptr<GCommandList> uploadCmdList);

			void SetMeshMaterial(UINT index, std::shared_ptr<Material> material);

			GModel(const GModel& copy);;

			~GModel();

			void Draw(std::shared_ptr<GCommandList> cmdList);

			std::shared_ptr<GModel> Dublicate(std::shared_ptr<GCommandList> otherDeviceCmdList) const;
		};
	}
}