#pragma once
#include "Component.h"
#include "GMesh.h"
#include "Material.h"

namespace PEPEngine
{
	namespace Common
	{
		class Renderer : public Component
		{

		protected:

			Renderer(json& json):Component(json)
			{
			}
			
		public:

			virtual std::vector<std::shared_ptr<Material>>& GetSharedMaterials() = 0;

			UINT virtual GetMeshCount() = 0;

			std::shared_ptr<GMesh> virtual GetMesh(UINT index) = 0;

			Renderer() : Component()
			{
			};

			void Update() override = 0;
			void virtual PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList, UINT meshIndex = 0) = 0;
		};
	}
}
