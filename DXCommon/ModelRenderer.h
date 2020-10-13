#pragma once
#include "Renderer.h"
#include "GCommandList.h"
namespace DX
{
	namespace Common
	{
		using namespace DX::Allocator;
		using namespace DX::Utils;
		using namespace DX::Graphics;
		
		class Transform;
		class GModel;

		class ModelRenderer : public Renderer
		{
		protected:

			ObjectConstants objectWorldData{};
			std::shared_ptr<ConstantBuffer<ObjectConstants>> modelDataBuffer = nullptr;
			std::shared_ptr<GDevice> device;
			std::shared_ptr<GModel> model;


			void Draw(std::shared_ptr<GCommandList> cmdList) override;

			void Update() override;

		public:

			ModelRenderer(const std::shared_ptr<GDevice> device, std::shared_ptr<GModel> model);

			void SetModel(std::shared_ptr<GModel> asset);
		};
	}
}