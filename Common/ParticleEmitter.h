#pragma once
#include "d3dUtil.h"
#include "DirectXBuffers.h"
#include "GDescriptor.h"
#include "Emitter.h"

namespace PEPEngine::Common
{

	class ParticleEmitter :	public Emitter
	{
		friend class CrossAdapterParticleEmitter;

		std::shared_ptr<ConstantUploadBuffer<ObjectConstants>> objectPositionBuffer = nullptr;

		std::shared_ptr<GBuffer> ParticlesPool = nullptr;
		std::shared_ptr<CounteredStructBuffer<DWORD>> ParticlesAlive = nullptr;
		std::shared_ptr<CounteredStructBuffer<DWORD>> ParticlesDead = nullptr;
		std::shared_ptr<GBuffer> InjectedParticles = nullptr;


		std::vector<ParticleData> newParticles;

		GDescriptor particlesComputeDescriptors;
		GDescriptor particlesRenderDescriptors;

		EmitterData emitterData = {};
		ObjectConstants objectWorldData{};

		int dirtyCount = Utils::globalCountFrameResources;

		bool isWorked = false;
		

	protected:
		void Update() override;
		
		void PopulateDrawCommand(std::shared_ptr<Graphics::GCommandList> cmdList) override;

		double CalculateGroupCount(DWORD particleCount) const;

		void DescriptorInitialize();
		void BufferInitialize();

		void Serialize(json& j) override;

		void Deserialize(json& j) override;

	public:	

		SERIALIZE_FROM_JSON(ParticleEmitter, Emitter)
		
		void ChangeParticleCount(UINT count);
		void Initialize(DWORD particleCount);

		ParticleEmitter(DWORD particleCount = 100);
		void Dispatch(std::shared_ptr<GCommandList> cmdList) override;
		
	};
}