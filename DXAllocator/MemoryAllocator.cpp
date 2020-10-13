#include "pch.h"
#include "MemoryAllocator.h"

namespace DX::Allocator
{

		std::shared_ptr<LinearAllocationStrategy<>> MemoryAllocator::allocatorStrategy = std::make_shared<LinearAllocationStrategy<INIT_LINEAR_SIZE>>();
}
