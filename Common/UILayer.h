#pragma once
#include "RenderPass.h"

namespace PEPEngine::Common
{
	
	class UILayer :
		public RenderPass
	{
		GDescriptor srvMemory;
		HWND hwnd;

		void SetupRenderBackend();;

		void Initialize();;

		
	public:
		UILayer(float width, float height, const HWND hwnd);

		~UILayer();

		void RenderMainWindowAsDockPanel();
		void Render(std::shared_ptr<GCommandList> cmdList) override;;

		void Update() override;;

		void ChangeRenderTargetSize(float newWidth, float newHeight) override;;

		static void CreateDeviceObject();;

		static void Invalidate();;

		LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};
}
