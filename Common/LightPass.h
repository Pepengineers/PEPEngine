#pragma once
#include "RenderPass.h"

namespace PEPEngine::Common
{
	using namespace Common;

	class GPass;
	class GModel;
	class SSAOPass;
	class ShadowPass;

	class LightPass : public RenderPass
	{
		GPass& gpass;
		SSAOPass& ssaoPass;
		ShadowPass& shadowPass;

		GShader vertexShader;
		GShader pixelShader;
		GRootSignature rootSignature;
		GraphicPSO pso;

		std::shared_ptr<GModel> quadModel;

		void LoadAndCompileShaders();

		void CreateAndInitializeRS();

		void InitializePSO();

	public:
		LightPass(float width, float height, GPass& pass, SSAOPass& ssao, ShadowPass& shadow);

		void Render(std::shared_ptr<GCommandList> cmdList) override;;
		void ChangeRenderTargetSize(float width, float height) override;

		void Update() override
		{
		};
	};
}
