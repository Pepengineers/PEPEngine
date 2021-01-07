#pragma once
#include "Component.h"
#include "d3dApp.h"
#include <cassert>

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;
		using namespace Utils;

		class Mousepad;
		class KeyboardDevice;

		class CameraController :
			public Component
		{
			
			
			KeyboardDevice* keyboard;
			Mousepad* mouse;
			GameTimer* timer{};


			float xMouseSpeed = 100;
			float yMouseSpeed = 70;

			void Serialize(json& j) override
			{
				j["Type"] = ComponentID;

				auto jPos = json();  ;
				jPos["xMouseSpeed"] = xMouseSpeed;
				jPos["yMouseSpeed"] = yMouseSpeed;

				j["Variables"] = jPos;
			};

			void Deserialize(json& j) override
			{
				auto jPos = j["Variables"];
				assert(TryReadVariable<float>(jPos, "xMouseSpeed", &xMouseSpeed));
				assert(TryReadVariable<float>(jPos, "yMouseSpeed", &yMouseSpeed));
				
				Initialize();
			};
			
		public:
			SERIALIZE_FROM_JSON(CameraController)
			
			void Initialize();
			CameraController();

			void Update() override;;
		};
	}
}
