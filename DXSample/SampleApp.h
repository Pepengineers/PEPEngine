#pragma once
#include "AssetsLoader.h"
#include "d3dApp.h"
#include "GModel.h"
#include "GTexture.h"
#include <string>
#include "FrameResource.h"
#include "GMemory.h"
#include "GraphicPSO.h"
#include "Light.h"

class SampleApp :
    public DXLib::D3DApp
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
	
	std::unordered_map<std::wstring,std::shared_ptr<GModel>> models;

	std::vector<std::shared_ptr<FrameResource>> frameResources;
	std::shared_ptr<FrameResource> currentFrameResource;

	std::vector<std::shared_ptr<GameObject>> gameObjects;
	std::vector<Light*> lights;
	std::unique_ptr<Camera> camera = nullptr;

	PassConstants mainPassCB;
	
	D3D12_VIEWPORT viewport;
	D3D12_RECT rect;
	
	GTexture depthBuffer;
	

	GMemory dsvMemory;
	GMemory rtvMemory;
	GMemory srvMemory;
	std::shared_ptr<GRootSignature> rootSignature;
	std::unordered_map<std::wstring, std::shared_ptr<GShader>> shaders;
	std::vector<D3D12_INPUT_ELEMENT_DESC> defaultInputLayout;
	std::shared_ptr<GraphicPSO> opaque;
};

