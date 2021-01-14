#include "Snake.h"

#include "AIComponent.h"
#include "CameraController.h"
#include "GameObject.h"
#include "GDeviceFactory.h"
#include "GModel.h"
#include "GPass.h"
#include "GraphicPSO.h"
#include "Light.h"
#include "LightPass.h"
#include "MathHelper.h"
#include "ModelRenderer.h"
#include "ShadowPass.h"
#include "SSAOPass.h"
#include "Transform.h"
#include "Window.h"
#include "AssetDatabase.h"
#include "AScene.h"
#include "ParticleEmitter.h"

namespace Snake
{

	SnakeApp::PlayerController::PlayerController(){
		auto& app = static_cast<D3DApp&>(D3DApp::GetApp());
		keyboard = app.GetKeyboard();
		timer = app.GetTimer();
	}

	void SnakeApp::PlayerController::Update(){

		if(!keyboard->KeyBufferIsEmpty()){
			auto event = keyboard->ReadKey();

			switch (event.GetKeyCode()){
			case 'W':{
				if (direction != down) {
					direction = up;
				}
				break;
			}
			case 'S':
				if (direction != up) {
					direction = down;
				}
				break;
			case 'A':
				if (direction != right) {
					direction = left;
				}
				break;
			case 'D':
				if (direction != left) {
					direction = right;
				}
				break;
			default:
				break;
			}
		}

		auto& app = Snake::SnakeApp::Instance();

		auto level = app.level;

		gameObject->GetTransform()->AdjustPosition(direction * timer->DeltaTime() * speed);

		auto position = gameObject->GetTransform()->GetWorldPosition();
		lastPosition.x = position.x;
		lastPosition.y = position.y;
		lastPosition.z = position.z;

		auto food = app.food;

		if (food) {

			auto foodBounds = food->GetComponent<ModelRenderer>()->GetModel()->GetGModel()->Bounds;
			auto snakeHeadBounds = gameObject->GetComponent<ModelRenderer>()->GetModel()->GetGModel()->Bounds;

			foodBounds.Center = DirectX::XMFLOAT3{ food->GetTransform()->GetWorldPosition() };
			snakeHeadBounds.Center = DirectX::XMFLOAT3{ gameObject->GetTransform()->GetWorldPosition() };

			if (foodBounds.Intersects(snakeHeadBounds)) {
				level->GetScene()->RemoveGameObject(app.food);
				app.IncreaseTail();
				app.isFoodSpawned = false;
				speed *= 1.5f;
			}
		}

		//if (app.food) {
		//	auto foodPosition = app.food->GetTransform()->GetWorldPosition();

		//	if (std::abs(position.x - foodPosition.x) < 3.0f && std::abs(position.z == foodPosition.z) < 3.0f) {
		//		level->GetScene()->RemoveGameObject(app.food);
		//		app.isFoodSpawned = false;
		//		speed *= 1.5f;
		//	}
		//}

		if(position.x >= 200.0f || position.x <= -200.0f || position.z >= 200.0f || position.z <= -200.0f){
			
			std::shared_ptr<GameObject> p = nullptr;

			for(auto& sharedObject : level->GetScene()->objects){
				if (sharedObject->GetName() == gameObject->GetName()) {
					p = sharedObject;
				}
			}

			level->GetScene()->RemoveGameObject(p);
		}
	}

	SnakeApp::SnakeApp(HINSTANCE hInstance) : D3DApp(hInstance)  {

	}                         


	bool SnakeApp::Initialize() {
		device = GDeviceFactory::GetDevice();		
		
		if (!D3DApp::Initialize())
		{
			return false;
		}

		MainWindow->SetVSync(true);		
		
		//uiPass = std::make_shared<UILayer>(MainWindow->GetClientWidth(), MainWindow->GetClientHeight(), MainWindow->GetWindowHandle());

		
		auto atlas = AssetDatabase::Get<AModel>(L"Atlas");
		if(atlas == nullptr)
		{
			atlas = AssetDatabase::AddModel("Data\\Objects\\Atlas\\Atlas.obj");
		}
		pbody = AssetDatabase::Get<AModel>(L"P-Body");
		if (pbody == nullptr)
		{
			pbody = AssetDatabase::AddModel("Data\\Objects\\P-Body\\P-Body.obj");
		}

		auto dragon = AssetDatabase::Get<AModel>(L"DesertDragon");
		if (!dragon) {
			dragon = AssetDatabase::AddModel("Data\\Objects\\DesertDragon\\DesertDragon.fbx");
		}
		

		level = AssetDatabase::Get<AScene>(L"SnakeScene");

		if(level == nullptr)
		{
			level = AssetDatabase::CreateAsset<AScene>(L"SnakeScene");

			auto scene = level->GetScene();


			auto cameraGO = std::make_unique<GameObject>("MainCamera");
			cameraGO->GetTransform()->SetEulerRotate(Vector3(-58, 182, 0));
			cameraGO->GetTransform()->SetPosition(Vector3(-48, 434, -326));

			cameraGO->AddComponent(std::make_shared<Camera>(AspectRatio()));
	//		cameraGO->AddComponent(std::make_shared<CameraController>());

			scene->AddGameObject(std::move(cameraGO));

			auto sun = std::make_unique<GameObject>("Directional Light");
			auto light = std::make_shared<Light>();
			light->Type = LightType::Directional;
			light->Direction = Vector3(0.0f, 0.0f, 0.0f);
			light->Intensity = 0.1f;
			sun->AddComponent(light);
			scene->AddGameObject(std::move(sun));

			for(int32_t i = -20; i < 20; ++i){
					auto rModel = std::make_unique<GameObject>("Boundary " + std::to_string(i));
					auto renderer = std::make_shared<ModelRenderer>(dragon);
					rModel->AddComponent(renderer);
					rModel->GetTransform()->SetPosition(Vector3(i * 10.0f, 0.0f, -190.0f));
					rModel->GetTransform()->SetScale(Vector3::One * 0.01f);

					scene->AddGameObject(std::move(rModel));
				}

			for(int32_t i = -20; i < 20; ++i){
				auto rModel = std::make_unique<GameObject>("Boundary " + std::to_string(40 + i));
				auto renderer = std::make_shared<ModelRenderer>(dragon);
				rModel->AddComponent(renderer);
				rModel->GetTransform()->SetPosition(Vector3(-190.0f, 0.0f, i * 10.0f));
				rModel->GetTransform()->SetScale(Vector3::One * 0.01f);

				scene->AddGameObject(std::move(rModel));
			}

			for(int32_t i = -20; i < 20; ++i){
				auto rModel = std::make_unique<GameObject>("Boundary " + std::to_string(80 + i));
				auto renderer = std::make_shared<ModelRenderer>(dragon);
				rModel->AddComponent(renderer);
				rModel->GetTransform()->SetPosition(Vector3(i * 10.0f, 0.0f, 190.0f));
				rModel->GetTransform()->SetScale(Vector3::One * 0.01f);

				scene->AddGameObject(std::move(rModel));
			}

			for (int32_t i = -20; i < 20; ++i) {
				auto rModel = std::make_unique<GameObject>("Boundary " + std::to_string(80 + i));
				auto renderer = std::make_shared<ModelRenderer>(dragon);
				rModel->AddComponent(renderer);
				rModel->GetTransform()->SetPosition(Vector3(190.0f, 0.0f, i * 10.0f));
				rModel->GetTransform()->SetScale(Vector3::One * 0.01f);

				scene->AddGameObject(std::move(rModel));
			}


			//for (int i = 0; i < 12; ++i)
			//{
			//	auto particleEmitter = std::make_unique<GameObject>("Particle " + std::to_string(i));
			//	particleEmitter->AddComponent(std::make_shared<ParticleEmitter>(1500));
			//	particleEmitter->GetTransform()->SetPosition(
			//		Vector3::Right * -30  + Vector3::Forward * 10 * i);
			//	scene->AddGameObject(std::move(particleEmitter));
			//	for (int j = 0; j < 3; ++j)
			//	{
			//		auto rModel = std::make_unique<GameObject>("Atlas" + std::to_string(i+j));
			//		auto renderer = std::make_shared<ModelRenderer>(atlas);
			//		rModel->AddComponent(renderer);
			//		rModel->GetTransform()->SetPosition(
			//			Vector3::Right * -30 * j + Vector3::Forward * 10 * i);

			//		/*	auto ai = std::make_shared<AIComponent>();
			//			rModel->AddComponent(ai);*/


			//		auto pos = rModel->GetTransform()->GetWorldPosition() + (Vector3::Up * 1 * 10);

			//		auto sun1 = std::make_unique<GameObject>("Light");
			//		sun1->GetTransform()->SetPosition(pos);
			//		auto light = std::make_shared<Light>();
			//		light->Color = Vector4(MathHelper::RandF(), MathHelper::RandF(), MathHelper::RandF(), 1);
			//		light->Intensity = 1;
			//		(i + j) % 2 == 1 ? light->Type = Spot : light->Type = Point;

			//		sun1->AddComponent(light);

			//		scene->AddGameObject(std::move(sun1));

			//		scene->AddGameObject(std::move(rModel));
			//	}
			//}
			
			scene->Prepare();
			scene->Update();
			
			AssetDatabase::UpdateAsset(level);
		}
		else
		{		
			
			auto scene = level->GetScene();
			scene->Prepare();
			scene->Update();
		}

		snakeHead = std::make_unique<GameObject>("SnakeHead");

		auto rModel = std::make_shared<ModelRenderer>(atlas);

		auto pc = std::make_shared<PlayerController>();

		snakeHead->AddComponent(rModel);
		snakeHead->AddComponent(pc);

		snakeHead->GetTransform()->SetPosition(Vector3(0.0f, 0.0f, 0.0f));

		level->GetScene()->AddGameObject(snakeHead);
	}


	void SnakeApp::Update(const GameTimer& gt)
	{
		auto renderQueue = device->GetCommandQueue();

		auto values = renderFenceValues[currentFrameResourceIndex];

		if (values != 0 && !renderQueue->IsFinish(values))
		{
			renderQueue->WaitForFenceValue(values);
		}

		level->GetScene()->Update();
		

		if(isFoodSpawned == false){
			SpawnFood();
			isFoodSpawned = true;
		}



		static bool spawned = false;

		if(keyboard.KeyIsPressed('P'))
		{
			if(!spawned)
			{
				auto rModel = std::make_shared<GameObject>();
				auto renderer = std::make_shared<ModelRenderer>(pbody);
				rModel->AddComponent(renderer);
				rModel->GetTransform()->SetPosition(Camera::mainCamera->gameObject->GetTransform()->GetWorldPosition());

				level->GetScene()->AddGameObject(std::move(rModel));

				spawned = true;
			}
		}
		else
		{
			spawned = false;
		}

		static bool sceneSaved = false;
		if (keyboard.KeyIsPressed('I'))
		{
			if(!sceneSaved)
			{
				sceneSaved = true;
				AssetDatabase::UpdateAsset(level);
			}
		}
		else sceneSaved = false;

	}

	void SnakeApp::Draw(const GameTimer& gt)
	{
		if (isResizing) return;

		auto renderQueue = device->GetCommandQueue();
		
		auto computeQueue = device->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		auto cmdList = computeQueue->GetCommandList();

		level->GetScene()->Dispatch(cmdList);

		computeQueue->Wait(renderQueue);
		
		computeFenceValues[currentFrameResourceIndex] = computeQueue->ExecuteCommandList(cmdList);
		

		
		
		cmdList = renderQueue->GetCommandList();

		
		level->GetScene()->Render(cmdList);

		if (uiPass) {
			uiPass->Render(cmdList);
		}

		cmdList->TransitionBarrier(MainWindow->GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
		cmdList->FlushResourceBarriers();

		renderQueue->Wait(computeQueue);
		renderFenceValues[currentFrameResourceIndex] = renderQueue->ExecuteCommandList(cmdList);

		currentFrameResourceIndex = MainWindow->Present();
	}

	void SnakeApp::OnResize()
	{
		D3DApp::OnResize();

		currentFrameResourceIndex = MainWnd()->GetCurrentBackBufferIndex();

		if (Camera::mainCamera)
		{
			Camera::mainCamera->SetAspectRatio(AspectRatio());

			Camera::mainCamera->ChangeRenderSize(MainWindow->GetClientHeight(), MainWindow->GetClientWidth());
		}

		
		if (uiPass)
			uiPass->ChangeRenderTargetSize(MainWindow->GetClientWidth(), MainWindow->GetClientHeight());
		
	}

	LRESULT SnakeApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (uiPass)
			uiPass->MsgProc(hwnd, msg, wParam, lParam);		
		
		return D3DApp::MsgProc(hwnd, msg, wParam, lParam);
	}

	void SnakeApp::SpawnFood(){

		//auto x = -185.0f + static_cast<float>(rand(GetApp()->timer.DeltaTime())) / static_cast<float>(RAND_MAX) * (2 * 185.0f);
		//auto y = 0.0f;
		//auto z = -185.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (2 * 185.0f);

		auto x = MathHelper::RandF(-185.0f, 185.0f);
		auto y = 0.0f;
		auto z = MathHelper::RandF(-185.0f, 185.0f);

		food = std::make_shared<GameObject>();
		food->GetTransform()->SetPosition(Vector3(x, y, z));
		food->AddComponent(std::make_shared<ModelRenderer>(pbody));

		Scene::currentScene->AddGameObject(food);

	}

	void SnakeApp::IncreaseTail()
	{

		auto tailPart = std::make_shared<GameObject>();

		tailPart->AddComponent(std::make_shared<ModelRenderer>(pbody));


		if (snakeTail.size() == 0) {
			tailPart->GetTransform()->SetPosition(snakeHead->GetTransform()->GetBackwardVector() * 5.0f);
			tailPart->GetTransform()->SetParent(snakeHead->GetTransform().get());
		}
		else{
			tailPart->GetTransform()->SetPosition(snakeHead->GetTransform()->GetBackwardVector() * 5.0f);
			tailPart->GetTransform()->SetParent(snakeTail[snakeTail.size() - 1]->GetTransform().get());
		}
		snakeTail.push_back(tailPart);
		/*snakeTailPartsPositions.push_back(Vector3(0.0f, 0.0f, 0.0f));

		for(uint32_t i = 1; i < snakeTailPartsPositions.size(); ++i){
			snakeTail[i]->GetTransform()->SetPosition(snakeTailPartsPositions[i - 1]);
		}

		snakeTailPartsPositions[0] = snakeHead->GetComponent<PlayerController>()->lastPosition;
		snakeTail[0]->GetTransform()->SetPosition(snakeTailPartsPositions[0]);*/


		level->GetScene()->AddGameObject(tailPart);
	}

}
