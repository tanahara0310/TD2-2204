#include "TitleScene.h"
#include "EngineSystem/EngineSystem.h"
#include "Scene/SceneManager.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "Engine/Camera/Debug/DebugCamera.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Render/RenderManager.h"
#include "Engine/Graphics/Light/LightManager.h"
#include "Engine/Graphics/Light/LightData.h"
#include "Engine/Graphics/Resource/ResourceFactory.h"
#include "Engine/Graphics/Model/ModelManager.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Input/KeyboardInput.h"
#include "Engine/Input/GamepadInput.h"
#include "Engine/WinApp/WinApp.h"
#include "MathCore.h"
#include <dinput.h>
#include <cmath>


void TitleScene::Initialize(EngineSystem* engine) {
	BaseScene::Initialize(engine);

	// InputSourceの初期化（必須）
	InputSource::Initialize(engine);

	// KeyConfigの設定
	{
		keyConfig_ = std::make_unique<KeyConfig>();

		// 上下方向の移動入力（Vector2として取得）
		ActionBuilder(keyConfig_->AddAction("Move", ActionType::Vector2))
			.BindKeyboardWASD(DIK_W, DIK_S, DIK_A, DIK_D)
			.BindGamepadLeftStick();

		// 上方向の入力（キーボード上キー or ゲームパッドの十字キー上）
		ActionBuilder(keyConfig_->AddAction("Up", ActionType::Bool))
			.BindKey(DIK_UP)
			.BindKey(DIK_W)
			.BindGamepadButton(GamepadButton::DPadUp);

		// 下方向の入力（キーボード下キー or ゲームパッドの十字キー下）
		ActionBuilder(keyConfig_->AddAction("Down", ActionType::Bool))
			.BindKey(DIK_DOWN)
			.BindKey(DIK_S)
			.BindGamepadButton(GamepadButton::DPadDown);

		// 決定ボタン（キーボードスペース or ゲームパッドAボタン）
		ActionBuilder(keyConfig_->AddAction("Confirm", ActionType::Bool))
			.BindKey(DIK_SPACE)
			.BindGamepadButton(GamepadButton::A);
	}

	// 背景の生成と初期化
	{
		auto modelManager = engine_->GetComponent<ModelManager>();
		auto& textureManager = TextureManager::GetInstance();
		
		auto backgroundModel = modelManager->CreateStaticModel("Resources/Models/Background/Background.obj");
		auto backgroundTexture = textureManager.Load("Resources/Textures/Background.png");
		auto background = std::make_unique<Background>();
		background_ = background.get();
		background->Initialize(std::move(backgroundModel), backgroundTexture);
		gameObjects_.push_back(std::move(background));
	}

	{
		// UI初期化
		titleUI_ = std::make_unique<TitleUI>();
		auto sprites = titleUI_->Initialize(engine);

		// スプライトをgameObjects_に追加
		for (auto& sprite : sprites) {
			gameObjects_.push_back(std::move(sprite));
		}
	}

}

void TitleScene::Update() {
	BaseScene::Update();


	if (!titleUI_) {
		return;
	}

	float deltaTime = 1.0f / 60.0f; // 仮のデルタタイム

	// クールダウンタイマーを減少
	if (stickInputCooldown_ > 0.0f) {
		stickInputCooldown_ -= deltaTime;
	}

	// 遷移中でなければ入力を受け付ける
	if (!isTransitioning_) {
		bool selectionChanged = false;

		// キーボード/十字キーでの選択
		if (keyConfig_->GetDown("Up")) {
			titleUI_->SetSelectionState(TitleUI::SelectionState::Start);
			selectionChanged = true;
			stickInputCooldown_ = kStickInputDelay; // クールダウンをリセット
		}

		if (keyConfig_->GetDown("Down")) {
			titleUI_->SetSelectionState(TitleUI::SelectionState::Quit);
			selectionChanged = true;
			stickInputCooldown_ = kStickInputDelay; // クールダウンをリセット
		}

		// スティック入力での選択（クールダウン中でなければ）
		if (!selectionChanged && stickInputCooldown_ <= 0.0f) {
			Vector2 moveInput = keyConfig_->Get<Vector2>("Move");
			
			// 上方向（Y軸正）
			if (moveInput.y > kStickThreshold) {
				titleUI_->SetSelectionState(TitleUI::SelectionState::Start);
				stickInputCooldown_ = kStickInputDelay;
			}
			// 下方向（Y軸負）
			else if (moveInput.y < -kStickThreshold) {
				titleUI_->SetSelectionState(TitleUI::SelectionState::Quit);
				stickInputCooldown_ = kStickInputDelay;
			}
		}

		// 決定ボタン（キーボード or ゲームパッド）
		if (keyConfig_->GetDown("Confirm")) {
			switch (titleUI_->GetSelectionState()) {
			case TitleUI::SelectionState::Start:
				// 決定アニメーション開始（視覚的フィードバック）
				titleUI_->OnConfirm();
				
				// シーン遷移を即座に開始
				isTransitioning_ = true;
				transitionTimer_ = 0.0f;
				break;
				
			case TitleUI::SelectionState::Quit:
				// アプリケーション終了
				PostQuitMessage(0);
				break;
			}
		}
	}

	// 遷移処理
	if (isTransitioning_) {
		UpdateSceneTransition(deltaTime);
	}

	// UI更新（ステートマシーンも含む）
	if (titleUI_) {
		titleUI_->Update();
	}
}



void TitleScene::UpdateSceneTransition(float deltaTime) {
	transitionTimer_ += deltaTime;

	// 遷移時間が経過したらシーン遷移
	if (transitionTimer_ >= kTransitionDuration) {
		sceneManager_->ChangeScene("GameScene");
	}
}

void TitleScene::Draw() {
	BaseScene::Draw();
}

void TitleScene::Finalize() {
}

void TitleScene::SetupReleaseCameraParameters(Camera* camera)
{
	// タイトルシーン専用のカメラパラメータ
	// より引きの視点で全体を見渡せるように設定
	camera->SetTranslate({ 0.0f, 0.0f, -70.0f });
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
}
