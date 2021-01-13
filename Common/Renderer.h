#pragma once
#include "Component.h"
#include "GCommandList.h"

namespace PEPEngine
{
	using namespace Graphics;
	
	namespace Common
	{
		class AMaterial;
		class GMesh;
		class AModel;
		
		class Renderer : public Component
		{
		protected:

			Renderer(json& json):Component(json)
			{
			}
			
		public:

			virtual std::vector<std::shared_ptr<AMaterial>>& GetSharedMaterials() = 0;

			uint32_t virtual GetMeshCount() = 0;

			std::shared_ptr<GMesh> virtual GetMesh(uint32_t index) = 0;

			std::shared_ptr<AModel> virtual GetModel() = 0;

			Renderer() : Component()
			{
			};

			void Update() override = 0;
			void virtual PopulateDrawCommand(std::shared_ptr<Graphics::GCommandList> cmdList, uint32_t meshIndex = 0) = 0;
		};
	}
}
