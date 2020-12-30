
struct ParticlesWorldData    
{	
    float TotalTime;
    float DeltaTime;
    float ParticleRadius;
    uint ParticlesCount;
};

ConstantBuffer<ParticlesWorldData> ParticlesEmitterBuffer : register(b0);

struct ParticleData
{	
    float4 Color;
	float3 Position;
    float3 Velocity;
    float3 Acceleration;
};



RWStructuredBuffer<ParticleData> Particles : register(u0);
ConsumeStructuredBuffer<uint> ParticlesIndexes : register(u1);


[numthreads(16, 16, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID, uint GI : SV_GroupIndex)
{
    uint index = ParticlesIndexes.Consume();
	
    ParticleData particle = Particles[index];

    particle.Velocity += particle.Acceleration * ParticlesEmitterBuffer.DeltaTime;
    particle.Position += particle.Velocity * ParticlesEmitterBuffer.DeltaTime;

    Particles[index] = particle;
}