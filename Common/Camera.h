#pragma once
#include "Component.h"
#include "d3dUtil.h"
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


			int NumFramesDirty = Utils::globalCountFrameResources;
		public:

			const Vector3& GetFocusPosition() const;

			Camera(float aspect);;

			void SetAspectRatio(float aspect);

			void SetFov(float fov);

			float GetFov() const;

			const Matrix& GetViewMatrix() const;

			const Matrix& GetProjectionMatrix() const;
		};
	}
}
