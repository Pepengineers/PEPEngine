#pragma once
#include "Component.h"
#include "d3dApp.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace DirectX;
		using namespace SimpleMath;

		class Rotater :
			public Component
		{
		public:
			Rotater(float speed = 1) : speed(speed)
			{
			}


			SERIALIZE_FROM_JSON(Rotater)
			
		private:
			void Update() override;;


			const float time = 2;
			float currentTime = 0;
			bool invers = false;


			float speed;


			void Serialize(json& j) override
			{
				j["Type"] = ComponentID;

				auto jPos = json(); 
				jPos["speed"] = speed;

				j["Variables"] = jPos;
			};

			void Deserialize(json& j) override
			{
				auto jPos = j["Variables"];
				assert(TryReadVariable<float>(jPos, "speed", &speed));
				
			};
		};
	}
}
