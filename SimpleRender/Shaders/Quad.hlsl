#include "Common.hlsl"

struct VertexIn
{
	float3 PositionObjectSpace : POSITION;
	float2 UV : TEXCOORD;
};

struct VertexOut
{
	float4 PositionNDC : SV_POSITION;
	float3 CameraToFragmentVectorViewSpace : VIEW_RAY;
	float2 UV : TEXCOORD;
};

VertexOut VS(VertexIn input)
{
	VertexOut output;

	output.UV = input.UV;
	
	// Quad covering screen in NDC space ([-1.0, 1.0] x [-1.0, 1.0] x [0.0, 1.0] x [1.0])
	output.PositionNDC = float4(2.0f * output.UV.x - 1.0f,
		1.0f - 2.0f * output.UV.y,
		0.0f,
		1.0f);

	// Transform quad corners to view space near plane.
	const float4 ph = mul(output.PositionNDC,
		WorldBuffer.InvProj);
	output.CameraToFragmentVectorViewSpace = ph.xyz / ph.w;


	return output;
}

float4 PS(VertexOut input) : SV_Target
{
    const int3 fragmentPositionScreenSpace = int3(input.PositionNDC.rg, 0);
	
    const float4 position = PositionTexture.Load(fragmentPositionScreenSpace);
    const float4 normal = NormalMap.Load(fragmentPositionScreenSpace);
    const float4 diffuse = BaseColorMap.Load(fragmentPositionScreenSpace);

    float4 resultColor;
	
    for (int i = 0; i < WorldBuffer.LightsCount; ++i)
    {
        float3 L = Lights[i].PositionWorld - position;
        float dist = length(L);

        if (dist > 12.0f)
        {
           continue;
        }

        L /= dist;

        float att = max(0.0f, 1.0f - (dist / 20.0f));

        float lightAmount = saturate(dot(normal, L));
        float3 color = lightAmount * Lights[i].Color * att;

	//Specular calc
        float3 V = WorldBuffer.CameraWorldPosition - position;
        float3 H = normalize(L + V);
        float specular = pow(saturate(dot(normal, H)), 10) * att;

        float3 finalDiffuse = color * diffuse;
        float3 finalSpecular = specular * diffuse * att;

        float4 totalColor = float4((finalDiffuse + finalSpecular), 1.0f);
        resultColor += totalColor;
    }

    return resultColor;	
}

