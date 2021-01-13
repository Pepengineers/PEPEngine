#pragma once
#include <array>

#include "Component.h"
#include "d3dUtil.h"
#include "DirectXBuffers.h"
#include "ShaderBuffersData.h"
#include "SimpleMath.h"

namespace PEPEngine
{
	namespace Common
	{
		using namespace DirectX::SimpleMath;

		class GPass;
		class LightPass;
		class SSAOPass;
		class ShadowPass;

		using namespace Graphics;
		using namespace Utils;
		
		class Camera : public Component
		{
			
			
			void Update() override;

			void CreateProjection();

			Matrix view = Matrix::Identity;
			Matrix projection = Matrix::Identity;

			float fov = 60;
			float aspectRatio = 0;
			float nearZ = 0.1;
			float farZ = 10000;

			Vector3 focusPosition = Vector3::Zero;

			int NumFramesDirty = globalCountFrameResources;

			CameraConstants cameraData;

			UINT currentFrameResourceIndex = 0;

			std::array<std::shared_ptr<ConstantUploadBuffer<CameraConstants>>, globalCountFrameResources>
			CameraConstantBuffers;


			std::shared_ptr<GRenderTexture> renderTarget = nullptr;

			D3D12_VIEWPORT viewport;
			D3D12_RECT rect;

			std::shared_ptr<GPass> gpass;
			std::shared_ptr<LightPass> lightPass;
			std::shared_ptr<SSAOPass> ambiantPass;
			std::shared_ptr<ShadowPass> shadowPass;


			void Serialize(json& j) override
			{
				j["Type"] = ComponentID;

				auto jPos = json();
				jPos["aspect"] = aspectRatio;
				jPos["nearZ"] = nearZ;
				jPos["farZ"] = farZ;
				jPos["fov"] = fov;
				
				j["CameraData"] = jPos;
			};

			void Deserialize(json& j) override
			{
				auto jPos = j["CameraData"];
				(TryReadVariable<float>(jPos, "aspect", &aspectRatio));
				(TryReadVariable<float>(jPos, "nearZ", &nearZ));
				(TryReadVariable<float>(jPos, "farZ", &farZ));
				(TryReadVariable<float>(jPos, "fov", &fov));

				InitializeCamera(nullptr);
			};
			
		public:

			SERIALIZE_FROM_JSON(Camera)
			
			void Render(std::shared_ptr<GCommandList> cmdList);

			inline static Camera* mainCamera = nullptr;

			const D3D12_VIEWPORT GetViewPort() const;
			const D3D12_RECT GetRect() const;

			CameraConstants& GetCameraData();

			ConstantUploadBuffer<CameraConstants>& GetCameraDataBuffer() const;;

			void SetRenderTarget(std::shared_ptr<GTexture> target, GDescriptor* rtv);

			void SetRenderTarget(std::shared_ptr<GRenderTexture> target);

			GRenderTexture* GetRenderTarget() const;

			const Vector3& GetFocusPosition() const;
			void InitializeCamera(std::shared_ptr<GRenderTexture> target);

			Camera(float aspect, std::shared_ptr<GRenderTexture> target = nullptr);

			void ChangeRenderSize(UINT height, UINT width) const;

			void SetAspectRatio(float aspect);

			void SetFov(float fov);

			float GetFov() const;

			const Matrix& GetViewMatrix() const;

			const Matrix& GetProjectionMatrix() const;

			void SetShadowTransform(const Matrix& shadowTransformMatrix);
			void OnGUI() override;
		};
	}
}
