#pragma once
#include "Renderer.h"
#include "GCommandList.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;
		using namespace Utils;
		using namespace Graphics;

		class Transform;
		class GModel;

		class ModelRenderer : public Renderer
		{
		protected:

			ObjectConstants objectWorldData{};
			std::shared_ptr<ConstantUploadBuffer<ObjectConstants>> modelDataBuffer = nullptr;
			std::shared_ptr<GDevice> device;
			std::shared_ptr<GModel> model;


			void PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList) override;

			void Update() override;

		public:

			ModelRenderer(std::shared_ptr<GDevice> device, std::shared_ptr<GModel> model);

			void SetModel(std::shared_ptr<GModel> asset);
		};
	}
}
