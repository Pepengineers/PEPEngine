#pragma once
#include <array>


#include "Camera.h"
#include "GDeviceFactory.h"
#include "MathHelper.h"
#include "RenderPass.h"

namespace PEPEngine::Common
{
	class GPass;

	class SSAOPass :
		public RenderPass
	{
		GPass& gpass;

		static const int MaxBlurRadius = 5;
		static const DXGI_FORMAT AmbientMapFormat = DXGI_FORMAT_R16_UNORM;

		UINT renderTargetWidth{};
		UINT renderTargetHeight{};

		GraphicPSO ssaoPSO;
		GraphicPSO blurPSO;

		std::shared_ptr<GRootSignature> ssaoPrimeRootSignature;

		GTexture randomVectorMap;
		GTexture ambientMap0;
		GTexture ambientMap1;

		GDescriptor randomVectorSrvMemory;
		GDescriptor ambientMapMapSrvMemory;
		GDescriptor ambientMapRtvMemory;

		Vector4 offsets[14];

		D3D12_VIEWPORT viewport{};
		D3D12_RECT scissorRect{};

		std::shared_ptr<GDevice> device;

		std::shared_ptr<ConstantUploadBuffer<SsaoConstants>> SsaoConstantUploadBuffer;

		GShader ssaoVertexShader;
		GShader ssaoPixelShader;
		GShader blurVertexShader;
		GShader blurPixelShader;

		void BuildRandomVectorTexture(std::shared_ptr<GCommandList> cmdList);

		void BuildOffsetVectors();

		GTexture CreateAmbientMap() const;

		void CreateSSAORS();

		void LoadAndCompileShaders();

		void CreatePSO();
		void BlurAmbientMap(std::shared_ptr<GCommandList> cmdList, bool horzBlur);

		void ChangeResolutionSize(UINT newWidth, UINT newHeight);

		void RebuildDescriptors();

		void GetOffsetVectors(Vector4 offsets[]);;

		std::vector<float> CalcGaussWeights(float sigma) const;;
		void Render(std::shared_ptr<GCommandList> cmdList) override;;

		void BlurAmbientMap(std::shared_ptr<GCommandList> cmdList,
		                    std::shared_ptr<ConstantUploadBuffer<SsaoConstants>> currFrame, int blurCount);

	public:


		SSAOPass(GPass& pass, UINT width, UINT height);

		UINT SsaoMapWidth() const;;
		
		UINT SsaoMapHeight() const;;

		GTexture& AmbientMap();;

		GDescriptor* AmbientMapSrv();;

		void Update() override;

		void OnResize() override;;
	};
}
