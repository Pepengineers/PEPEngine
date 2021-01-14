#pragma once
#include "AssetDatabase.h"
#include "d3dApp.h"
#include "GPass.h"
#include "LightPass.h"
#include "RenderPass.h"
#include "Scene.h"
#include "SSAOPass.h"
#include "UILayer.h"

namespace Snake
{
	using namespace PEPEngine;
	using namespace Common;
	using namespace Utils;
	using namespace Graphics;
	using namespace Allocator;

	class SnakeApp : public D3DApp {
	public:
		class PlayerController : public Component{
		public:
			PlayerController();
		public:
			void Update();
		public:
			Vector3 up{ 0.0f, 0.0f, 1.0f };
			Vector3 down = -up;
			Vector3 right{ 1.0f, 0.0f, 0.0f };
			Vector3 left = -right;
		public:
			Vector3 direction = { 1.0f, 0.0f, 0.0f };
			Vector3 lastPosition;
		private:
			KeyboardDevice* keyboard;
			GameTimer* timer;
			float speed = 20.0f;

		};
		class FoodComponent : public Component{

		};
	private:
		SnakeApp(HINSTANCE hInstance);
	public:
		static inline SnakeApp& Instance(HINSTANCE* pHinstance = nullptr) {
			static SnakeApp instance(*pHinstance);
			return instance;
		}
	public:
		bool Initialize() override;

	protected:
		void Pick(const MousePoint& mouse_point);
		void Update(const GameTimer& gt) override;
		void Draw(const GameTimer& gt) override;

	protected:
		void OnResize() override;
		LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

	private:
		void SpawnFood();
		void IncreaseTail();
	public:
		std::shared_ptr<GameObject> snakeHead = nullptr;
		std::shared_ptr<GameObject> food = nullptr;
		std::vector<std::shared_ptr<GameObject>> snakeTail;
		std::vector<Vector3> snakeTailPartsPositions;

	public:
		bool isFoodSpawned = false;
	private:

		static SnakeApp* pInstance;

		std::shared_ptr<GDevice> device;
		AssetDatabase assetLoader;

		UINT currentFrameResourceIndex = 0;

		UINT64 renderFenceValues[globalCountFrameResources];
		UINT64 computeFenceValues[globalCountFrameResources];

		std::shared_ptr<AScene> level = nullptr;
		std::shared_ptr<AModel> pbody = nullptr;

		
		std::vector<Camera*> cameras;

		std::shared_ptr<UILayer> uiPass;
	};
}
