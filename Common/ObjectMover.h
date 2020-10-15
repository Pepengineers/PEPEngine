#pragma once
#include "Component.h"

namespace PEPEngine
{
	namespace Common
	{
		class KeyboardDevice;

		class ObjectMover :
			public Component
		{
			KeyboardDevice* keyboard;

			void PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList) override;;

			void Update() override;;
		public:
			ObjectMover();;
		};
	}
}
