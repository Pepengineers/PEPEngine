#pragma once
#include "Component.h"

class GCommandList;
class GameTimer;
class Mouse;
class Keyboard;

using namespace DirectX::SimpleMath;

class CameraController :
	public Component
{
	Keyboard* keyboard;
	Mouse* mouse;
	GameTimer* timer;


	

public:
	float cameraSpeed = 6.0f;
	float xMouseSpeed = 50;
	float yMouseSpeed = 35;
	
	CameraController();

	void Update() override;;
	void Draw(std::shared_ptr<GCommandList> cmdList) override;;
};
