#pragma once
#include "d3d12.h"
#include <SimpleMath.h>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "DirectXBuffers.h"
#include "MemoryAllocator.h"
#include "GCommandList.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;
		using namespace Graphics;

		class NativeModel;
		class GMesh;
		class AMaterial;

		class GModel
		{
			std::shared_ptr<NativeModel> model;

			std::vector<std::shared_ptr<GMesh>> gmeshes{};

			std::shared_ptr<GDevice> device;

			std::vector<std::shared_ptr<AMaterial>> materials;
			
		public:

			void Serialize(json& j);
			void Deserialize(json& j);

			void SetMaterial(std::shared_ptr<AMaterial> material, UINT slot = 0);

			std::vector<std::shared_ptr<AMaterial>>& GetMaterials();

			std::shared_ptr<GDevice> GetDevice() const;;
			
			DirectX::SimpleMath::Matrix scaleMatrix = DirectX::SimpleMath::Matrix::CreateScale(1);

			UINT GetMeshesCount() const;

			std::shared_ptr<GMesh> GetMesh(UINT index);

			std::wstring GetName() const;

			GModel(std::shared_ptr<NativeModel> model, std::shared_ptr<GCommandList> uploadCmdList);

			GModel(const GModel& copy);

			~GModel();

			void Render(std::shared_ptr<GCommandList> cmdList, UINT meshIndex = 0);

			std::shared_ptr<GModel> Dublicate(std::shared_ptr<GCommandList> otherDeviceCmdList) const;
		};
	}
}
