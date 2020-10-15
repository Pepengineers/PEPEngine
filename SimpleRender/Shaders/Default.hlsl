#include "Common.hlsl"

struct VertexIn
{
	float3 PositionObjectSpace : POSITION;
	float3 NormalObjectSpace : NORMAL;
	float2 UV : TEXCOORD;
	float3 TangentObjectSpace : TANGENT;
};

struct VertexOut {
	float4 PositionClipSpace : SV_POSITION;
	float4 ShadowPosClip : POSITION0;
	float4 SsaoPosClip : POSITION1;
	float3 PositionWorldSpace : POS_WORLD;
	float3 PositionViewSpace : POS_VIEW;
	float3 NormalWorldSpace : NORMAL_WORLD;
	float3 NormalViewSpace : NORMAL_VIEW;
	float3 TangentWorldSpace : TANGENT_WORLD;
	float3 TangentViewSpace : TANGENT_VIEW;
	float3 BinormalWorldSpace : BINORMAL_WORLD;
	float3 BinormalViewSpace : BINORMAL_VIEW;
	float2 UV : TEXCOORD;
};


VertexOut VS(VertexIn input)
{
	VertexOut output = (VertexOut)0.0f;

	output.PositionWorldSpace = mul(float4(input.PositionObjectSpace, 1.0f),	ObjectBuffer.World).xyz;
	output.PositionViewSpace = mul(float4(output.PositionWorldSpace, 1.0f),
		WorldBuffer.View).xyz;
	output.PositionClipSpace = mul(float4(output.PositionViewSpace, 1.0f),
		WorldBuffer.Proj);
	
	float4 texC = mul(float4(input.UV, 0.0f, 1.0f), ObjectBuffer.TexTransform);
	output.UV = texC.xy;
	
	output.NormalWorldSpace = mul(float4(input.NormalObjectSpace, 0.0f),
		ObjectBuffer.World).xyz;
	output.NormalViewSpace = mul(float4(output.NormalWorldSpace, 0.0f),
		WorldBuffer.View).xyz;

	output.TangentWorldSpace = mul(float4(input.TangentObjectSpace, 0.0f),
		ObjectBuffer.World).xyz;
	output.TangentViewSpace = mul(float4(output.TangentWorldSpace, 0.0f),
		WorldBuffer.View).xyz;

	output.BinormalWorldSpace = normalize(cross(output.NormalWorldSpace,
		output.TangentWorldSpace));
	output.BinormalViewSpace = normalize(cross(output.NormalViewSpace,
		output.TangentViewSpace));	

	// Generate projective tex-coords to project SSAO map onto scene.
	output.SsaoPosClip = mul(output.PositionWorldSpace, WorldBuffer.ViewProjTex);

	// Generate projective tex-coords to project shadow map onto scene.
	output.ShadowPosClip = mul(output.PositionWorldSpace, WorldBuffer.ShadowTransform);

	return output;
}

struct PixelShaderOutput
{
	float4 Normal_Roughness : SV_Target0;
	float4 BaseColor_Metalness : SV_Target1;
};


PixelShaderOutput PS(VertexOut input)
{
	PixelShaderOutput output;
	MaterialData material = Materials[ObjectBuffer.MaterialIndex];
	
	float4 baseColor = MaterialTexture[material.DiffuseMapIndex].Sample(gsamAnisotropicWrap, input.UV);
	clip(baseColor.a - 0.1f);

	input.NormalWorldSpace = normalize(input.NormalWorldSpace);
	
	float4 NormalMapColor = MaterialTexture[material.NormalMapIndex].Sample(gsamAnisotropicWrap, input.UV);
	
	// Normal (encoded in view space)
	const float3 normalObjectSpace = normalize(NormalMapColor.xyz * 2.0f - 1.0f);
	const float3x3 tbnWorldSpace = float3x3(normalize(input.TangentWorldSpace),
		normalize(input.BinormalWorldSpace),
		normalize(input.NormalWorldSpace));
	const float3 normalWorldSpace = normalize(mul(normalObjectSpace, tbnWorldSpace));
	const float3x3 tbnViewSpace = float3x3(normalize(input.TangentViewSpace),
		normalize(input.BinormalViewSpace),
		normalize(input.NormalViewSpace));
	
	output.Normal_Roughness.xy = Encode(normalize(mul(normalObjectSpace,
		tbnViewSpace)));

	// Base color and metalness
	const float metalness = MaterialTexture[material.MetallicMapIndex].Sample(gsamAnisotropicWrap,
		input.UV).r;
	
	baseColor.a = metalness;

	output.BaseColor_Metalness = baseColor;
	
	// Roughness
	output.Normal_Roughness.z = MaterialTexture[material.RoughnessMapIndex].Sample(gsamAnisotropicWrap,
		input.UV).r;
		
	return output;
}
