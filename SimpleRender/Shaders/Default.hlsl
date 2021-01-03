#include "Common.hlsl"

struct VertexIn
{
	float3 PositionObjectSpace : POSITION;
	float3 NormalObjectSpace : NORMAL;
	float2 UV : TEXCOORD;
	float3 TangentObjectSpace : TANGENT;
};

struct VertexOut
{
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

	output.PositionWorldSpace = mul(float4(input.PositionObjectSpace, 1.0f), ObjectBuffer.World).xyz;
	output.PositionViewSpace = mul(float4(output.PositionWorldSpace, 1.0f),
	                               CameraBuffer.View).xyz;
	output.PositionClipSpace = mul(float4(output.PositionViewSpace, 1.0f),
	                               CameraBuffer.Proj);

	float4 texC = mul(float4(input.UV, 0.0f, 1.0f), ObjectBuffer.TexTransform);
	output.UV = texC.xy;

	output.NormalWorldSpace = mul(float4(input.NormalObjectSpace, 0.0f),
	                              ObjectBuffer.World).xyz;
	output.NormalViewSpace = mul(float4(output.NormalWorldSpace, 0.0f),
	                             CameraBuffer.View).xyz;

	output.TangentWorldSpace = mul(float4(input.TangentObjectSpace, 0.0f),
	                               ObjectBuffer.World).xyz;
	output.TangentViewSpace = mul(float4(output.TangentWorldSpace, 0.0f),
	                              CameraBuffer.View).xyz;

	output.BinormalWorldSpace = normalize(cross(output.NormalWorldSpace,
	                                            output.TangentWorldSpace));
	output.BinormalViewSpace = normalize(cross(output.NormalViewSpace,
	                                           output.TangentViewSpace));

	// Generate projective tex-coords to project SSAO map onto scene.
	output.SsaoPosClip = mul(output.PositionWorldSpace, CameraBuffer.ViewProjTex);

	// Generate projective tex-coords to project shadow map onto scene.
	output.ShadowPosClip = mul(output.PositionWorldSpace, CameraBuffer.ShadowTransform);

	return output;
}

struct PixelShaderOutput
{
	float4 Normal : SV_Target0;
	float4 BaseColor : SV_Target1;
	float4 PositionBuffer : SV_Target2;
};


PixelShaderOutput PS(VertexOut input)
{
	PixelShaderOutput output;
	MaterialData material = Materials[ObjectBuffer.MaterialIndex];

	float4 baseColor = MaterialTexture[material.DiffuseMapIndex].Sample(gsamAnisotropicWrap, input.UV);
	clip(baseColor.a - material.AlphaThreshold);

	float4 normalColor = MaterialTexture[material.NormalMapIndex].Sample(gsamAnisotropicWrap, input.UV);

	// Interpolating normal can unnormalize it, so renormalize it.
    input.NormalWorldSpace = normalize(input.NormalWorldSpace);

	output.BaseColor = baseColor;
    output.Normal = float4(( mul(input.NormalWorldSpace, (float3x3)CameraBuffer.View)), 0.0);
	output.PositionBuffer = float4(input.PositionWorldSpace, 1);

	return output;
}


float4 PSDebug(VertexOut input) : SV_Target
{
	MaterialData material = Materials[ObjectBuffer.MaterialIndex];

	float4 baseColor = MaterialTexture[material.DiffuseMapIndex].Sample(gsamAnisotropicWrap, input.UV);
	clip(baseColor.a - 0.1f);


	return baseColor;
}
