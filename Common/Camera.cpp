#include "pch.h"
#include "Camera.h"
#include "GameObject.h"
#include "Transform.h"

namespace PEPEngine
{
	namespace Common
	{
		void Camera::Update()
		{
			auto transform = gameObject->GetTransform();

			focusPosition = transform->GetForwardVector() + transform->GetWorldPosition();

			view = XMMatrixLookAtLH(transform->GetWorldPosition(), focusPosition, transform->GetUpVector());

			if (NumFramesDirty > 0)
			{
				CreateProjection();
				NumFramesDirty--;
			}
		}

		void Camera::CreateProjection()
		{
			const float fovRadians = DirectX::XMConvertToRadians(fov);
			projection = DirectX::XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
		}

		const Vector3& Camera::GetFocusPosition() const
		{
			return focusPosition;
		}

		Camera::Camera(float aspect) : aspectRatio(aspect)
		{
		}

		void Camera::SetAspectRatio(float aspect)
		{
			aspectRatio = aspect;
			NumFramesDirty = globalCountFrameResources;
		}

		void Camera::SetFov(float fov)
		{
			this->fov = fov;
			NumFramesDirty = globalCountFrameResources;
		}

		float Camera::GetFov() const
		{
			return fov;
		}

		const Matrix& Camera::GetViewMatrix() const
		{
			return this->view;
		}

		const Matrix& Camera::GetProjectionMatrix() const
		{
			return this->projection;
		}
	}
}
