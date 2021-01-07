// Defaults for number of lights.
#define POINT_LIGHT 0
#define SPOT_LIGHT 1
#define DIRECTIONAL_LIGHT 2

struct ObjectData
{
	float4x4 World;
	float4x4 TexTransform;
	uint MaterialIndex;
	uint gObjPad0;
	uint gObjPad1;
	uint gObjPad2;
};

ConstantBuffer<ObjectData> ObjectBuffer : register(b0);

Texture2D MaterialTexture[] : register(t0);

struct CameraData
{
	float4x4 View;
	float4x4 InvView;
	float4x4 Proj;
	float4x4 InvProj;
	float4x4 ViewProj;
	float4x4 InvViewProj;
	float4x4 ViewProjTex;
	float4x4 ShadowTransform;
	float2 RenderTargetSize;
	float2 InvRenderTargetSize;
	float3 CameraWorldPosition;
	float padding;

	float NearZ;
	float FarZ;
};

ConstantBuffer<CameraData> CameraBuffer : register(b1);

struct WorldData
{
	uint LightsCount;
	float TotalTime;
	float DeltaTime;
};

ConstantBuffer<WorldData> WorldBuffer : register(b2);


struct MaterialData
{
	float4 DiffuseColor;
	float SpecularPower;
	float AlphaThreshold;
	int DiffuseMapIndex;
	int NormalMapIndex;
};

StructuredBuffer<MaterialData> Materials : register(t0, space1);

struct LightData
{
	float4 Color;
	float3 PositionWorld;
	float3 DirectionWorld;
	float3 PositionView;
	float3 DirectionView;
	float SpotlightAngle;
	float Range;
	float Intensity;
	int Type;
	bool Enabled;
	bool Selected;
	float2 Padding;
};

StructuredBuffer<LightData> Lights : register(t1, space1);

Texture2D<float4> NormalMap : register (t0, space2);
Texture2D<float4> BaseColorMap : register (t1, space2);
Texture2D<float4> PositionTexture : register (t2, space2);
Texture2D<float> AmbientMap : register (t3, space2);
Texture2D<float> ShadowMap : register (t4, space2);


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


//
// Space transformations
//

float NdcZToScreenSpaceZ(const float depthNDC,
                         const float4x4 projection)
{
	// depthNDC = A + B / depthV, where A = projection[2, 2] and B = projection[3,2].
	const float depthV = projection._m32 / (depthNDC - projection._m22);

	return depthV;
}

int2 NdcToScreenSpace(const float2 ndcPoint,
                      const float screenTopLeftX,
                      const float screenTopLeftY,
                      const float screenWidth,
                      const float screenHeight)
{
	const int2 viewportPoint =
		int2(
			(ndcPoint.x + 1.0f) * screenWidth * 0.5f + screenTopLeftX,
			(1.0f - ndcPoint.y) * screenHeight * 0.5f + screenTopLeftY
		);

	return viewportPoint;
}

float3 ViewRayToViewPosition(const float3 normalizedViewRayV,
                             const float depthNDC,
                             const float4x4 projection)
{
	const float depthV = NdcZToScreenSpaceZ(depthNDC,
	                                        projection);

	//
	// Reconstruct full view space position (x,y,z).
	// Find t such that p = t * normalizedViewRayV.
	// p.z = t * normalizedViewRayV.z
	// t = p.z / normalizedViewRayV.z
	//
	const float3 fragmentPositionViewSpace = (depthV / normalizedViewRayV.z) * normalizedViewRayV;

	return fragmentPositionViewSpace;
}

//
// Octahedron-normal vector encoding/decoding 
//
float2 OctWrap(float2 v)
{
	return (1.0 - abs(v.yx)) * (v.xy >= 0.0 ? 1.0 : -1.0);
}

float2 Encode(float3 n)
{
	n /= (abs(n.x) + abs(n.y) + abs(n.z));
	n.xy = n.z >= 0.0 ? n.xy : OctWrap(n.xy);
	n.xy = n.xy * 0.5 + 0.5;
	return n.xy;
}

float3 Decode(float2 encN)
{
	encN = encN * 2.0 - 1.0;

	float3 n;
	n.z = 1.0 - abs(encN.x) - abs(encN.y);
	n.xy = n.z >= 0.0 ? encN.xy : OctWrap(encN.xy);
	n = normalize(n);
	return n;
}

// Convert clip space coordinates to view space
float4 ClipToView(float4 clip)
{
	// View space position.
	float4 view = mul(CameraBuffer.InvProj, clip);
	// Perspecitive projection.
	view = view / view.w;

	return view;
}

// Convert screen space coordinates to view space.
float4 ScreenToView(float4 screen)
{
	// Convert to normalized texture coordinates
	float2 texCoord = screen.xy / CameraBuffer.RenderTargetSize;

	// Convert to clip space
	float4 clip = float4(float2(texCoord.x, 1.0f - texCoord.y) * 2.0f - 1.0f, screen.z, screen.w);

	return ClipToView(clip);
}


//
// Constants
//
#define PI 3.141592f
#define F0_NON_METALS 0.04f

//
// Lighting
//

//
// Specular term:
//

// fSpecular = F(l, h) * G(l, v, h) * D(h) / 4 * dotNL * dotNV
//
// D(h) is the microgeometry normal distribution function(NDF) evaluated at the half-vector h; in other words, the
// concentration (relative to surface area) of surface points which are oriented such that they could reflect
// light from l into v.
//
// G(l, v, h) is the geometry function; it tells us the percentage of surface points with m = h that 
// are not shadowed or masked, as a function of the light direction l and the view direction v.
//
// Therefore, the product of D(h) and G(l; v; h) gives us the concentration of active surface
// points, the surface points that actively participate in the reflectance by successfully 
// reflecting light from l into v.
//
// F(l; h) is the Fresnel reflectance of the active surface points as a function of the light
// direction l and the active microgeometry normal m = h.
//
// It tells us how much of the incoming light is reflected from each of the active surface points.
//
// Finally, the denominator 4 * dotNL * dotNV is a correction factor, 
// which accounts for quantities being transformed between the local space of the microgeometry
// and that of the overall macrosurface.
//

//
// Fresnel Reflectance:
//
// The Fresnel reflectance function computes the fraction of light reflected from an optically
// flat surface.
//
// Its value depends on two things : the incoming angle (the angle between the light vector and the surface
// normal - also referred to as the incident angle or angle of incidence) and the refractive index of the
// material.
//
// Since the refractive index may vary over the visible spectrum, the Fresnel reflectance is a
// spectral quantity - for production purposes, an RGB triple.
//
// We also know that each of the RGB values have to lie within the 0 to 1 range, since a surface cannot 
// reflect less than 0 % or more than 100 % of the incoming light.
//
// Since the Fresnel reflectance stays close to the value for 0 over most of the visible parts of a given
// 3D scene, we can think of this value(which we will denote F0) as the characteristic specular reflectance
// of the material.
// 
// This value has all the properties of what is typically thought of as a "color", it is
// composed of RGB values between 0 and 1, and it is a measure of selective reflectance of light.
// For this reason, we will also refer to this value as the specular color of the surface.
//

// Schlick fresnel:
// f0 is the normal incidence reflectance (F() at 0 degrees, used as specular color)
// f90 is the reflectance at 90 degrees
float3
F_Schlick(const float3 f0,
          const float f90,
          const float dotLH)
{
	return f0 + (f90 - f0) * pow(1.0f - dotLH, 5.0f);
}

float
Fd_Disney(const float dotVN,
          const float dotLN,
          const float dotLH,
          float linearRoughness)
{
	float energyBias = lerp(0.0f, 0.5f, linearRoughness);
	float energyFactor = lerp(1.0, 1.0 / 1.51, linearRoughness);
	float fd90 = energyBias + 2.0 * dotLH * dotLH * linearRoughness;
	float f0 = 1.0f;
	float lightScatter = F_Schlick(f0, fd90, dotLN).x;
	float viewScatter = F_Schlick(f0, fd90, dotVN).x;
	return lightScatter * viewScatter * energyFactor;
}

//
// Normal distribution Function:
//
// The microgeometry in most surfaces does not have uniform distributions of surface point orientations.
//
// More surface points have normals pointing "up" (towards the macroscopic surface normal n) than
// "sideways" (away from n). 
//
// The statistical distribution of surface orientations is defined via the microgeometry 
// normal distribution function D(m). 
//
// Unlike F(), the value of D() is not restricted to lie between 0 and 1, although values must be non-negative, 
// they can be arbitrarily large (indicating a very high concentration of surface points with normals 
// pointing in a particular direction).
//
// Also, unlike F(), the function D() is not spectral nor RGB valued, but scalar valued.
//
// In microfacet BRDF terms, D() is evaluated for the direction h, to help determine 
// the concentration of potentially active surface points(those for which m = h).
//
// The function D() determines the size, brightness, and shape of the specular highlight.
//

// GGX/Trowbridge-Reitz
// m is roughness
float
D_TR(const float m,
     const float dotNH)
{
	const float m2 = m * m;
	const float denom = dotNH * dotNH * (m2 - 1.0f) + 1.0f;
	return m2 / (PI * denom * denom);
}

//
// Geometry function:
//
// The geometry function G(l, v, h) represents the probability that surface points with a given microgeometry
// normal m will be visible from both the light direction l and the view direction v.
//
// In the microfacet BRDF, m is replaced with h (for similar reasons as in the previous two terms).
//
// Since the function G() represents a probability, its value is a scalar and constrained to lie between 0 and 1.
//
// The geometry function typically does not introduce any new parameters to the BRDF; it either has no parameters, or
// uses the roughness parameter(s) of the D() function.
//
// In many cases, the geometry function partially cancels out the dotNL * dotNV denominator in fSpecular equation, replacing it with some other expression.
// The geometry function is essential for BRDF energy conservation, without such a term the BRDF
// can reflect arbitrarily more light energy than it receives.
//
// A key part of the microfacet BRDF derivation relates to the ratio between the active surface area
// (the area covered by surface regions that reflect light energy from l to v) and 
// the total surface area of the macroscopic surface.If shadowing and masking
// are not accounted for, then the active area may exceed the total area, an obvious impossibility which
// can lead to the BRDF not conserving energy, in some cases by a huge amount

float
V_SmithGGXCorrelated(float dotNL,
                     float dotNV,
                     float alphaG)
{
	// Original formulation of G_SmithGGX Correlated
	// lambda_v = (-1 + sqrt ( alphaG2 * (1 - dotNL2 ) / dotNL2 + 1)) * 0.5 f;
	// lambda_l = (-1 + sqrt ( alphaG2 * (1 - dotNV2 ) / dotNV2 + 1)) * 0.5 f;
	// G_SmithGGXCorrelated = 1 / (1 + lambda_v + lambda_l );
	// V_SmithGGXCorrelated = G_SmithGGXCorrelated / (4.0 f * dotNL * dotNV );

	// This is the optimized version
	float alphaG2 = alphaG * alphaG;
	// Caution : the " dotNL *" and " dotNV *" are explicitely inversed , this is not a mistake .
	float Lambda_GGXV = dotNL * sqrt((-dotNV * alphaG2 + dotNV) * dotNV + alphaG2);
	float Lambda_GGXL = dotNV * sqrt((-dotNL * alphaG2 + dotNL) * dotNL + alphaG2);

	return 0.5f / (Lambda_GGXV + Lambda_GGXL);
}

float
G_SmithGGX(const float dotNL,
           const float dotNV,
           float alpha)
{
	const float alphaSqr = alpha * alpha;
	const float G_V = dotNV + sqrt((dotNV - dotNV * alphaSqr) * dotNV + alphaSqr);
	const float G_L = dotNL + sqrt((dotNL - dotNL * alphaSqr) * dotNL + alphaSqr);

	return rcp(G_V * G_L);
}

//
// Diffuse term:
//

// Lambertian diffuse term
float3
Fd_Lambert(const float3 diffuseColor)
{
	return diffuseColor / PI;
}

float
Fr_DisneyDiffuse(const float dotNV,
                 const float dotNL,
                 const float dotLH,
                 const float linearRoughness)
{
	const float energyBias = lerp(0, 0.5, linearRoughness);
	const float energyFactor = lerp(1.0, 1.0 / 1.51, linearRoughness);
	const float fd90 = energyBias + 2.0 * dotLH * dotLH * linearRoughness;
	const float3 f0 = float3(1.0f, 1.0f, 1.0f);
	const float lightScatter = F_Schlick(f0, fd90, dotNL).r;
	const float viewScatter = F_Schlick(f0, fd90, dotNV).r;

	return lightScatter * viewScatter * energyFactor;
}

//
// Image Based Lighting
//

float3
DiffuseIBL(const float3 baseColor,
           const float metalness,
           SamplerState textureSampler,
           const float3 normalWorldSpace)
{
	//// When we sample a cube map, we need to use data in world space, not view space.
	//const float3 diffuseReflection = diffuseIBLCubeMap.SampleLevel(textureSampler,
	//    normalWorldSpace,
	//    0).rgb;
	//const float3 diffuseColor = (1.0f - metalness) * baseColor;

	//return diffuseColor * diffuseReflection;
	return (1.0f - metalness) * baseColor;
}

float3
SpecularIBL(const float3 baseColor,
            const float metalness,
            const float roughness,
            SamplerState textureSampler,
            const float3 viewVectorViewSpace,
            const float3 positionWorldSpace,
            const float3 eyePositionWorldSpace,
            const float3 normalWorldSpace,
            const float3 normalViewSpace)
{
	// Compute incident vector. 
	// When we sample a cube map, we need to use data in world space, not view space.
	const float3 incidentVectorWorldSpace = positionWorldSpace - eyePositionWorldSpace;
	const float3 reflectionVectorWorldSpace = reflect(incidentVectorWorldSpace,
	                                                  normalWorldSpace);

	// Our cube map has 10 mip map levels
	const int mipmap = roughness * 10.0f;
	/* const float3 specularReflection = specularIBLCubeMap.SampleLevel(textureSampler,
	     reflectionVectorWorldSpace,
	     mipmap).rgb;*/

	// Specular reflection color
	const float3 dielectricColor = float3(0.04f, 0.04f, 0.04f);
	const float3 f0 = lerp(dielectricColor, baseColor, metalness);
	const float3 F = F_Schlick(f0,
	                           1.0f,
	                           dot(viewVectorViewSpace, normalViewSpace));

	return F; // *specularReflection;
}

//
// BRDF
//

float3
DiffuseBrdf(const float3 baseColor,
            const float metalness)
{
	const float3 diffuseColor = (1.0f - metalness) * baseColor;
	return Fd_Lambert(diffuseColor);
}

float3
SpecularBrdf(const float3 N,
             const float3 V,
             const float3 L,
             const float3 baseColor,
             const float roughness,
             const float metalness)
{
	// Disney's reparametrization of roughness
	const float alpha = roughness * roughness;

	const float3 H = normalize(V + L);
	const float dotNL = abs(dot(N, V)) + 1e-5f;
	const float dotNV = abs(dot(N, V)) + 1e-5f; // avoid artifacts
	const float dotNH = saturate(dot(N, H));
	const float dotLH = saturate(dot(L, H));

	//
	// Specular term: (D * F * G) / (4 * dotNL * dotNV)
	//

	const float D = D_TR(roughness, dotNH);

	const float3 f0 = (1.0f - metalness) * float3(F0_NON_METALS,
	                                              F0_NON_METALS,
	                                              F0_NON_METALS) + baseColor * metalness;
	const float3 F = F_Schlick(f0, 1.0f, dotLH);

	// G / (4 * dotNL * dotNV)
#ifdef V_SMITH
    const float G_Correlated = V_SmithGGXCorrelated(dotNV,
        dotNL,
        alpha);
#else
	const float G_Correlated = G_SmithGGX(dotNL,
	                                      dotNV,
	                                      alpha);
#endif

	return D * F * G_Correlated;
}


struct Plane
{
	float3 N; // Plane normal.
	float d; // Distance to origin.
};

struct Sphere
{
	float3 c; // Center point.
	float r; // Radius.
};

struct Cone
{
	float3 T; // Cone tip.
	float h; // Height of the cone.
	float3 d; // Direction of the cone.
	float r; // bottom radius of the cone.
};

// Four planes of a view frustum (in view space).
// The planes are:
//  * Left,
//  * Right,
//  * Top,
//  * Bottom.
// The back and/or front planes can be computed from depth values in the 
// light culling compute shader.
struct Frustum
{
	Plane planes[4]; // left, right, top, bottom frustum planes.
};


// Compute a plane from 3 noncollinear points that form a triangle.
// This equation assumes a right-handed (counter-clockwise winding order) 
// coordinate system to determine the direction of the plane normal.
Plane ComputePlane(float3 p0, float3 p1, float3 p2)
{
	Plane plane;

	float3 v0 = p1 - p0;
	float3 v2 = p2 - p0;

	plane.N = normalize(cross(v0, v2));

	// Compute the distance to the origin using p0.
	plane.d = dot(plane.N, p0);

	return plane;
}

// Check to see if a sphere is fully behind (inside the negative halfspace of) a plane.
// Source: Real-time collision detection, Christer Ericson (2005)
bool SphereInsidePlane(Sphere sphere, Plane plane)
{
	return dot(plane.N, sphere.c) - plane.d < -sphere.r;
}

// Check to see if a point is fully behind (inside the negative halfspace of) a plane.
bool PointInsidePlane(float3 p, Plane plane)
{
	return dot(plane.N, p) - plane.d < 0;
}

// Check to see if a cone if fully behind (inside the negative halfspace of) a plane.
// Source: Real-time collision detection, Christer Ericson (2005)
bool ConeInsidePlane(Cone cone, Plane plane)
{
	// Compute the farthest point on the end of the cone to the positive space of the plane.
	float3 m = cross(cross(plane.N, cone.d), cone.d);
	float3 Q = cone.T + cone.d * cone.h - m * cone.r;

	// The cone is in the negative halfspace of the plane if both
	// the tip of the cone and the farthest point on the end of the cone to the 
	// positive halfspace of the plane are both inside the negative halfspace 
	// of the plane.
	return PointInsidePlane(cone.T, plane) && PointInsidePlane(Q, plane);
}

// Check to see of a light is partially contained within the frustum.
bool SphereInsideFrustum(Sphere sphere, Frustum frustum, float zNear, float zFar)
{
	bool result = true;

	// First check depth
	// Note: Here, the view vector points in the -Z axis so the 
	// far depth value will be approaching -infinity.
	if (sphere.c.z - sphere.r > zNear || sphere.c.z + sphere.r < zFar)
	{
		result = false;
	}

	// Then check frustum planes
	for (int i = 0; i < 4 && result; i++)
	{
		if (SphereInsidePlane(sphere, frustum.planes[i]))
		{
			result = false;
		}
	}

	return result;
}

bool ConeInsideFrustum(Cone cone, Frustum frustum, float zNear, float zFar)
{
	bool result = true;

	Plane nearPlane = {float3(0, 0, -1), -zNear};
	Plane farPlane = {float3(0, 0, 1), zFar};

	// First check the near and far clipping planes.
	if (ConeInsidePlane(cone, nearPlane) || ConeInsidePlane(cone, farPlane))
	{
		result = false;
	}

	// Then check frustum planes
	for (int i = 0; i < 4 && result; i++)
	{
		if (ConeInsidePlane(cone, frustum.planes[i]))
		{
			result = false;
		}
	}

	return result;
}

float3 ExpandNormal(float3 n)
{
	return n * 2.0f - 1.0f;
}

// This lighting result is returned by the 
// lighting functions for each light type.
struct LightingResult
{
	float4 Diffuse;
	float4 Specular;
};

float4 DoNormalMapping(float3x3 TBN, Texture2D tex, sampler s, float2 uv)
{
	float3 normal = tex.Sample(s, uv).xyz;
	normal = ExpandNormal(normal);

	// Transform normal from tangent space to view space.
	normal = mul(normal, TBN);
	return normalize(float4(normal, 0));
}

float4 DoBumpMapping(float3x3 TBN, Texture2D tex, sampler s, float2 uv, float bumpScale)
{
	// Sample the heightmap at the current texture coordinate.
	float height_00 = tex.Sample(s, uv).r * bumpScale;
	// Sample the heightmap in the U texture coordinate direction.
	float height_10 = tex.Sample(s, uv, int2(1, 0)).r * bumpScale;
	// Sample the heightmap in the V texture coordinate direction.
	float height_01 = tex.Sample(s, uv, int2(0, 1)).r * bumpScale;

	float3 p_00 = {0, 0, height_00};
	float3 p_10 = {1, 0, height_10};
	float3 p_01 = {0, 1, height_01};

	// normal = tangent x bitangent
	float3 normal = cross(normalize(p_10 - p_00), normalize(p_01 - p_00));

	// Transform normal from tangent space to view space.
	normal = mul(normal, TBN);

	return float4(normal, 0);
}

float4 DoDiffuse(LightData light, float4 L, float4 N)
{
	float NdotL = max(dot(N, L), 0);
	return light.Color * NdotL;
}

float4 DoSpecular(LightData light, MaterialData material, float4 V, float4 L, float4 N)
{
	float4 R = normalize(reflect(-L, N));
	float RdotV = max(dot(R, V), 0);

	return light.Color * pow(RdotV, material.SpecularPower);
}

// Compute the attenuation based on the range of the light.
float DoAttenuation(LightData light, float d)
{
	return 1.0f - smoothstep(light.Range * 0.75f, light.Range, d);
}

float DoSpotCone(LightData light, float4 L)
{
	// If the cosine angle of the light's direction 
	// vector and the vector from the light source to the point being 
	// shaded is less than minCos, then the spotlight contribution will be 0.
	float minCos = cos(radians(light.SpotlightAngle));
	// If the cosine angle of the light's direction vector
	// and the vector from the light source to the point being shaded
	// is greater than maxCos, then the spotlight contribution will be 1.
	float maxCos = lerp(minCos, 1, 0.5f);
	float cosAngle = dot(light.DirectionView, -L);
	// Blend between the maxixmum and minimum cosine angles.
	return smoothstep(minCos, maxCos, cosAngle);
}

LightingResult DoPointLight(LightData light, MaterialData mat, float4 V, float4 P, float4 N)
{
	LightingResult result;

	float4 L = float4(light.PositionWorld, 1) - P;
	float distance = length(L);
	L = L / distance;

	float attenuation = DoAttenuation(light, distance);

	result.Diffuse = DoDiffuse(light, L, N) * attenuation * light.Intensity;
	result.Specular = DoSpecular(light, mat, V, L, N) * attenuation * light.Intensity;

	return result;
}

LightingResult DoDirectionalLight(LightData light, MaterialData mat, float4 V, float4 P, float4 N)
{
	LightingResult result;

	float4 L = normalize(float4(-light.DirectionView, 1));

	result.Diffuse = DoDiffuse(light, L, N) * light.Intensity;
	result.Specular = DoSpecular(light, mat, V, L, N) * light.Intensity;

	return result;
}

LightingResult DoSpotLight(LightData light, MaterialData mat, float4 V, float4 P, float4 N)
{
	LightingResult result;

	float4 L = float4(light.PositionWorld, 1) - P;
	float distance = length(L);
	L = L / distance;

	float attenuation = DoAttenuation(light, distance);
	float spotIntensity = DoSpotCone(light, L);

	result.Diffuse = DoDiffuse(light, L, N) * attenuation * spotIntensity * light.Intensity;
	result.Specular = DoSpecular(light, mat, V, L, N) * attenuation * spotIntensity * light.Intensity;

	return result;
}
