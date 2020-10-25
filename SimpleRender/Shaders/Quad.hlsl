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

    float4 resultColor = diffuse * 0.1; //ambient
	
	
    for (int i = 0; i < WorldBuffer.LightsCount; ++i)
    {    // Skip lights that are not enabled.
        if (!Lights[i].Enabled)
            continue;
    	
        if (Lights[i].Type != DIRECTIONAL_LIGHT && length(Lights[i].PositionWorld - position) > Lights[i].Range)
            continue;
        float4 V = normalize(float4(WorldBuffer.CameraWorldPosition,0.0) - position);	
        LightingResult result = (LightingResult)0;
        MaterialData mat_data = (MaterialData)0;
        mat_data.SpecularPower = 15;

    	if(Lights[i].Type == POINT_LIGHT)
            result = DoPointLight(Lights[i], mat_data, V, position, normal);
         if (Lights[i].Type == SPOT_LIGHT)
            result = DoSpotLight(Lights[i], mat_data, V, position, normal);
         if (Lights[i].Type == DIRECTIONAL_LIGHT)
            result = DoDirectionalLight(Lights[i], mat_data, V, position, normal);

        float4 totalColor = saturate(result.Diffuse) + saturate(result.Specular);
        resultColor += totalColor;
       
    }


    return resultColor;	
}

