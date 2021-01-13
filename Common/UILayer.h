#pragma once
#include "RenderPass.h"
#include "GDescriptor.h"
#include "GCommandList.h"

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
		float killcount = 0;
		float timer = 0;
		UILayer(float width, float height, HWND hwnd);

		~UILayer();

		void RenderMainWindowAsDockPanel();
		void Render(std::shared_ptr<GCommandList> cmdList) override;;

		void Update() override;;

		void ChangeRenderTargetSize(float newWidth, float newHeight) override;;

		void setCount(float count, float time);

		static void CreateDeviceObject();;

		static void Invalidate();;

		LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};
}
