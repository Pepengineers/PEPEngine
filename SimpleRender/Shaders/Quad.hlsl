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
   const int3 fragmentPositionScreenSpace = int3(input.PositionNDC.xy, 0);

   // Ambient accessibility (1.0f - ambient occlussion factor)
const float ambientAccessibility = 1.0f;// AmbientMap.Load(fragmentPositionScreenSpace);

   const float4 normal_roughness = Normal_RoughnessTexture.Load(fragmentPositionScreenSpace);

   // Compute fragment position in view space
   const float fragmentZNDC = DepthTexture.Load(fragmentPositionScreenSpace);
   const float3 rayViewSpace = normalize(input.CameraToFragmentVectorViewSpace);
   const float3 fragmentPositionViewSpace = ViewRayToViewPosition(rayViewSpace,
       fragmentZNDC,
       WorldBuffer.Proj);

   const float3 fragmentPositionWorldSpace = mul(float4(fragmentPositionViewSpace, 1.0f),
       WorldBuffer.InvView).xyz;

   const float2 encodedNormal = normal_roughness.xy;
   const float3 normalViewSpace = normalize(Decode(encodedNormal));
   const float3 normalWorldSpace = normalize(mul(float4(normalViewSpace, 0.0f),
       WorldBuffer.InvView).xyz);

   const float4 baseColor_metalness = BaseColor_MetalnessTexture.Load(fragmentPositionScreenSpace);
   const float3 baseColor = baseColor_metalness.xyz;
   const float metalness = baseColor_metalness.w;

   // As we are working at view space, we do not need camera position to 
   // compute vector from geometry position to camera.
   const float3 fragmentPositionToCameraViewSpace = normalize(-fragmentPositionViewSpace);

   const float3 indirectDiffuseColor = DiffuseIBL(baseColor,
       metalness,
       gsamAnisotropicWrap,
       normalWorldSpace);

   const float3 indirectSpecularColor = SpecularIBL(baseColor,
       metalness,
       normal_roughness.z,
       gsamAnisotropicWrap,
       fragmentPositionToCameraViewSpace,
       fragmentPositionWorldSpace,
       WorldBuffer.EyePosW,
       normalWorldSpace,
       normalViewSpace);

   const float3 color = indirectDiffuseColor + indirectSpecularColor;


	return float4(color * ambientAccessibility, 1.0f);
	
}

