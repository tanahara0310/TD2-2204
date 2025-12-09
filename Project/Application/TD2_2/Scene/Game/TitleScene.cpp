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
#include <numbers>
#include "../../Effect/Lightning/LightningEffectManager.h"
#include "Engine/Utility/Random/RandomGenerator.h"
#include "Engine/Math/Easing/EasingUtil.h"
#include "Engine/Graphics/PostEffect/PostEffectManager.h"
#include "Engine/Graphics/PostEffect/Effect/FadeEffect.h"
#include "Engine/Graphics/PostEffect/PostEffectNames.h"

void TitleScene::Initialize(EngineSystem* engine) {

	BaseScene::Initialize(engine);

	// InputSourceの初期化（必須）
	InputSource::Initialize(engine);


	cameraController_ = std::make_unique<TitleCameraController>();
	// TitleUIの初期化
	{
		titleUI_ = std::make_unique<TitleUI>();
		auto uiObjects = titleUI_->Initialize(engine_);
		for (auto& obj : uiObjects) {
			gameObjects_.push_back(std::move(obj));
		}
	}

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

		// 左方向の入力（プリセット選択用）
		ActionBuilder(keyConfig_->AddAction("Left", ActionType::Bool))
			.BindKey(DIK_LEFT)
			.BindKey(DIK_A)
			.BindGamepadButton(GamepadButton::DPadLeft);

		// 右方向の入力（プリセット選択用）
		ActionBuilder(keyConfig_->AddAction("Right", ActionType::Bool))
			.BindKey(DIK_RIGHT)
			.BindKey(DIK_D)
			.BindGamepadButton(GamepadButton::DPadRight);

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
		background_->GetTransform().translate = { 0.0f, 28.7f, -5.4f };
		background_->GetTransform().rotate = { 0.0f, 0.0f, 0.0f };


		// 行列を更新して変更を反映
		background_->GetTransform().TransferMatrix();

		gameObjects_.push_back(std::move(background));
	}


	// デモ演出用の自機と敵の初期化
	{
		auto modelManager = engine_->GetComponent<ModelManager>();
		auto& textureManager = TextureManager::GetInstance();

		// デモプレイヤーの生成
		auto playerModel = modelManager->CreateStaticModel("Resources/Models/Player/Player.obj");
		auto playerTexture = textureManager.Load("Resources/Textures/Player.png");
		auto demoPlayer = std::make_unique<TitlePlayerDemo>();
		demoPlayer_ = demoPlayer.get();
		demoPlayer->Initialize(std::move(playerModel), playerTexture);
		
		// 初期位置を画面左端に設定（初期は背景の後ろ、中段）
		Vector3 playerInitPos = { -35.0f, 24.0f, 10.0f };
		demoPlayer->GetTransform().translate = playerInitPos;
		demoPlayer->GetTransform().rotate.y = std::numbers::pi_v<float> / 2.0f; // +X方向を向く
		demoPlayer->SetInitialPosition(playerInitPos); // initialPositionも更新
		demoPlayer->SetMoveSpeed(23.0f); // 初期速度を設定（調整済み）
		demoPlayer->GetTransform().TransferMatrix();
		
		gameObjects_.push_back(std::move(demoPlayer));

		// デモエネミーの生成
		auto enemyModel = modelManager->CreateStaticModel("Resources/Models/Boss/Boss.obj");
		auto enemyTexture = textureManager.Load("Resources/Textures/Boss.png");
		auto demoEnemy = std::make_unique<TitleEnemyDemo>();
		demoEnemy_ = demoEnemy.get();
		demoEnemy->Initialize(std::move(enemyModel), enemyTexture);
		demoEnemy->SetTarget(demoPlayer_);
		
		// 初期位置を画面左端（プレイヤーの後ろ）に設定（初期は背景の後ろ、中段）
		Vector3 enemyInitPos = { -45.0f, 24.0f, 10.0f };
		demoEnemy->GetTransform().translate = enemyInitPos;
		demoEnemy->GetTransform().rotate.y = std::numbers::pi_v<float> / 2.0f; // +X方向を向く
		demoEnemy->SetInitialPosition(enemyInitPos); // initialPositionも更新
		demoEnemy->SetChaseSpeed(20.0f); // 初期速度を設定（調整済み）
		demoEnemy->GetTransform().TransferMatrix();
		
		gameObjects_.push_back(std::move(demoEnemy));

		// デモマネージャーの初期化
		demoManager_ = std::make_unique<TitleDemoManager>();
		demoManager_->Initialize(demoPlayer_, demoEnemy_);
	}

	// 雷エフェクトの初期化
	{
		lightningManager_ = std::make_unique<LightningEffectManager>();
		lightningManager_->Initialize(engine_->GetComponent<ModelManager>(), &TextureManager::GetInstance());

		lightningFrameManager_ = std::make_unique<TitleLightningFrameManager>();
		lightningFrameManager_->Initialize(lightningManager_.get(), gameObjects_);
	}

	// 決定演出マネージャーの初期化
	{
		confirmAnimationManager_ = std::make_unique<TitleConfirmAnimationManager>();
		confirmAnimationManager_->Initialize(titleUI_.get(), lightningManager_.get());
	}

	// BGM再生
	{
		auto audio = engine_->GetComponent<SoundManager>();
		if (audio) {
			titleBGM_ = audio->CreateSoundResource("Resources/Audio/BGM/Title.mp3");

			if (titleBGM_ && titleBGM_->IsValid()) {
				titleBGM_->Play(true);
				titleBGM_->SetVolume(0.5f);
			}
		}
	}

}

void TitleScene::Update() {
	BaseScene::Update();

	// カメラコントローラーを毎フレーム適用
	if (cameraController_ && cameraManager_) {
		auto* activeCamera = cameraManager_->GetActiveCamera();
		if (activeCamera) {
			auto* camera = dynamic_cast<Camera*>(activeCamera);
			if (camera) {
				cameraController_->ApplyToCamera(camera);
			}
		}
	}

#ifdef _DEBUG
	// カメラコントローラーのImGui表示
	if (cameraController_) {
		cameraController_->DrawImGui();
	}

	// デモ演出のImGuiコントロール
	if (demoManager_) {
		demoManager_->DrawImGui();
	}
#endif

	if (!titleUI_) {
		return;
	}

	float deltaTime = 1.0f / 60.0f;

	// クールダウンタイマーを減少
	if (stickInputCooldown_ > 0.0f) {
		stickInputCooldown_ -= deltaTime;
	}

	// デモマネージャーの更新
	if (demoManager_) {
		demoManager_->Update(deltaTime);
	}

	// 雷エフェクトフレームの更新
	if (lightningFrameManager_) {
		bool isConfirmAnimating = confirmAnimationManager_ && confirmAnimationManager_->IsAnimating();
		lightningFrameManager_->Update(deltaTime, isConfirmAnimating);

		// 現在選択中のモデル位置へ枠を移動＆色変更
		bool isStartSelected = (titleUI_->GetSelectionState() == TitleUI::SelectionState::Start);
		lightningFrameManager_->UpdateFramePosition(isStartSelected);
	}

	// 遷移中または決定演出中でなければ入力を受け付ける
	bool isConfirmAnimating = confirmAnimationManager_ && confirmAnimationManager_->IsAnimating();
	if (!isTransitioning_ && !isConfirmAnimating) {
		bool selectionChanged = false;

		// キーボード/十字キーでの選択
		if (keyConfig_->GetDown("Up")) {
			titleUI_->SetSelectionState(TitleUI::SelectionState::Start);
			selectionChanged = true;
			stickInputCooldown_ = kStickInputDelay;
		}

		if (keyConfig_->GetDown("Down")) {
			titleUI_->SetSelectionState(TitleUI::SelectionState::Quit);
			selectionChanged = true;
			stickInputCooldown_ = kStickInputDelay;
		}

		// プリセット選択（左右キー）		
		if (keyConfig_->GetDown("Left")) {
			titleUI_->SelectPreviousPreset();
			stickInputCooldown_ = kStickInputDelay;
		}

		if (keyConfig_->GetDown("Right")) {
		 titleUI_->SelectNextPreset();
			stickInputCooldown_ = kStickInputDelay;
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
			// 左方向（X軸負）
			else if (moveInput.x < -kStickThreshold) {
			 titleUI_->SelectPreviousPreset();
				stickInputCooldown_ = kStickInputDelay;
			}
			// 右方向（X軸正）
			else if (moveInput.x > kStickThreshold) {
				titleUI_->SelectNextPreset();
				stickInputCooldown_ = kStickInputDelay;
			}
		}

		// 決定ボタン（キーボード or ゲームパッド）
		if (keyConfig_->GetDown("Confirm")) {
			bool isStartSelected = (titleUI_->GetSelectionState() == TitleUI::SelectionState::Start);
			if (confirmAnimationManager_) {
				confirmAnimationManager_->StartAnimation(isStartSelected, frameEffectIds_);
			}
		}
	}

	// 決定演出の更新
	if (confirmAnimationManager_) {
		confirmAnimationManager_->Update(deltaTime);

		// 演出が終了したら
		if (!confirmAnimationManager_->IsAnimating() && isConfirmAnimating) {
			if (confirmAnimationManager_->WasStartSelected()) {
				// スタート選択時はシーン遷移を開始
			 isTransitioning_ = true;
				transitionTimer_ = 0.0f;
			} else {
				// やめる選択時は待機時間を開始
				isWaitingForQuit_ = true;
				quitWaitTimer_ = 0.0f;
			}
		}
	}

	// フェードアウト処理
	if (isFadingOut_) {
		UpdateFadeOut(deltaTime);
	}

	// 終了待機処理
	if (isWaitingForQuit_) {
		quitWaitTimer_ += deltaTime;
		if (quitWaitTimer_ >= kQuitWaitDuration) {
			// 待機時間が経過したらフェードアウトを開始
			isWaitingForQuit_ = false;
			isFadingOut_ = true;
			fadeOutTimer_.Start(kFadeOutDuration, false);
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

void TitleScene::UpdateFadeOut(float deltaTime) {
	// タイマーの更新
	fadeOutTimer_.Update(deltaTime);

	// 進行度を取得（0.0～1.0）
	float progress = fadeOutTimer_.GetProgress();

	// PostEffectManagerを取得してフェードを適用
	auto* postEffectManager = engine_->GetComponent<PostEffectManager>();
	if (postEffectManager) {
		auto* fadeEffect = postEffectManager->GetEffect<FadeEffect>(PostEffectNames::FadeEffect);
		if (fadeEffect) {
			// フェードエフェクトを有効化
			postEffectManager->SetEffectEnabled(PostEffectNames::FadeEffect, true);

			// 黒フェードを設定
			fadeEffect->SetFadeType(FadeEffect::FadeType::BlackFade);
			// フェードアウト（徐々に黒くなる）
			fadeEffect->SetFadeAlpha(progress);
		}
	}

	// フェードアウトが完了したらアプリケーションを終了
	if (fadeOutTimer_.IsFinished()) {
		isFadingOut_ = false;
		PostQuitMessage(0);
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
