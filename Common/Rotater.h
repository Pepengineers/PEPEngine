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

		private:
			void Update() override;;

			void PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList) override
			{
			};

			const float time = 2;
			float currentTime = 0;
			bool invers = false;


			float speed;
		};
	}
}