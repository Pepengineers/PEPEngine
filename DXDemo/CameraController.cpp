#include "CameraController.h"
#include "SampleApp.h"
#include "GameObject.h"

CameraController::CameraController()
{
	DXLib::SampleApp& app = static_cast<DXLib::SampleApp&>(DXLib::SampleApp::GetApp());
	keyboard = app.GetKeyboard();
	mouse = app.GetMouse();
	timer = app.GetTimer();
}

void CameraController::Update()
{
	while (!keyboard->CharBufferIsEmpty())
	{
		unsigned char ch = keyboard->ReadChar();
	}

	while (!keyboard->KeyBufferIsEmpty())
	{
		KeyboardEvent kbe = keyboard->ReadKey();
		unsigned char keycode = kbe.GetKeyCode();

		if (keycode == (VK_F2) && keyboard->KeyIsPressed(VK_F2))
		{
			DXLib::SampleApp& app = static_cast<DXLib::SampleApp&>(DXLib::SampleApp::GetApp());
			app.pathMapShow = (app.pathMapShow + 1) % app.maxPathMap;
		}
	}

	auto curSpeed = cameraSpeed;
	float dt = timer->DeltaTime();

	auto tr = gameObject->GetTransform();

	if (keyboard->KeyIsPressed(VK_SHIFT))
	{
		curSpeed *= 10;
	}

	while (!mouse->EventBufferIsEmpty())
	{
		MouseEvent me = mouse->ReadEvent();
		if (mouse->IsRightDown())
		{
			if (me.GetType() == MouseEvent::EventType::RAW_MOVE)
			{
				gameObject->GetTransform()->AdjustEulerRotation(
					-1 * (me.GetPosY()) * dt * yMouseSpeed,
					-1* (me.GetPosX()) * dt * xMouseSpeed, 0);
			}
		}
	}


	if (keyboard->KeyIsPressed('W'))
	{
		tr->AdjustPosition(tr->GetForwardVector() * curSpeed * dt);
	}
	if (keyboard->KeyIsPressed('S'))
	{
		tr->AdjustPosition(tr->GetBackwardVector() * curSpeed * dt);
	}
	if (keyboard->KeyIsPressed('A'))
	{
		tr->AdjustPosition(tr->GetLeftVector() * curSpeed * dt);
	}
	if (keyboard->KeyIsPressed('D'))
	{
		tr->AdjustPosition(tr->GetRightVector() * curSpeed * dt);
	}
	if (keyboard->KeyIsPressed(VK_SPACE))
	{
		tr->AdjustPosition(tr->GetUpVector() * curSpeed * dt);
	}
	if (keyboard->KeyIsPressed('Z'))
	{
		tr->AdjustPosition(tr->GetDownVector() * curSpeed * dt);
	}

	if (keyboard->KeyIsPressed(VK_CONTROL))
	{
		auto eulerAngels = tr->GetEulerAngels();
		auto worldPosition = tr->GetWorldPosition();

		std::wstring str = L"Position:" + std::to_wstring(worldPosition.x) + L" " + std::to_wstring(worldPosition.y) + L" " + std::to_wstring(worldPosition.z) + L"\n";
		str += L"Rotation:" + std::to_wstring(eulerAngels.x) + L" " + std::to_wstring(eulerAngels.y) + L" " + std::to_wstring(eulerAngels.z) + L"\n";
		OutputDebugString(str.c_str());
	}
	
}

void CameraController::Draw(std::shared_ptr<GCommandList> cmdList)
{
}
