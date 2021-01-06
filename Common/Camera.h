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

			UINT currentFrameResourceIndex = 0;
			
			std::array<std::shared_ptr<ConstantUploadBuffer<CameraConstants>>,globalCountFrameResources> CameraConstantBuffers;


			std::shared_ptr<GRenderTexture> renderTarget = nullptr;
			
			D3D12_VIEWPORT viewport;
			D3D12_RECT rect;

		public:

			inline static Camera* mainCamera = nullptr;

			const D3D12_VIEWPORT GetViewPort() const { return viewport; }
			const D3D12_RECT GetRect() const { return rect; }

			CameraConstants& GetCameraData() { return cameraData; }

			ConstantUploadBuffer<CameraConstants>& GetCameraDataBuffer() const;;

			void SetRenderTarget(std::shared_ptr<GTexture> target, GDescriptor* rtv);

			void SetRenderTarget(std::shared_ptr<GRenderTexture> target);

			GRenderTexture* GetRenderTarget() const;
						
			const Vector3& GetFocusPosition() const;

			Camera(float aspect, const  std::shared_ptr<GRenderTexture> target = nullptr);

			void SetAspectRatio(float aspect);

			void SetFov(float fov);

			float GetFov() const;

			const Matrix& GetViewMatrix() const;

			const Matrix& GetProjectionMatrix() const;
		};
	}
}
