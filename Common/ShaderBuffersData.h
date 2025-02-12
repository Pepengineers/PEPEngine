#pragma once
#include <d3d12.h>
#include <SimpleMath.h>

#define MaxLights 16

namespace PEPEngine
{
	namespace Common
	{
		using namespace DirectX::SimpleMath;



		static const DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		static const DXGI_FORMAT DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		static const DXGI_FORMAT AmbientMapFormat = DXGI_FORMAT_R16_UNORM;


		enum DeferredPassSlots : UINT
		{
			ObjectWorldDataBuffer = 0,
			MaterialTextures,
			CameraDataBuffer,
			WorldDataBuffer,
			MaterialsDataBuffer,
			LightDataBuffer,
			NormalMap,
			BaseColorMap,
			PositionMap,
			AmbientMap,
			ShadowMap,
			SkyMap
		};


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

		enum LightType : INT
		{
			Point = 0,
			Spot,
			Directional,
		};

		struct alignas(sizeof(Vector4)) LightData
		{
			Vector4 Color;
			Vector3 PositionWorld;
			Vector3 DirectionWorld;
			Vector3 PositionView;
			Vector3 DirectionView;
			float SpotlightAngle;
			float Range;
			float Intensity;
			LightType Type;
			bool Enabled;
			bool Selected;
			Vector2 Padding;
		};

		struct CameraConstants
		{
			Matrix View = Matrix::Identity;
			Matrix InvView = Matrix::Identity;
			Matrix Proj = Matrix::Identity;
			Matrix InvProj = Matrix::Identity;
			Matrix ViewProj = Matrix::Identity;
			Matrix InvViewProj = Matrix::Identity;
			Matrix ViewProjTex = Matrix::Identity;
			Matrix ShadowTransform = Matrix::Identity;

			Vector2 RenderTargetSize = Vector2{0.0f, 0.0f};
			Vector2 InvRenderTargetSize = Vector2{0.0f, 0.0f};

			Vector3 EyePosW = Vector3{0.0f, 0.0f, 0.0f};
			float padding;

			float NearZ = 0.0f;
			float FarZ = 0.0f;
		};

		struct WorldData
		{
			UINT LightsCount;
			float TotalTime;
			float DeltaTime;
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
			Vector4 DiffuseColor;		
			float SpecularPower = 1.0f;
			float AlphaThreshold = 0.1f;
			int DiffuseMapIndex = -1;
			int NormalMapIndex = -1;
		};


		inline static D3D12_INPUT_ELEMENT_DESC VertexInputLayout[] =
		{
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
			{
				"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
			{
				"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			},
		};


		struct alignas(sizeof(Vector4)) ParticleData
		{
			Vector3 Position;
			float LiveTime;

			Vector3 Velocity;
			float TotalLifeTime;

			DWORD TextureIndex;
			DWORD PD2;
			DWORD PD3;
			DWORD PD4;
		};

		struct alignas(sizeof(Vector4)) EmitterData
		{
			Vector4 Color;

			Vector3 Force;
			float DeltaTime;

			Vector2 Size;
			DWORD ParticlesTotalCount;
			DWORD SimulatedGroupCount;

			DWORD ParticleInjectCount;
			DWORD InjectedGroupCount;
			DWORD ParticlesAliveCount;
			DWORD AtlasTextureCount;

			bool UseTexture = false;
			bool Padding;
			bool Padding1;
			bool Padding2;
			Vector3 Padding3;
		};
		class ParticleRenderSlot
		{
		public:
			enum Slots
			{
				ObjectData,
				CameraData,
				WorldBuffer,
				EmitterData,
				ParticlesPool,
				ParticlesAliveIndex,
				Atlas,
				Count
			};
		};

		class ParticleComputeSlot
		{
		public:
			enum Slots
			{
				EmitterData,
				ParticlesPool,
				ParticleDead,
				ParticleAlive,
				ParticleInjection,
				Count
			};
		};

	}
}
