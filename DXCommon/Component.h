#pragma once
#include "MemoryAllocator.h"
#include "GCommandList.h"
#include <memory>
namespace DX
{
	namespace Common
	{
		using namespace DX::Allocator;
		using namespace DX::Graphics;
		
		class GameObject;

		class Component
		{
		public:

			GameObject* gameObject = nullptr;

			Component();

			virtual void Update() = 0;
			virtual void Draw(std::shared_ptr<GCommandList> cmdList) = 0;
		};
	}
}