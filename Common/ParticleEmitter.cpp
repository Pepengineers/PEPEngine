#include "ParticleEmitter.h"
#include "pch.h"

#include "AssetDatabase.h"
#include "ATexture.h"
#include "GameObject.h"
#include "GDeviceFactory.h"
#include "GTexture.h"
#include "MathHelper.h"
#include "Transform.h"
#include "ATexture.h"

namespace PEPEngine::Common
{
	double ParticleEmitter::CalculateGroupCount(DWORD particleCount) const
	{
		auto numGroups = (particleCount % 1024 != 0) ? ((particleCount / 1024) + 1) : (particleCount / 1024);
		auto secondRoot = std::pow(static_cast<double>(numGroups), static_cast<double>(1.0 / 2.0));
		secondRoot = std::ceil(secondRoot);
		return secondRoot;
	}

	void ParticleEmitter::DescriptorInitialize()
	{
		particlesComputeDescriptors = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4);

		particlesRenderDescriptors = device->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		                                                         2 + Atlas.size());


		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		for (int i = 0; i < Atlas.size(); ++i)
		{
			auto texture = Atlas[i];
			auto desc = texture->GetGTexture()->GetD3D12ResourceDesc();

			srvDesc.Format = desc.Format;
			srvDesc.Texture2D.PlaneSlice = 0;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;

			texture->GetGTexture()->CreateShaderResourceView(&srvDesc, &particlesRenderDescriptors, 2 + i);
		}
	}

	void ParticleEmitter::BufferInitialize()
	{
		if (objectPositionBuffer == nullptr)
			objectPositionBuffer = std::make_shared<ConstantUploadBuffer<ObjectConstants>>(
				device, 1, L"Emitter Position");

		if (ParticlesPool)
		{
			ParticlesPool->Reset();
			ParticlesPool.reset();
		}

		if (InjectedParticles)
		{
			InjectedParticles->Reset();
			InjectedParticles.reset();
		}

		if (ParticlesAlive)
		{
			ParticlesAlive->Reset();
			ParticlesAlive.reset();
		}

		if (ParticlesDead)
		{
			ParticlesDead->Reset();
			ParticlesDead.reset();
		}


		ParticlesPool = std::make_shared<GBuffer>(device, sizeof(ParticleData), emitterData.ParticlesTotalCount,
		                                          L"Particles Pool Buffer", D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		InjectedParticles = std::make_shared<GBuffer>(device, sizeof(ParticleData), emitterData.ParticleInjectCount,
		                                              L"Injected Particle Buffer",
		                                              D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		ParticlesAlive = std::make_shared<CounteredStructBuffer<DWORD>>(device, emitterData.ParticlesTotalCount,
		                                                                L"Particles Alive Index Buffer");
		ParticlesDead = std::make_shared<CounteredStructBuffer<DWORD>>(device, emitterData.ParticlesTotalCount,
		                                                               L"Particles Dead Index Buffer");

		{
			std::vector<UINT> deadIndex;

			for (int i = 0; i < emitterData.ParticlesTotalCount; ++i)
			{
				deadIndex.push_back(i);
			}

			auto queue = device->GetCommandQueue();
			auto cmdList = queue->GetCommandList();

			ParticlesDead->LoadData(deadIndex.data(), cmdList);
			ParticlesDead->SetCounterValue(cmdList, emitterData.ParticlesTotalCount);

			cmdList->TransitionBarrier(ParticlesDead->GetD3D12Resource(), D3D12_RESOURCE_STATE_COMMON);
			cmdList->FlushResourceBarriers();

			queue->ExecuteCommandList(cmdList);
			queue->Flush();
		}

		ParticlesDead->ReadCounter(&emitterData.ParticlesAliveCount);
		ParticlesAlive->ReadCounter(&emitterData.ParticlesAliveCount);


		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = ParticlesPool->GetElementCount();
		uavDesc.Buffer.StructureByteStride = ParticlesPool->GetStride();
		uavDesc.Buffer.CounterOffsetInBytes = 0;

		ParticlesPool->CreateUnorderedAccessView(&uavDesc, &particlesComputeDescriptors, 0);

		uavDesc.Buffer.NumElements = ParticlesDead->GetElementCount();
		uavDesc.Buffer.StructureByteStride = ParticlesDead->GetStride();
		uavDesc.Buffer.CounterOffsetInBytes = ParticlesDead->GetBufferSize() - sizeof(DWORD);
		ParticlesDead->CreateUnorderedAccessView(&uavDesc, &particlesComputeDescriptors, 1,
		                                         ParticlesDead->GetD3D12Resource());
		ParticlesAlive->CreateUnorderedAccessView(&uavDesc, &particlesComputeDescriptors, 2,
		                                          ParticlesAlive->GetD3D12Resource());

		uavDesc.Buffer.NumElements = InjectedParticles->GetElementCount();
		uavDesc.Buffer.StructureByteStride = InjectedParticles->GetStride();
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		InjectedParticles->CreateUnorderedAccessView(&uavDesc, &particlesComputeDescriptors, 3);


		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = emitterData.ParticlesTotalCount;
		srvDesc.Buffer.StructureByteStride = ParticlesPool->GetStride();

		ParticlesPool->CreateShaderResourceView(&srvDesc, &particlesRenderDescriptors, 0);

		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = ParticlesAlive->GetElementCount();
		srvDesc.Buffer.StructureByteStride = ParticlesAlive->GetStride();
		ParticlesAlive->CreateShaderResourceView(&srvDesc, &particlesRenderDescriptors, 1);


		newParticles.resize(InjectedParticles->GetElementCount());
	}

	void ParticleEmitter::Serialize(json& j)
	{
		auto jPos = json();

		jPos["ParticlesTotalCount"] = emitterData.ParticlesTotalCount;
		jPos["AtlasTexturesCount"] = Atlas.size();

		auto atlasMaps = json::array();
		for (auto && texture : Atlas)
		{
			auto tex = json();
			tex["id"] = texture->GetID();
		}

		jPos["Atlas"] = atlasMaps;
		
		j["EmitterData"] = jPos;
	}

	void ParticleEmitter::Deserialize(json& j)
	{
		auto jPos = j["EmitterData"];

		DWORD count;

		assert(Asset::TryReadVariable(jPos, "ParticlesTotalCount", &count));

		UINT atlasSize;
		assert(Asset::TryReadVariable(jPos, "AtlasTexturesCount", &atlasSize));

		Atlas.resize(atlasSize);
		
		auto atlasMaps = jPos["Atlas"];
		for (int i = 0; i < atlasSize; ++i)
		{
			UINT64 id;
			assert(Asset::TryReadVariable(atlasMaps[i], "id", &id));
			
			auto texture = AssetDatabase::FindAssetByID<ATexture>(id);
			
			Atlas[i] = texture;
		}
		
		Initialize(count);
	}

	void ParticleEmitter::ChangeParticleCount(UINT count)
	{
		emitterData.ParticlesTotalCount = count;
		emitterData.Color = DirectX::Colors::Blue;
		emitterData.Size = Vector2::One * 5.0f;
		emitterData.DeltaTime = 1.0f / 60.0f;
		emitterData.Force = Vector3(0, 9.8f, 0) * -1;
		emitterData.SimulatedGroupCount = CalculateGroupCount(count);
		emitterData.UseTexture = true;
		emitterData.ParticleInjectCount = count / 16;
		emitterData.InjectedGroupCount = CalculateGroupCount(emitterData.ParticleInjectCount);

		BufferInitialize();
	}


	void ParticleEmitter::Initialize(DWORD particleCount)
	{
		if(Atlas.size() <= 0)
		{	
			Atlas.resize(64);
			for (int i = 1; i <= 64; ++i)
			{
				auto image = AssetDatabase::Get<ATexture>(L"tile (" + std::to_wstring(i) + L")");
				if(image == nullptr)
				{
					image = AssetDatabase::AddTexture(L"Data\\Particles\\tile (" + std::to_wstring(i) + L").png",
						std::filesystem::path("ParticleAtlas").concat("\\tile (" + std::to_string(i) + ").png"));
				}
				
				Atlas[i] = (image);
			}

			emitterData.AtlasTextureCount = Atlas.size();

		}


		
		this->device = GDeviceFactory::GetDevice();

		emitterData.ParticlesTotalCount = particleCount;
		emitterData.Color = DirectX::Colors::Blue;
		emitterData.Size = Vector2::One * 5.0f;
		emitterData.DeltaTime = 1.0f / 60.0f;
		emitterData.Force = Vector3(0, 9.8f, 0) * -1;
		emitterData.SimulatedGroupCount = CalculateGroupCount(particleCount);
		emitterData.UseTexture = true;

		emitterData.ParticleInjectCount = particleCount / 16;


		emitterData.InjectedGroupCount = CalculateGroupCount(emitterData.ParticleInjectCount);
		
		PSOInitialize();

		DescriptorInitialize();

		BufferInitialize();
	}

	ParticleEmitter::ParticleEmitter(DWORD particleCount): Emitter()
	{
		Initialize(particleCount);
	}

	void ParticleEmitter::Update()
	{
		const auto transform = gameObject->GetTransform();

		if (transform->IsDirty())
		{
			objectWorldData.TextureTransform = transform->TextureTransform.Transpose();
			objectWorldData.World = (transform->GetWorldMatrix()).Transpose();
			objectPositionBuffer->CopyData(0, objectWorldData);
		}
	}

	void ParticleEmitter::PopulateDrawCommand(std::shared_ptr<Graphics::GCommandList> cmdList)
	{
		cmdList->TransitionBarrier(ParticlesPool->GetD3D12Resource(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cmdList->TransitionBarrier(ParticlesAlive->GetD3D12Resource(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		cmdList->FlushResourceBarriers();

		cmdList->SetRootSignature(renderSignature.get());
		cmdList->SetDescriptorsHeap(&particlesRenderDescriptors);

		cmdList->SetRootConstantBufferView(ParticleRenderSlot::ObjectData, *objectPositionBuffer.get());

		cmdList->SetRoot32BitConstants(ParticleRenderSlot::EmitterData, sizeof(EmitterData) / sizeof(float),
		                               &emitterData, 0);

		cmdList->SetRootDescriptorTable(ParticleRenderSlot::ParticlesPool, &particlesRenderDescriptors, 0);

		cmdList->SetRootDescriptorTable(ParticleRenderSlot::ParticlesAliveIndex, &particlesRenderDescriptors, 1);

		cmdList->SetRootDescriptorTable(ParticleRenderSlot::Atlas, &particlesRenderDescriptors, 2);


		cmdList->SetPipelineState(*renderPSO.get());
		cmdList->SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		cmdList->SetIBuffer();
		cmdList->SetVBuffer();
		cmdList->Draw(emitterData.ParticlesAliveCount);

		cmdList->TransitionBarrier(ParticlesPool->GetD3D12Resource(), D3D12_RESOURCE_STATE_COMMON);
		cmdList->TransitionBarrier(ParticlesAlive->GetD3D12Resource(), D3D12_RESOURCE_STATE_COMMON);
		cmdList->FlushResourceBarriers();
	}


	void ParticleEmitter::Dispatch(std::shared_ptr<GCommandList> cmdList)
	{
		isWorked = true;

		ParticlesAlive->ReadCounter(&emitterData.ParticlesAliveCount);

		emitterData.ParticlesAliveCount = std::clamp(emitterData.ParticlesAliveCount, static_cast<DWORD>(0),
		                                             emitterData.ParticlesTotalCount);


		cmdList->TransitionBarrier(ParticlesPool->GetD3D12Resource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		cmdList->TransitionBarrier(ParticlesAlive->GetD3D12Resource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		cmdList->TransitionBarrier(ParticlesDead->GetD3D12Resource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		cmdList->FlushResourceBarriers();

		cmdList->SetRootSignature(computeSignature.get());
		cmdList->SetDescriptorsHeap(&particlesComputeDescriptors);

		cmdList->SetRootDescriptorTable(ParticleComputeSlot::ParticlesPool, &particlesComputeDescriptors,
		                                ParticleComputeSlot::ParticlesPool - 1);
		cmdList->SetRootDescriptorTable(ParticleComputeSlot::ParticleDead, &particlesComputeDescriptors,
		                                ParticleComputeSlot::ParticleDead - 1);
		cmdList->SetRootDescriptorTable(ParticleComputeSlot::ParticleAlive, &particlesComputeDescriptors,
		                                ParticleComputeSlot::ParticleAlive - 1);

		if (emitterData.ParticlesTotalCount > emitterData.ParticlesAliveCount)
		{
			if (emitterData.ParticlesAliveCount + emitterData.ParticleInjectCount <= emitterData.ParticlesTotalCount)
			{
				cmdList->TransitionBarrier(InjectedParticles->GetD3D12Resource(),
				                           D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

				cmdList->FlushResourceBarriers();

				for (int i = 0; i < emitterData.ParticleInjectCount; ++i)
				{
					newParticles[i] = GenerateParticle();
				}

				InjectedParticles->LoadData(newParticles.data(), cmdList);

				cmdList->SetPipelineState(*injectedPSO.get());

				cmdList->SetRootDescriptorTable(ParticleComputeSlot::ParticleInjection, &particlesComputeDescriptors,
				                                ParticleComputeSlot::ParticleInjection - 1);

				cmdList->SetRoot32BitConstants(ParticleComputeSlot::EmitterData, sizeof(EmitterData) / sizeof(float),
				                               &emitterData, 0);

				cmdList->Dispatch(emitterData.InjectedGroupCount, emitterData.InjectedGroupCount, 1);

				cmdList->UAVBarrier(InjectedParticles->GetD3D12Resource());
				cmdList->FlushResourceBarriers();
			}
		}

		if (emitterData.ParticlesAliveCount > 0)
		{
			emitterData.SimulatedGroupCount = CalculateGroupCount(emitterData.ParticlesAliveCount);

			cmdList->SetRoot32BitConstants(ParticleComputeSlot::EmitterData, sizeof(EmitterData) / sizeof(float),
			                               &emitterData, 0);

			cmdList->SetPipelineState(*simulatedPSO.get());


			cmdList->Dispatch(emitterData.SimulatedGroupCount, emitterData.SimulatedGroupCount, 1);
		}

		ParticlesAlive->CopyCounterForRead(cmdList);

		cmdList->TransitionBarrier(ParticlesPool->GetD3D12Resource(), D3D12_RESOURCE_STATE_COMMON);
		cmdList->TransitionBarrier(ParticlesAlive->GetD3D12Resource(), D3D12_RESOURCE_STATE_COMMON);
		cmdList->TransitionBarrier(ParticlesDead->GetD3D12Resource(), D3D12_RESOURCE_STATE_COMMON);
		cmdList->FlushResourceBarriers();
	}
}
