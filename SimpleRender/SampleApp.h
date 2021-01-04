#pragma once
#include "AssetsLoader.h"
#include "d3dApp.h"
#include "GPass.h"
#include "LightPass.h"
#include "RenderPass.h"
#include "Scene.h"
#include "SSAOPass.h"

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
		void Update(const GameTimer& gt) override;
		void Draw(const GameTimer& gt) override;
	public:
		bool Initialize() override;
	protected:
		void OnResize() override;
		LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

	private:

		std::shared_ptr<GDevice> device;
		AssetsLoader assetLoader;

		std::unordered_map<std::wstring, std::shared_ptr<GModel>> models;

		UINT currentFrameResourceIndex = 0;

		UINT64 fenceValues[globalCountFrameResources];

		std::shared_ptr<Scene> scene = nullptr;

		std::shared_ptr<GPass> gpass;
		std::shared_ptr < LightPass> lightPass;
		std::shared_ptr < SSAOPass> ambiantPass;
	};
}
