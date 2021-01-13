#include "pch.h"
#include "Camera.h"

#include "d3dApp.h"
#include "GameObject.h"
#include "GDeviceFactory.h"
#include "GPass.h"
#include "GTexture.h"
#include "Transform.h"
#include "Window.h"
#include "GRenderTarger.h"
#include "LightPass.h"
#include "ShadowPass.h"
#include "SSAOPass.h"
#include "imgui.h"
#include "SSAOPass.h"

namespace PEPEngine::Common
{
	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	static const Matrix T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	void Camera::Update()
	{
		currentFrameResourceIndex = (currentFrameResourceIndex + 1) % globalCountFrameResources;

		auto transform = gameObject->GetTransform();

		if (transform->IsDirty())
		{
			NumFramesDirty = globalCountFrameResources;
		}

		if (NumFramesDirty > 0)
		{
			focusPosition = transform->GetForwardVector() + transform->GetWorldPosition();
			view = XMMatrixLookAtLH(transform->GetWorldPosition(), focusPosition, transform->GetUpVector());
			CreateProjection();

			auto viewProj = (view * projection);
			auto invView = view.Invert();
			auto invProj = projection.Invert();
			auto invViewProj = viewProj.Invert();

			Matrix viewProjTex = XMMatrixMultiply(viewProj, T);
			cameraData.View = view.Transpose();
			cameraData.InvView = invView.Transpose();
			cameraData.Proj = projection.Transpose();
			cameraData.InvProj = invProj.Transpose();
			cameraData.ViewProj = viewProj.Transpose();
			cameraData.InvViewProj = invViewProj.Transpose();
			cameraData.ViewProjTex = viewProjTex.Transpose();
			cameraData.EyePosW = transform->GetWorldPosition();
			cameraData.RenderTargetSize = Vector2(static_cast<float>(viewport.Width),
			                                      static_cast<float>(viewport.Height));
			cameraData.InvRenderTargetSize = Vector2(1.0f / cameraData.RenderTargetSize.x,
			                                         1.0f / cameraData.RenderTargetSize.y);


			cameraData.NearZ = nearZ;
			cameraData.FarZ = farZ;

			CameraConstantBuffers[currentFrameResourceIndex]->CopyData(0, cameraData);

			NumFramesDirty--;
		}

		gpass->Update();
		ambiantPass->Update();
		shadowPass->Update();
		lightPass->Update();
	}

	void Camera::CreateProjection()
	{
		const float fovRadians = DirectX::XMConvertToRadians(fov);
		projection = DirectX::XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
	}


	void Camera::Render(std::shared_ptr<GCommandList> cmdList)
	{
		gpass->Render(cmdList);
		ambiantPass->Render(cmdList);
		shadowPass->Render(cmdList);
		lightPass->Render(cmdList);
	}

	const D3D12_VIEWPORT Camera::GetViewPort() const
	{
		return viewport;
	}

	const D3D12_RECT Camera::GetRect() const
	{
		return rect;
	}

	CameraConstants& Camera::GetCameraData()
	{
		return cameraData;
	}

	ConstantUploadBuffer<CameraConstants>& Camera::GetCameraDataBuffer() const
	{
		return *CameraConstantBuffers[currentFrameResourceIndex].get();
	}

	void Camera::SetRenderTarget(std::shared_ptr<GTexture> target, GDescriptor* rtv)
	{
		renderTarget = std::make_shared<GRenderTexture>(target, rtv);

		SetRenderTarget(renderTarget);
	}

	void Camera::SetRenderTarget(std::shared_ptr<GRenderTexture> target)
	{
		renderTarget = target;

		NumFramesDirty = globalCountFrameResources;

		if (renderTarget != nullptr)
		{
			viewport.Height = static_cast<float>(renderTarget->GetRenderTexture()->GetD3D12ResourceDesc().Height);
			viewport.Width = static_cast<float>(renderTarget->GetRenderTexture()->GetD3D12ResourceDesc().Width);
		}
		else
		{
			const auto* const window = D3DApp::GetApp().GetMainWindow();
			viewport.Height = static_cast<float>(window->GetClientHeight());
			viewport.Width = static_cast<float>(window->GetClientWidth());
		}

		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;

		rect = {0, 0, static_cast<LONG>(viewport.Width), static_cast<LONG>(viewport.Height)};
	}

	GRenderTexture* Camera::GetRenderTarget() const
	{
		return renderTarget.get();
	}


	const Vector3& Camera::GetFocusPosition() const
	{
		return focusPosition;
	}

	void Camera::InitializeCamera(const std::shared_ptr<GRenderTexture> target)
	{
		const auto MainWindow = D3DApp::GetApp().GetMainWindow();

		gpass = std::make_shared<GPass>(MainWindow->GetClientWidth(), MainWindow->GetClientHeight());
		ambiantPass = std::make_shared<SSAOPass>(1920, 1080, *gpass.get());
		shadowPass = std::make_shared<ShadowPass>(2048, 2048);
		lightPass = std::make_shared<LightPass>(MainWindow->GetClientWidth(), MainWindow->GetClientHeight(),
		                                        *gpass.get(), *ambiantPass.get(), *shadowPass.get());

		mainCamera = this;


		for (int i = 0; i < globalCountFrameResources; ++i)
		{
			CameraConstantBuffers[i] = std::make_shared<ConstantUploadBuffer<CameraConstants>>(
				GDeviceFactory::GetDevice(), 1, L"Camera Data Buffer" + std::to_wstring(i));
		}


		SetRenderTarget(target);
	}

	Camera::Camera(float aspect, const std::shared_ptr<GRenderTexture> target) : Component(), aspectRatio(aspect)
	{
		InitializeCamera(target);
	}

	void Camera::ChangeRenderSize(UINT height, UINT width) const
	{
		if (gpass)
			gpass->ChangeRenderTargetSize(width, height);
		if (lightPass)
			lightPass->ChangeRenderTargetSize(width, height);
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

	void Camera::SetShadowTransform(const Matrix& shadowTransformMatrix)
	{
		cameraData.ShadowTransform = shadowTransformMatrix;
	}

	void Camera::OnGUI()
	{
#if defined(DEBUG) || defined(_DEBUG)

		// Exceptionally add an extra assert here for people confused about initial Dear ImGui setup
		// Most ImGui functions would normally just crash if the context is missing.
		IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");

		// We specify a default position/size in case there's no data in the .ini file.
		// We only do it to make the demo applications a little more welcoming, but typically this isn't required.
		ImGuiViewport* main_viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(main_viewport->GetWorkPos().x + main_viewport->GetWorkSize().x - 500, 0),
		                        ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(550, main_viewport->GetWorkSize().y), ImGuiCond_FirstUseEver);

		// Main body of the Demo window starts here.
		if (!ImGui::Begin("Render Maps"))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}
		ImGui::BeginGroup();
		{
			(ImGui::Text("Shadow"));
			{
				auto shadowMap = shadowPass->GetSrvMemory()->GetGPUHandle();

				ImGui::Image((ImTextureID)shadowMap.ptr, ImVec2(500, 500));
			}
			(ImGui::Text("Position"));
			{
				auto shadowMap = gpass->GetSRV()->GetGPUHandle(GPass::PositionMap);

				ImGui::Image((ImTextureID)shadowMap.ptr, ImVec2(500, 500));
			}
			(ImGui::Text("Normal"));
			{
				auto shadowMap = gpass->GetSRV()->GetGPUHandle(GPass::NormalMap);

				ImGui::Image((ImTextureID)shadowMap.ptr, ImVec2(500, 500));
			}		
			(ImGui::Text("Ambient"));
			{
				auto shadowMap = ambiantPass->AmbientMapSrv()->GetGPUHandle();

				ImGui::Image((ImTextureID)shadowMap.ptr, ImVec2(500, 500));
			}
		}
		ImGui::EndGroup();


		ImGui::End();
#endif
	}
}
