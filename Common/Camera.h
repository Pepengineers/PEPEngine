#pragma once
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

		class Camera : public Component
		{
			void PopulateDrawCommand(std::shared_ptr<GCommandList> cmdList) override
			{
			};
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

			std::shared_ptr<ConstantUploadBuffer<CameraConstants>> CameraConstantBuffer = nullptr;


			const GTexture* renderTarget = nullptr;
			GDescriptor* rtvDescriptor = nullptr;
			
			D3D12_VIEWPORT viewport;
			D3D12_RECT rect;

		public:

			inline static Camera* mainCamera = nullptr;

			const D3D12_VIEWPORT GetViewPort() const { return viewport; }
			const D3D12_RECT GetRect() const { return rect; }

			CameraConstants& GetCameraData() { return cameraData; }

			ConstantUploadBuffer<CameraConstants>& GetCameraDataBuffer() const;;

			void SetRenderTarget(const GTexture* target, GDescriptor* rtv);

			const GTexture* GetRenderTarget() const;

			GDescriptor* GetRTV() const;;
			
			const Vector3& GetFocusPosition() const;

			Camera(float aspect,const GTexture* target = nullptr, GDescriptor* rtv = nullptr);

			void SetAspectRatio(float aspect);

			void SetFov(float fov);

			float GetFov() const;

			const Matrix& GetViewMatrix() const;

			const Matrix& GetProjectionMatrix() const;
		};
	}
}
