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

			void Update() override;;
		public:
			ObjectMover();;
		};
	}
}
