#pragma once
#include "Component.h"
#include "Material.h"
namespace DX
{
	namespace Common
	{
		class Renderer : public Component
		{
		public:

			Renderer() : Component() {};

			void Update() override = 0;
			virtual void Draw(std::shared_ptr<GCommandList> cmdList) = 0;
		};
	}
}