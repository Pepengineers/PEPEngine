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
	                      CameraBuffer.InvProj);
	output.CameraToFragmentVectorViewSpace = ph.xyz / ph.w;


	return output;
}

float4 PS(VertexOut input) : SV_Target
{
	const int3 fragmentPositionScreenSpace = int3(input.PositionNDC.rg, 0);

	const float4 position = PositionTexture.Load(fragmentPositionScreenSpace);
	const float4 normal = NormalMap.Load(fragmentPositionScreenSpace);
	const float4 diffuse = BaseColorMap.Load(fragmentPositionScreenSpace);

	float4 viewDir = normalize(float4(CameraBuffer.CameraWorldPosition, 0.0) - position);
	
    float ambientAccess = AmbientMap.Sample(gsamAnisotropicWrap, input.UV, 0.0f).r;
		
    float4 resultColor = ambientAccess * diffuse;


	
	[loop]
	for (int i = 0; i < WorldBuffer.LightsCount; ++i)
	{
		if (!Lights[i].Enabled)
			continue;

		if (Lights[i].Type != DIRECTIONAL_LIGHT)
		{
			if (length(Lights[i].PositionWorld - position.xyz) > Lights[i].Range)
			{
				continue;
			}
		}

		LightingResult result = (LightingResult)0;
		MaterialData material = (MaterialData)0;
		material.SpecularPower = 15;


		switch (Lights[i].Type)
		{
		case POINT_LIGHT:
			{
				result = DoPointLight(Lights[i], material, viewDir, position, normal);
				break;
			}
		case SPOT_LIGHT:
			{
				result = DoSpotLight(Lights[i], material, viewDir, position, normal);
				break;
			}
		case DIRECTIONAL_LIGHT:
			{
				result = DoDirectionalLight(Lights[i], material, viewDir, position, normal);
				break;
			}
		}

		float4 totalColor = saturate(result.Diffuse) + saturate(result.Specular);
		resultColor += totalColor;
	}


	return resultColor;
}
