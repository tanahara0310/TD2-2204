#include "RecommendedScene.h"
#include "EngineSystem/EngineSystem.h"
#include "Scene/SceneManager.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "Engine/Graphics/Model/ModelManager.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Input/KeyboardInput.h"
#include "Engine/Input/GamepadInput.h"
#include <dinput.h>

void RecommendedScene::Initialize(EngineSystem* engine) {
	BaseScene::Initialize(engine);

	auto modelManager = engine_->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();
	
	// 黒い背景ボクセルの作成
	{
		// Voxelモデルリソースを取得または読み込み
		auto* voxelModelResource = modelManager->GetModelResource("Resources/Models/Voxel/Voxel.obj");
		if (!voxelModelResource) {
			modelManager->LoadModelResource("Resources/Models/Voxel", "Voxel.obj");
			voxelModelResource = modelManager->GetModelResource("Resources/Models/Voxel/Voxel.obj");
		}
		
		auto texture = textureManager.Load("Resources/SampleResources/white1x1.png");
		
		auto voxel = std::make_unique<Voxel>();
		voxel->Initialize(voxelModelResource, texture);
		
		// 画面全体を覆うように大きくスケール
		voxel->GetTransform().translate = { 0.0f, 0.0f, 50.0f }; // カメラから遠くに配置
		voxel->GetTransform().scale = { 1000.0f, 1000.0f, 1.0f }; // 大きくスケール
		voxel->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f }); // 黒色
		voxel->GetTransform().TransferMatrix();
		
		gameObjects_.push_back(std::move(voxel));
	}
	
	// コントローラーモデルの作成
	{
		auto model = modelManager->CreateStaticModel("Resources/Models/Controller/Controller.obj");
		auto texture = textureManager.Load("Resources/Models/Controller/Controller.png");
		
		auto controllerModel = std::make_unique<ControllerModel>();
		controllerModel_ = controllerModel.get();
		controllerModel->Initialize(std::move(model), texture);
		controllerModel_->GetTransform().translate = { 0.0f, -3.0f, -4.0f };
		
		gameObjects_.push_back(std::move(controllerModel));
	}
	
	// Recommendedモデルの作成
	{
		auto model = modelManager->CreateStaticModel("Resources/Models/Recommended/Recommended.obj");
		auto texture = textureManager.Load("Resources/SampleResources/white1x1.png");
		
		auto recommendedModel = std::make_unique<RecommendedModel>();
		auto* modelPtr = recommendedModel.get();
		recommendedModel->Initialize(std::move(model), texture, { 0.0f, 1.5f, 0.7f });
		
		// 出現アニメーションを開始（遅延なし）
		recommendedModel->StartAppearAnimation(0.0f);
		
		textModels_.push_back(modelPtr);
		gameObjects_.push_back(std::move(recommendedModel));
	}
}

void RecommendedScene::Update() {
	BaseScene::Update();
	
	float deltaTime = 1.0f / 60.0f;
	
	// 待機時間の更新
	if (!waitingForInput_) {
		waitTimer_ += deltaTime;
		if (waitTimer_ >= kWaitDuration) {
			waitingForInput_ = true;
		}
	}
	
	// 入力受付開始後、任意のボタンが押されたらタイトルシーンへ遷移
	if (waitingForInput_) {
		auto keyboard = engine_->GetComponent<KeyboardInput>();
		auto gamepad = engine_->GetComponent<GamepadInput>();
		
		bool anyButtonPressed = false;
		
		// キーボード入力チェック（スペースキーまたはEnterキー）
		if (keyboard) {
			if (keyboard->IsKeyTriggered(DIK_SPACE) || keyboard->IsKeyTriggered(DIK_RETURN)) {
				anyButtonPressed = true;
			}
		}
		
		// ゲームパッド入力チェック
		if (!anyButtonPressed && gamepad && gamepad->IsConnected()) {
			// 任意のボタンが押されたかチェック
			if (gamepad->IsButtonTriggered(GamepadButton::A) ||
				gamepad->IsButtonTriggered(GamepadButton::B) ||
				gamepad->IsButtonTriggered(GamepadButton::X) ||
				gamepad->IsButtonTriggered(GamepadButton::Y) ||
				gamepad->IsButtonTriggered(GamepadButton::Start) ||
				gamepad->IsButtonTriggered(GamepadButton::Back) ||
				gamepad->IsButtonTriggered(GamepadButton::LeftShoulder) ||
				gamepad->IsButtonTriggered(GamepadButton::RightShoulder)) {
				anyButtonPressed = true;
			}
		}
		
		// ボタンが押されたらタイトルシーンへ
		if (anyButtonPressed) {
			sceneManager_->ChangeScene("TitleScene");
		}
	}
}

void RecommendedScene::Draw() {
	BaseScene::Draw();
}

void RecommendedScene::Finalize() {
}

void RecommendedScene::SetupReleaseCameraParameters(Camera* camera) {
	if (!camera) {
		return;
	}
	
	// カメラの基本設定
	camera->SetTranslate({ 0.0f, 0.0f, -20.0f });
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
}
