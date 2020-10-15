#pragma once
#include "MemoryAllocator.h"
#include "GCommandList.h"
#include <memory>

namespace PEPEngine
{
	namespace Common
	{
		using namespace Allocator;
		using namespace Graphics;

		class GameObject;

		class Component
		{
		public:

			GameObject* gameObject = nullptr;

			Component();

			virtual void Update() = 0;
			virtual void PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList) = 0;
		};
	}
}
