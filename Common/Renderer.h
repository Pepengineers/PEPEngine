#pragma once
#include "Component.h"
#include "Material.h"

namespace PEPEngine
{
	namespace Common
	{
		class Renderer : public Component
		{
		public:

			virtual std::vector<std::shared_ptr<Material>>& GetSharedMaterials() = 0;


			Renderer() : Component()
			{
			};

			void Update() override = 0;
			void PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList) override = 0;
		};
	}
}
