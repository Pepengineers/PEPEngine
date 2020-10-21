#pragma once
#include "AssetsLoader.h"
#include "d3dApp.h"
#include "FrameResource.h"
#include "Light.h"
#include "GDescriptor.h"
#include "GraphicPSO.h"

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
		std::vector<std::shared_ptr<FrameResource>> frameResources;
		std::shared_ptr<FrameResource> currentFrameResource;

		std::vector<std::shared_ptr<GameObject>> gameObjects;
		std::vector<GameObject*> typedGO[RenderMode::Count];


		std::vector<Light*> lights;
		std::unique_ptr<Camera> camera = nullptr;

		PassConstants mainPassCB;

		D3D12_VIEWPORT viewport;
		D3D12_RECT rect;

		GTexture depthBuffer;


		GDescriptor dsvMemory;
		GDescriptor rtvMemory;

		GDescriptor defferedRTVMemory;
		GDescriptor defferedSRVMemory;

		std::vector<GTexture> defferedGBufferTextures;
		std::shared_ptr<GRootSignature> rootSignature;
		std::unordered_map<std::wstring, std::shared_ptr<GShader>> shaders;
		std::vector<D3D12_INPUT_ELEMENT_DESC> defaultInputLayout;
		std::shared_ptr<GraphicPSO> deferredGBufferPSO;
		std::shared_ptr<GraphicPSO> quadPso;
		std::shared_ptr<GraphicPSO> debugPso;
	};
}