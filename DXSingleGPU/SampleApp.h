#pragma once

#include "AssetsLoader.h"
#include "Camera.h"
#include "d3dApp.h"
#include "FrameResource.h"
#include "GameTimer.h"
#include "GMemory.h"
#include "GModel.h"
#include "GraphicPSO.h"
#include "GRootSignature.h"
#include "GShader.h"
#include "KeyboardDevice.h"
#include "Light.h"
#include "Mousepad.h"
#include "ShadowMap.h"
#include "SSAA.h"
#include "Ssao.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace PackedVector;

class ModelRenderer;

namespace DXLib
{
	class SampleApp : public D3DApp
	{
	public:
		SampleApp(HINSTANCE hInstance);
		SampleApp(const SampleApp& rhs) = delete;
		SampleApp& operator=(const SampleApp& rhs) = delete;
		~SampleApp();	

		void GeneratedMipMap();
		void BuildSsaoRootSignature();
		
		bool Initialize() override;

		
		UINT pathMapShow = 0;
		//off, shadowMap, ssaoMap
		const UINT maxPathMap = 3;

	private:

		LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
		
		void OnResize() override;
		void UpdateShadowTransform(const GameTimer& gt);
		void UpdateShadowPassCB(const GameTimer& gt);
		void UpdateSsaoCB(const GameTimer& gt);
		void LoadStudyTexture(std::shared_ptr<GCommandList> cmdList);
		void SetDefaultMaterialForModel(ModelRenderer* renderer);
		void LoadModels();
		void BuildTexturesHeap();
		void Update(const GameTimer& gt) override;
		void DrawSSAAMap(std::shared_ptr<GCommandList> cmdList);
		void DrawToWindowBackBuffer(std::shared_ptr<GCommandList> cmdList);
		void UpdateMaterial(const GameTimer& gt);
		void DrawSceneToShadowMap(std::shared_ptr<GCommandList> cmdList);
		void DrawNormals(std::shared_ptr<GCommandList> cmdList);
		void Draw(const GameTimer& gt) override;

		void UpdateGameObjects(const GameTimer& gt);
		void UpdateMainPassCB(const GameTimer& gt);


		void LoadTextures( std::shared_ptr<GCommandList> cmdList);
		void BuildRootSignature();
		void BuildShadersAndInputLayout();
		void BuildShapeGeometry();
		void BuildPSOs();
		void BuildFrameResources();
		void BuildMaterials();
		void BuildGameObjects();
		std::unique_ptr<GameObject> CreateGOWithRenderer(std::shared_ptr<GModel> model);
		static void DrawGameObjects(std::shared_ptr<GCommandList> cmdList, const custom_vector<GameObject*>& ritems);
		static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();
		void SortGO();
		
	private:

		UINT backBufferIndex = 0;
		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		D3D12_VIEWPORT viewport;
		D3D12_RECT rect;

		GMemory renderTargetMemory;
		
		custom_unordered_map<std::wstring, std::shared_ptr<GModel>> models = MemoryAllocator::CreateUnorderedMap<std::wstring, std::shared_ptr<GModel>>();
			
		custom_vector<std::unique_ptr<FrameResource>> frameResources = MemoryAllocator::CreateVector<std::unique_ptr<FrameResource>>();
		
		FrameResource* currentFrameResource = nullptr;
		int currentFrameResourceIndex = 0;

		
		std::unique_ptr<GRootSignature> rootSignature = nullptr;
		std::unique_ptr<GRootSignature> ssaoRootSignature = nullptr;
		std::unique_ptr<ShadowMap> shadowMap;
		std::unique_ptr<Ssao> ssao;
		std::unique_ptr<SSAA> ssaa;
		
		GMemory srvHeap;
		AssetsLoader loader;
				
		
		custom_unordered_map<std::string, std::unique_ptr<GShader>> shaders = MemoryAllocator::CreateUnorderedMap<std::string, std::unique_ptr<GShader>>();
				
		custom_unordered_map<PsoType::Type, std::unique_ptr<GraphicPSO>> psos = MemoryAllocator::CreateUnorderedMap<PsoType::Type, std::unique_ptr<GraphicPSO>>();
		
		custom_vector<Light*> lights = MemoryAllocator::CreateVector<Light*>();		

		custom_vector<D3D12_INPUT_ELEMENT_DESC> defaultInputLayout = MemoryAllocator::CreateVector<D3D12_INPUT_ELEMENT_DESC>();
		custom_vector<D3D12_INPUT_ELEMENT_DESC> treeSpriteInputLayout = MemoryAllocator::CreateVector<D3D12_INPUT_ELEMENT_DESC>();

		custom_vector<std::unique_ptr<GameObject>> gameObjects = MemoryAllocator::CreateVector<std::unique_ptr<GameObject>>();
		
		custom_vector<custom_vector<GameObject*>> typedGameObjects = MemoryAllocator::CreateVector<custom_vector<GameObject*>>();

		PassConstants mainPassCB;
		PassConstants mShadowPassCB;

		float mLightNearZ = 0.0f;
		float mLightFarZ = 0.0f;
		Vector3 mLightPosW;
		Matrix mLightView = Matrix::Identity;
		Matrix mLightProj = Matrix::Identity;
		Matrix mShadowTransform = Matrix::Identity;

		float mLightRotationAngle = 0.0f;
		Vector3 mBaseLightDirections[3] = {
			Vector3(0.57735f, -0.57735f, 0.57735f),
			Vector3(-0.57735f, -0.57735f, 0.57735f),
			Vector3(0.0f, -0.707f, -0.707f)
		};
		Vector3 mRotatedLightDirections[3];

		

		BoundingSphere mSceneBounds;
	};

	
}
