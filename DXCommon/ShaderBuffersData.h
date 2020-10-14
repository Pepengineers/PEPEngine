#pragma once
#include <SimpleMath.h>

#define MaxLights 16

namespace DX
{
	namespace Common
	{
		using namespace DirectX::SimpleMath;

		static const UINT MaxMaterialTexturesMaps = 6;
		
		static const DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		static const DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		static const DXGI_FORMAT AmbientMapFormat = DXGI_FORMAT_R16_UNORM;
		static const DXGI_FORMAT NormalMapFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		static const DXGI_FORMAT DepthMapFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		struct Vertex
		{
			Vertex()
			{
			}

			Vertex(
				Vector3& p,
				Vector3& n,
				Vector3& t,
				Vector2& uv) :
				Position(p),
				Normal(n),
				TexCord(uv),
				TangentU(t)
			{
			}

			Vertex(
				float px, float py, float pz,
				float nx, float ny, float nz,
				float tx, float ty, float tz,
				float u, float v) :
				Position(px, py, pz),
				Normal(nx, ny, nz),
				TexCord(u, v),
				TangentU(tx, ty, tz)
			{
			}

			Vector3 Position;
			Vector3 Normal;
			Vector2 TexCord;
			Vector3 TangentU;
		};

		enum LightType : UINT
		{		
			Point = 0,
			Spot,
			Directional,
		};

		struct LightData
		{
			Vector4   PositionWorld;
			Vector4   DirectionWorld;
			Vector4   PositionView;
			Vector4   DirectionView;
			Vector4   Color;			
			float    SpotlightAngle;
			float    Range;
			float    Intensity;
			bool    Enabled;
			bool    Selected;
			UINT    Type;
			Vector2  Padding;
		};

		struct PassConstants
		{
			Matrix View = Matrix::Identity;
			Matrix InvView = Matrix::Identity;
			Matrix Proj = Matrix::Identity;
			Matrix InvProj = Matrix::Identity;
			Matrix ViewProj = Matrix::Identity;
			Matrix InvViewProj = Matrix::Identity;
			Matrix ViewProjTex = Matrix::Identity;
			Matrix ShadowTransform = Matrix::Identity;
			Vector3 EyePosW = Vector3{0.0f, 0.0f, 0.0f};
			float debugMap = 0.0f;
			Vector2 RenderTargetSize = Vector2{0.0f, 0.0f};
			Vector2 InvRenderTargetSize = Vector2{0.0f, 0.0f};
			float NearZ = 0.0f;
			float FarZ = 0.0f;
			float TotalTime = 0.0f;
			float DeltaTime = 0.0f;
			Vector4 AmbientLight = Vector4{0.0f, 0.0f, 0.0f, 1.0f};
			Vector4 FogColor = Vector4{0.7f, 0.7f, 0.7f, 1.0f};
			float gFogStart = 5.0f;
			float gFogRange = 150.0f;
			Vector2 cbPerObjectPad2;
		};

		struct ObjectConstants
		{
			Matrix World = Matrix::Identity;
			Matrix TextureTransform = Matrix::CreateScale(Vector3::One);
			UINT MaterialIndex = 0;
			UINT gObjPad0;
			UINT gObjPad1;
			UINT gObjPad2;
		};

		struct SsaoConstants
		{
			Matrix Proj;
			Matrix InvProj;
			Matrix ProjTex;
			Vector4 OffsetVectors[14];

			// For SsaoBlur.hlsl
			Vector4 BlurWeights[3];

			Vector2 InvRenderTargetSize = {0.0f, 0.0f};

			// Coordinates given in view space.
			float OcclusionRadius = 0.5f;
			float OcclusionFadeStart = 0.2f;
			float OcclusionFadeEnd = 2.0f;
			float SurfaceEpsilon = 0.05f;
		};

		struct alignas(sizeof(Vector4)) MaterialData
		{
			Vector4  GlobalAmbient;
			Vector4  AmbientColor;
			Vector4  EmissiveColor;
			Vector4  DiffuseColor;
			Vector4  SpecularColor;
			Vector4  Reflectance;
			float   Opacity;
			float   SpecularPower;
			float   IndexOfRefraction;			
			float   BumpIntensity;
			float   SpecularScale;
			float   AlphaThreshold;
			float DiffuseMapIndex = -1;
			float NormalMapIndex = -1;
			float HeightMapIndex = -1;
			float MetallicMapIndex = -1;
			float RounghessMapIndex = -1;
			float AOMapIndex = -1;			
		};

		class StandardForwardShaderSlot
		{
		public:
			enum Register
			{
				ObjectData,
				CameraData,
				MaterialData,
				SkyMap,
				ShadowMap,
				AmbientMap,
				TexturesMap,
				Count
			};
		};
	}
}
