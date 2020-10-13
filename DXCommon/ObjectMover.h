#pragma once
#include "Component.h"
namespace DX
{
	namespace Common
	{
		class KeyboardDevice;

		class ObjectMover :
			public Component
		{
			KeyboardDevice* keyboard;

			void Draw(std::shared_ptr<GCommandList> cmdList) override;;

			void Update() override;;
		public:
			ObjectMover();;
		};
	}
}