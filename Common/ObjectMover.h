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

			

			void Serialize(json& j) override
			{
				j["Type"] = ComponentID;				
			};

			void Deserialize(json& j) override
			{
				
			};

		public:
			SERIALIZE_FROM_JSON(ObjectMover)
			
			ObjectMover();;
		};
	}
}
