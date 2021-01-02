#pragma once
#include "Renderer.h"
#include "MemoryAllocator.h"


namespace PEPEngine::Common
{
	using namespace Allocator;
	using namespace Graphics;

	class RenderPass
	{
	protected:
		custom_vector<Renderer*> renderers = MemoryAllocator::CreateVector<Renderer*>();

	public:

		virtual ~RenderPass()
		{
			renderers.clear();
		};

		void AddTargets(Renderer** targets, UINT size)
		{
			for (int i = 0; i < size; ++i)
			{
				this->renderers.push_back(targets[i]);
			}
		}

		void AddTarget(Renderer* target)
		{
			renderers.push_back(target);
		}

		void virtual Render(std::shared_ptr<GCommandList> cmdList) = 0;

		void virtual OnResize() = 0;
	};
}
