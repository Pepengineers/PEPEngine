#pragma once
#include "RenderPass.h"

namespace PEPEngine::Common
{
	using namespace Common;
	
	class GPass;
	class GModel;
	class SSAOPass;
	
	class LightPass : public RenderPass
	{
		GPass& gpass;
		SSAOPass& ssaoPass;
		
		GShader vertexShader;
		GShader pixelShader;
		GRootSignature rootSignature;
		GraphicPSO pso;

		std::shared_ptr<GModel> quadModel;

		std::shared_ptr<GDevice> device;

		void LoadAndCompileShaders();

		void CreateAndInitializeRS();

		void InitializePSO();

	public:
		LightPass(GPass& pass, SSAOPass& ssao);

		void Render(std::shared_ptr<GCommandList> cmdList) override;;
		void OnResize() override;
		void Update() override {};
	};
}


