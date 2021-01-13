#pragma once
#include "Renderer.h"
#include "GCommandList.h"
#include "DirectXBuffers.h"
#include "ShaderBuffersData.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;
		using namespace Utils;
		using namespace Graphics;

		class Transform;
		class AModel;
		class AMaterial;

		class ModelRenderer : public Renderer
		{
		protected:
			ObjectConstants modelWorldData{};
			std::shared_ptr<ConstantUploadBuffer<ObjectConstants>> modelDataBuffer = nullptr;
			std::shared_ptr<AModel> model;

			void PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList, UINT meshIndex = 0) override;

			void Update() override;

			std::vector<std::shared_ptr<AMaterial>> materials;
			
			void Serialize(json& j) override;

			void Deserialize(json& j) override;		

		public:

			SERIALIZE_FROM_JSON(ModelRenderer, Renderer)
			
			void SetMaterial(std::shared_ptr<AMaterial> material, UINT slot);
			ModelRenderer(std::shared_ptr<AModel> model);
			void SetModel(std::shared_ptr<AModel> asset);
						
			UINT GetMeshCount() override;
			std::shared_ptr<GMesh> GetMesh(UINT index) override;
			std::vector<std::shared_ptr<AMaterial>>& GetSharedMaterials() override;
			std::shared_ptr<AModel> GetModel() override;
		};
	}
}
