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
#include "Engine/Audio/SoundManager.h"
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

	// カメラコントローラーの初期化
	cameraController_ = std::make_unique<TitleCameraController>();

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
		
		// タイトルシーン専用の背景モデルパラメータ
		background_->GetTransform().translate = { 47.8f, 12.7f, -27.7f };
		background_->GetTransform().rotate = { 0.0f, 0.0f, 0.0f };

		
		// 行列を更新して変更を反映
		background_->GetTransform().TransferMatrix();
		
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

	// ライトニングエフェクトの作成
	CreateLightningEffects();

	// BGM再生
	{
		auto audio = engine_->GetComponent<SoundManager>();
		if (audio) {
			titleBGM_ = audio->CreateSoundResource("Resources/Audio/BGM/Title.mp3");
			
			if (titleBGM_ && titleBGM_->IsValid()) {
				titleBGM_->Play(true); // ループ再生
				titleBGM_->SetVolume(0.4f); // 音量調整
			}
		}
	}

}

void TitleScene::Update() {
	BaseScene::Update();

	// ライトニングエフェクトの更新
	if (lightningManager_) {
		lightningManager_->UpdateAllEffects();
	}

	// カメラコントローラーを毎フレーム適用（デフォルト値も含む）
	if (cameraController_ && cameraManager_) {
		auto* activeCamera = cameraManager_->GetActiveCamera();
		if (activeCamera) {
			auto* camera = dynamic_cast<Camera*>(activeCamera);
			if (camera) {
				cameraController_->ApplyToCamera(camera);
			}
		}
	}

	// カメラコントローラーのImGui表示
	if (cameraController_) {
		cameraController_->DrawImGui();
	}

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
	// カメラコントローラーを使ってカメラパラメータを適用
	if (cameraController_) {
		cameraController_->ApplyToCamera(camera);
	}
}

void TitleScene::CreateLightningEffects()
{
	auto modelManager = engine_->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();
	
	// ライトニングエフェクトマネージャーの初期化
	lightningManager_ = std::make_unique<LightningEffectManager>();
	lightningManager_->Initialize(modelManager, &textureManager);
	
	// UIモデルの取得
	StartModel* startModel = titleUI_->GetStartModel();
	/*YameruModel* yameruModel = titleUI_->GetYameruModel();*/
	
	// エフェクト設定（直線ライトニング）
	LightningEffectManager::LinearEffectConfig config;
	config.color = { 0.3f, 0.6f, 1.0f, 1.0f };  // 青系の雷
	config.voxelScale = { 1.0f, 1.0f, 1.0f };
	config.noiseScale = 0.3f;
	config.noiseSpeed = 12.0f;
	config.segmentCount = 5;
	config.pathType = Lightning::PathType::Linear;
	config.enableAnimation = true;
	
	// UIモデルを囲むサイズ（大きめに設定して間隔を広げる）
	constexpr float frameWidth = 4.0f;   // 横幅
	constexpr float frameHeight = 1.5f;  // 縦幅
	
	// ===== StartModel用のライトニング（四方を囲む） =====
	if (startModel) {
		Vector3 startPos = startModel->GetWorldPosition();
		
		// 上辺（左から右へ）
		config.startOffset = { -frameWidth / 2.0f, frameHeight / 2.0f, 0.0f };
		config.endOffset = { frameWidth / 2.0f, frameHeight / 2.0f, 0.0f };
		startLightningEffects_[0] = lightningManager_->CreateLinearEffectAtPosition(startPos, config, gameObjects_);
		
		// 下辺（左から右へ）
		config.startOffset = { -frameWidth / 2.0f, -frameHeight / 2.0f, 0.0f };
		config.endOffset = { frameWidth / 2.0f, -frameHeight / 2.0f, 0.0f };
		startLightningEffects_[1] = lightningManager_->CreateLinearEffectAtPosition(startPos, config, gameObjects_);
		
		// 左辺（下から上へ）
		config.startOffset = { -frameWidth / 2.0f, -frameHeight / 2.0f, 0.0f };
		config.endOffset = { -frameWidth / 2.0f, frameHeight / 2.0f, 0.0f };
		startLightningEffects_[2] = lightningManager_->CreateLinearEffectAtPosition(startPos, config, gameObjects_);
		
		// 右辺（下から上へ）
		config.startOffset = { frameWidth / 2.0f, -frameHeight / 2.0f, 0.0f };
		config.endOffset = { frameWidth / 2.0f, frameHeight / 2.0f, 0.0f };
		startLightningEffects_[3] = lightningManager_->CreateLinearEffectAtPosition(startPos, config, gameObjects_);
	}
	//
	//// ===== YameruModel用のライトニング（四方を囲む） =====
	//if (yameruModel) {
	//	Vector3 yameruPos = yameruModel->GetWorldPosition();
	//	
	//	// 上辺（左から右へ）
	//	config.startOffset = { -frameWidth / 2.0f, frameHeight / 2.0f, 0.0f };
	//	config.endOffset = { frameWidth / 2.0f, frameHeight / 2.0f, 0.0f };
	//	yameruLightningEffects_[0] = lightningManager_->CreateLinearEffectAtPosition(yameruPos, config, gameObjects_);
	//	
	//	// 下辺（左から右へ）
	//	config.startOffset = { -frameWidth / 2.0f, -frameHeight / 2.0f, 0.0f };
	//	config.endOffset = { frameWidth / 2.0f, -frameHeight / 2.0f, 0.0f };
	//	yameruLightningEffects_[1] = lightningManager_->CreateLinearEffectAtPosition(yameruPos, config, gameObjects_);
	//	
	//	// 左辺（下から上へ）
	//	config.startOffset = { -frameWidth / 2.0f, -frameHeight / 2.0f, 0.0f };
	//	config.endOffset = { -frameWidth / 2.0f, frameHeight / 2.0f, 0.0f };
	//	yameruLightningEffects_[2] = lightningManager_->CreateLinearEffectAtPosition(yameruPos, config, gameObjects_);
	//	
	//	// 右辺（下から上へ）
	//	config.startOffset = { frameWidth / 2.0f, -frameHeight / 2.0f, 0.0f };
	//	config.endOffset = { frameWidth / 2.0f, frameHeight / 2.0f, 0.0f };
	//	yameruLightningEffects_[3] = lightningManager_->CreateLinearEffectAtPosition(yameruPos, config, gameObjects_);
	//}
}
