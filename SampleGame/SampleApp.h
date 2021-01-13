#pragma once
#include "AssetDatabase.h"
#include "d3dApp.h"
#include "GPass.h"
#include "LightPass.h"
#include "RenderPass.h"
#include "Scene.h"
#include "SSAOPass.h"
#include "UILayer.h"



namespace SimpleRender
{
	using namespace PEPEngine;
	using namespace Common;
	using namespace Utils;
	using namespace Graphics;
	using namespace Allocator;

	class SampleApp :
		public D3DApp
	{
	public:
		SampleApp(HINSTANCE hInstance);

	protected:
		void Pick(const MousePoint& mouse_point);
		void Update(const GameTimer& gt) override;
		void Draw(const GameTimer& gt) override;
	public:
		bool Initialize() override;
	protected:
		void OnResize() override;
		LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

	private:
		
	
		float killcount = 0;
		float timer = 0;
		std::shared_ptr<GDevice> device;
		AssetDatabase assetLoader;

		UINT currentFrameResourceIndex = 0;

		UINT64 renderFenceValues[globalCountFrameResources];
		UINT64 computeFenceValues[globalCountFrameResources];

		std::shared_ptr<AScene> level = nullptr;
		std::shared_ptr<AModel> pbody = nullptr;

		
		std::vector<Camera*> cameras;

		std::shared_ptr<UILayer> uiPass;
	};
}
