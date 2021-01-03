#pragma once
#include "Renderer.h"
#include "MemoryAllocator.h"


namespace PEPEngine::Common
{
	using namespace Allocator;
	using namespace Graphics;

	class RenderPass
	{
	

	public:

		virtual ~RenderPass()
		{
		};		

		void virtual Render(std::shared_ptr<GCommandList> cmdList) = 0;

		void virtual Update() = 0;
		
		void virtual OnResize() = 0;
	};
}
