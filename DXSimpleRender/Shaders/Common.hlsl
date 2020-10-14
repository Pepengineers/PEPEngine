// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif

#define MaxLights 16

struct ObjectData
{
	float4x4 World;
	float4x4 TexTransform;
	uint materialIndex;
	uint gObjPad0;
	uint gObjPad1;
	uint gObjPad2;
};

ConstantBuffer<ObjectData> Object : register(b0);

struct WorldData
{
	float4x4 View;
	float4x4 InvView;
	float4x4 Proj;
	float4x4 InvProj;
	float4x4 ViewProj;
	float4x4 InvViewProj;
	float4x4 ViewProjTex;
	float4x4 ShadowTransform;
	float3 EyePosW;
	float debugMap;
	float2 RenderTargetSize;
	float2 InvRenderTargetSize;
	float NearZ;
	float FarZ;
	float TotalTime;
	float DeltaTime;
	float4 AmbientLight;

	float4 gFogColor;
	float gFogStart;
	float gFogRange;
	float2 cbPerObjectPad2;
};

ConstantBuffer<WorldData> World : register(b1);

struct MaterialData
{
	float4  GlobalAmbient;
	float4  AmbientColor;
	float4  EmissiveColor;
	float4  DiffuseColor;
	float4  SpecularColor;
	float4  Reflectance;
	float   Opacity;
	float   SpecularPower;
	float   IndexOfRefraction;
	bool    HasAmbientTexture;
	bool    HasEmissiveTexture;
	bool    HasDiffuseTexture;
	bool    HasSpecularTexture;
	bool    HasSpecularPowerTexture;
	bool    HasNormalTexture;
	bool    HasBumpTexture;
	bool    HasOpacityTexture;
	float   BumpIntensity;
	float   SpecularScale;
	float   AlphaThreshold;
	float2  Padding;
};

ConstantBuffer<MaterialData> Materials : register(b0, space1);

struct LightData
{
	float4   PositionWorld;
	float4   DirectionWorld;
	float4   PositionView;
	float4   DirectionView;
	float4   Color;
	float    SpotlightAngle;
	float    Range;
	float    Intensity;
	bool    Enabled;
	bool    Selected;
	uint    Type;
	float2  Padding;
};

ConstantBuffer<LightData> Lights : register(b1, space1);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);


float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N) * N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}


