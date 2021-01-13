#include "Common.hlsl"

struct VertexIn
{
	float3 PosL : POSITION;
	float2 TexC : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 TexC : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	// Transform to world space.
	float4 posW = mul(float4(vin.PosL, 1.0f), ObjectBuffer.World);

	// Transform to homogeneous clip space.
	vout.PosH = mul(posW, CameraBuffer.ViewProj);

	// Output vertex attributes for interpolation across triangle.
	//float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), ObjectBuffer.TexTransform);
	vout.TexC = vin.TexC;

	return vout;
}


void PS(VertexOut pin)
{	
	MaterialData material = Materials[ObjectBuffer.MaterialIndex];
	float4 diffuseAlbedo = MaterialTexture[material.DiffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);   
    clip(diffuseAlbedo.a - material.AlphaThreshold);
}
