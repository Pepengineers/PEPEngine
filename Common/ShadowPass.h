#pragma once
#include "RenderPass.h"
#include "Scene.h"
#include "Light.h"
namespace PEPEngine::Common
{
	class ShadowPass :
		public RenderPass
	{

		D3D12_VIEWPORT viewPort;
		D3D12_RECT rect;

		DXGI_FORMAT mFormat = DXGI_FORMAT_R24G8_TYPELESS;

		GDescriptor srvMemory;

		GDescriptor dsvMemory;

		GTexture shadowMap;

		GraphicPSO pso;
		GRootSignature rootSignature;
		GShader vertexShader;
		GShader pixelShader;

		Matrix projectionMatrix;
		
		Light* mainDirectionalLight = nullptr;

		CameraConstants shadowMapConstants;

		std::shared_ptr<ConstantUploadBuffer<CameraConstants>> LightCameraBuffer;
		
	public:
		void InitRootSignature();

		void LoadAndCompileShaders();

		void InitPSO();

		ShadowPass(float width, float height);

		GDescriptor* GetSrvMemory();

		GDescriptor* GetDsvMemory();

	private:
		void BuildResource();;

		void BuildDescriptors();

	public:
		void Render(std::shared_ptr<GCommandList> cmdList) override;;
		void Update() override;;
		void ChangeRenderTargetSize(float newWidth, float newHeight) override;;
	};
}

