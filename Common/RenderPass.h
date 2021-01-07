#pragma once
#include "GDeviceFactory.h"
#include "Renderer.h"
#include "MemoryAllocator.h"


namespace PEPEngine::Common
{
	using namespace Allocator;
	using namespace Graphics;

	class RenderPass
	{
	protected:
		float width;
		float height;
		std::shared_ptr<GDevice> device;
	public:

		virtual ~RenderPass()
		{
		};

		RenderPass(const float width, const float height): width(width), height(height),
		                                                   device(GDeviceFactory::GetDevice())
		{
		}


		void virtual Render(std::shared_ptr<GCommandList> cmdList) = 0;

		void virtual Update() = 0;

		void virtual ChangeRenderTargetSize(float newWidth, float newHeight) = 0;
	};
}
