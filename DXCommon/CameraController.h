#pragma once
#include "Component.h"
#include "d3dApp.h"


namespace DX
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


			double xMouseSpeed = 100;
			double yMouseSpeed = 70;

		public:

			CameraController();

			void Update() override;;
			void Draw(std::shared_ptr<GCommandList> cmdList) override;;
		};
	}
}
