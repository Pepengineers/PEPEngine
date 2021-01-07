#pragma once
#include "ATexture.h"
#include "GShader.h"
#include "Renderer.h"
#include "ShaderBuffersData.h"

namespace PEPEngine::Common
{



	class Emitter : public Component
	{

	protected:
		std::shared_ptr<GRootSignature> renderSignature;
		std::shared_ptr<GRootSignature> computeSignature;
		std::shared_ptr<GraphicPSO> renderPSO;
		std::shared_ptr<ComputePSO> injectedPSO;
		std::shared_ptr<ComputePSO> simulatedPSO;
		std::shared_ptr<GShader> injectedShader;
		std::shared_ptr<GShader> simulatedShader;

		static ParticleData GenerateParticle();
		void CompileComputeShaders();

		void PSOInitialize();

		std::vector<std::shared_ptr<ATexture>> Atlas;

		std::shared_ptr<GDevice> device;

		virtual void Serialize(json& j) override
		{
			
		}

		virtual void Deserialize(json& j) override
		{
			
		}

		Emitter(json& json) :Component(json)
		{
		}

		
		
	public:

		Emitter() : Component() {  }

		virtual void PopulateDrawCommand(std::shared_ptr<Graphics::GCommandList> cmdList) = 0;
		virtual void Dispatch(std::shared_ptr<GCommandList> cmdList) = 0;
	};
}
