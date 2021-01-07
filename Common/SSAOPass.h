#pragma once
#include <array>


#include "Camera.h"
#include "MathHelper.h"
#include "RenderPass.h"
#include "GTexture.h"
#include "GDescriptor.h"
#include "GraphicPSO.h"
#include "GShader.h"

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

		UINT currentFrameResourceIndex = 0;
		std::array<std::shared_ptr<ConstantUploadBuffer<SsaoConstants>>, globalCountFrameResources>
		SsaoConstantUploadBuffers;

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


		void BlurAmbientMap(std::shared_ptr<GCommandList> cmdList,
		                    int blurCount);

	public:


		SSAOPass(float width, float height, GPass& pass);

		UINT SsaoMapWidth() const;;

		UINT SsaoMapHeight() const;;

		GTexture& AmbientMap();;

		GDescriptor* AmbientMapSrv();;

		void Update() override;
		void Render(std::shared_ptr<GCommandList> cmdList) override;

		void ChangeRenderTargetSize(float width, float height) override;;
	};
}
