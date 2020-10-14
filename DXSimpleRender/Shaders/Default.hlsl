#include "Common.hlsl"

struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 TexC : TEXCOORD;
	float3 TangentU : TANGENT;
};

struct VertexOut
{
	float4 PosView : SV_POSITION;
	float4 ShadowPosH : POSITION0;
	float4 SsaoPosH : POSITION1;
	float3 PosW : POSITION2;
	float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexC : TEXCOORD;
};


VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;
	
	float4 posW = mul(float4(vin.PosL, 1.0f), Object.World);
	vout.PosW = posW.xyz;

	vout.NormalW = mul(vin.NormalL, (float3x3)Object.World);
	vout.PosView = mul(posW, World.ViewProj);
	vout.TangentW = mul(vin.TangentU, (float3x3)Object.World);
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), Object.TexTransform);
	vout.TexC = texC.xy;

	// Generate projective tex-coords to project SSAO map onto scene.
	vout.SsaoPosH = mul(posW, World.ViewProjTex);

	// Generate projective tex-coords to project shadow map onto scene.
	vout.ShadowPosH = mul(posW, World.ShadowTransform);

	return vout;
}

struct PixelShaderOutput
{
	float4 LightAccumulation    : SV_Target0;   // Ambient + emissive (R8G8B8_????) Unused (A8_UNORM)
	float4 Diffuse              : SV_Target1;   // Diffuse Albedo (R8G8B8_UNORM) Unused (A8_UNORM)
	float4 Specular             : SV_Target2;   // Specular Color (R8G8B8_UNROM) Specular Power(A8_UNORM)
	float4 NormalVS             : SV_Target3;   // View space normal (R32G32B32_FLOAT) Unused (A32_FLOAT)
};


PixelShaderOutput PS(VertexOut pin)
{
	PixelShaderOutput o;
	o.LightAccumulation = float4(1.0,1.0,1.0,1.0);
	o.Diffuse = float4(1.0,0.0,0.0,1.0);
	o.Specular = float4(1.0,1.0,0.0,1.0);
	o.NormalVS = float4(1.0,0.0,1.0,1.0);
	return o;
}
