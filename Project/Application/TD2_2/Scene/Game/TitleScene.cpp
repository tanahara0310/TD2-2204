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
		
		// 初期位置を画面左端に設定（初期は背景の後ろ）
		Vector3 playerInitPos = { -35.0f, 24.0f, kDemoZBehind_ };
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
		
		// 初期位置を画面左端（プレイヤーの後ろ）に設定（初期は背景の後ろ）
		Vector3 enemyInitPos = { -45.0f, 24.0f, kDemoZBehind_ };
		demoEnemy->GetTransform().translate = enemyInitPos;
		demoEnemy->GetTransform().rotate.y = std::numbers::pi_v<float> / 2.0f; // +X方向を向く
		demoEnemy->SetInitialPosition(enemyInitPos); // initialPositionも更新
		demoEnemy->SetChaseSpeed(20.0f); // 初期速度を設定（調整済み）
		demoEnemy->GetTransform().TransferMatrix();
		
		gameObjects_.push_back(std::move(demoEnemy));

		// デモの初期設定（パターン1: 敵が自機を追跡、+X方向）
		demoPlayer_->SetChasingMode(false); // プレイヤーは通常移動
		demoPlayer_->SetMoveDirection(1.0f); // +X方向
		demoEnemy_->SetChasingMode(true); // エネミーは追跡
		demoEnemy_->SetMoveDirection(1.0f); // +X方向
	}

	// 雷エフェクトの初期化と生成
	{
		lightningManager_ = std::make_unique<LightningEffectManager>();
		lightningManager_->Initialize(engine_->GetComponent<ModelManager>(), &TextureManager::GetInstance());

		LightningEffectManager::EffectConfig config;
		config.segmentCount = 4;               // 直線に近い少ないセグメント
		config.noiseScale = 0.8f;             // ノイズ極小
		config.noiseSpeed = 60.0f;              // ノイズ速度も低め
		config.randomOffsetRange = 0.3f;      // 始点/終点の揺らぎ範囲を拡大（動きを見せる）
		config.voxelScale = { 1.0f, 1.0f, 1.0f }; // ボクセルサイズ小さめ
		config.initialVisible = false; // タイトルでは初期非表示
		// タイトルはボクセル間隔を狭く
		config.voxelSpacing = 0.2f;

		// 色設定
		Vector4 startColor = { 0.6f, 0.9f, 1.0f, 1.0f };
		Vector4 quitColor = { 0.9f, 1.0f, 1.0f, 1.0f };

		// 矩形サイズ（共通）
		float halfWidth = 1.9f;
		float halfHeight = 0.5f;
		float frameYOffset = 0.0f; // オフセットなし（モデルの中心に合わせる）

		// 初期中心はStartの位置（新しい座標: {0.0f, -5.5f, -60.9f}）
		Vector3 startCenter = { 0.0f, -5.5f + frameYOffset, -60.9f };

		config.color = startColor;
		// 上辺（index 0）
		config.startOffset = { -halfWidth,  halfHeight, 0.0f };
		config.endOffset = { halfWidth,  halfHeight, 0.0f };
		frameEffectIds_[0] = lightningManager_->CreateEffect(startCenter, config, gameObjects_);
		// 下辺（index 1）
		config.startOffset = { -halfWidth, -halfHeight, 0.0f };
		config.endOffset = { halfWidth, -halfHeight, 0.0f };
		frameEffectIds_[1] = lightningManager_->CreateEffect(startCenter, config, gameObjects_);
		// 左辺（index 2）
		config.startOffset = { -halfWidth, -halfHeight, 0.0f };
		config.endOffset = { -halfWidth,  halfHeight, 0.0f };
		frameEffectIds_[2] = lightningManager_->CreateEffect(startCenter, config, gameObjects_);
		// 右辺（index 3）
		config.startOffset = { halfWidth, -halfHeight, 0.0f };
		config.endOffset = { halfWidth,  halfHeight, 0.0f };
		frameEffectIds_[3] = lightningManager_->CreateEffect(startCenter, config, gameObjects_);

		// 初期は非表示（パルス時のみ表示）
		for (int i = 0; i < 4; ++i) {
			if (frameEffectIds_[i] >= 0) {
				lightningManager_->SetEffectVisible(frameEffectIds_[i], false);
			}
		}

		// 選択状態キャッシュ初期化
		lastIsStartSelected_ = true;
	}

	// BGM再生
	{
		auto audio = engine_->GetComponent<SoundManager>();
		if (audio) {
			titleBGM_ = audio->CreateSoundResource("Resources/Audio/BGM/Title.mp3");

			if (titleBGM_ && titleBGM_->IsValid()) {
				titleBGM_->Play(true); // ループ再生
				titleBGM_->SetVolume(0.5f); // 音量調整
			}
		}
	}

}

void TitleScene::Update() {
	BaseScene::Update();

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

#ifdef _DEBUG
	// カメラコントローラーのImGui表示
	if (cameraController_) {
		cameraController_->DrawImGui();
	}

	// デモ演出のImGuiコントロール
	if (ImGui::Begin("Title Demo Control")) {
		ImGui::Text("Demo Settings");
		ImGui::Separator();

		// 現在のパターン表示
		const char* patternName = (currentDemoPattern_ == DemoPattern::EnemyChasePlayer)
			? "Enemy Chase Player" : "Player Chase Enemy";
		ImGui::Text("Current Pattern: %s", patternName);
		ImGui::Text("Moving Direction: %s", isMovingRight_ ? "+X (Right)" : "-X (Left)");
		ImGui::Text("Z Position: %s", isDemoBehindBackground_ ? "Behind Background" : "In Front of Background");
		ImGui::Text("Switch Counter: %d / %d (Behind: %d, Front: %d)", 
			demoSwitchCounter_, 
			kDemoTotalCycle_,
			kDemoBehindCount_,
			kDemoInFrontCount_);

		ImGui::Spacing();

		// パターン手動切り替えボタン
		if (ImGui::Button("Switch Demo Pattern")) {
			SwitchDemoPattern();
		}

		ImGui::Spacing();
		ImGui::Separator();

		// リセットボタン
		if (ImGui::Button("Reset Demo Positions")) {
			if (demoPlayer_) {
				demoPlayer_->ResetToInitialPosition();
			}
			if (demoEnemy_) {
				demoEnemy_->ResetToInitialPosition();
			}
		}

		ImGui::Spacing();

		// プレイヤーの速度調整
		if (demoPlayer_) {
			float playerSpeed = demoPlayer_->GetMoveSpeed();
			if (ImGui::SliderFloat("Player Speed", &playerSpeed, 1.0f, 20.0f)) {
				demoPlayer_->SetMoveSpeed(playerSpeed);
			}
		}

		// エネミーの速度調整
		if (demoEnemy_) {
			float enemySpeed = demoEnemy_->GetChaseSpeed();
			if (ImGui::SliderFloat("Enemy Chase Speed", &enemySpeed, 1.0f, 25.0f)) {
				demoEnemy_->SetChaseSpeed(enemySpeed);
			}
		}

		ImGui::Spacing();
		ImGui::Separator();

		// 現在位置の表示
		if (demoPlayer_) {
			Vector3 playerPos = demoPlayer_->GetWorldPosition();
			ImGui::Text("Player Pos: (%.1f, %.1f, %.1f)", playerPos.x, playerPos.y, playerPos.z);
			ImGui::Text("Player Mode: %s", demoPlayer_->IsChasingMode() ? "Chasing" : "Moving");
		}
		if (demoEnemy_) {
			Vector3 enemyPos = demoEnemy_->GetWorldPosition();
			ImGui::Text("Enemy Pos: (%.1f, %.1f, %.1f)", enemyPos.x, enemyPos.y, enemyPos.z);
			ImGui::Text("Enemy Mode: %s", demoEnemy_->IsChasingMode() ? "Chasing" : "Moving");
		}

		ImGui::End();
	}
#endif

	if (!titleUI_) {
		return;
	}

	float deltaTime = 1.0f / 60.0f; // 仮のデルタタイム

	// クールダウンタイマーを減少
	if (stickInputCooldown_ > 0.0f) {
		stickInputCooldown_ -= deltaTime;
	}

	// 雷エフェクトの更新
	if (lightningManager_) {
		lightningManager_->UpdateAllEffects();
	}

	// 現在選択中のモデル位置へ枠を移動＆色変更（選択変更時）
	{
		bool isStartSelected = (titleUI_->GetSelectionState() == TitleUI::SelectionState::Start);
		float frameYOffset = 0.0f; // オフセットなし（モデルの中心に合わせる）
		Vector3 startCenter = { 0.0f, -5.5f + frameYOffset, -60.9f };
		Vector3 quitCenter = { 0.0f, -7.0f + frameYOffset, -60.9f };
		Vector3 target = isStartSelected ? startCenter : quitCenter;

		for (int i = 0; i < 4; i++) {
			if (frameEffectIds_[i] >= 0) {
				lightningManager_->SetEffectPosition(frameEffectIds_[i], target);
			}
		}

		if (lastIsStartSelected_ != isStartSelected) {
			Vector4 startColor = { 0.6f, 0.9f, 1.0f, 1.0f };
			Vector4 quitColor = { 0.9f, 1.0f, 1.0f, 1.0f };
			Vector4 color = isStartSelected ? startColor : quitColor;
			for (int i = 0; i < 4; i++) {
				if (frameEffectIds_[i] >= 0) {
					lightningManager_->SetEffectColor(frameEffectIds_[i], color);
				}
			}
			lastIsStartSelected_ = isStartSelected;
		}
	}

	// 選択中に一定間隔で一瞬だけ表示するパルス制御（ランダムな2辺を雷っぽく点滅）
	// ただし決定演出中は実行しない
	if (!isConfirmAnimating_) {
		pulseTimer_ += deltaTime;

		// パルス未表示状態で、間隔到達したら開始（乱数で間隔を決定）
		if (visibleTimer_ <= 0.0f && pulseTimer_ >= nextPulseInterval_) {
			pulseTimer_ = 0.0f;
			visibleTimer_ = kPulseDuration_;
			flickerTimer_ = 0.0f;
			// 次回間隔を0.5〜2.0秒の乱数で設定
			nextPulseInterval_ = RandomGenerator::GetInstance().GetFloat(kPulseIntervalMin_, kPulseIntervalMax_);
			// 辺をランダム選択（2本、重複なし）
			currentEdgeIndexA_ = RandomGenerator::GetInstance().GetInt(0, 3);
			do {
				currentEdgeIndexB_ = RandomGenerator::GetInstance().GetInt(0, 3);
			} while (currentEdgeIndexB_ == currentEdgeIndexA_);
			// 選ばれた2辺のみ即座に表示、その他は即座に非表示
			for (int i = 0; i < 4; i++) {
				if (frameEffectIds_[i] < 0) continue;
				bool isSelected = (i == currentEdgeIndexA_ || i == currentEdgeIndexB_);
				lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], isSelected);
			}
		}

		// 表示中なら雷っぽく点滅（選ばれた2辺のみ）
		if (visibleTimer_ > 0.0f && currentEdgeIndexA_ >= 0 && currentEdgeIndexB_ >= 0) {
			visibleTimer_ -= deltaTime;
			flickerTimer_ += deltaTime;
			if (flickerTimer_ >= kFlickerInterval_) {
				flickerTimer_ = 0.0f;
				static bool flickerOn = false;
				flickerOn = !flickerOn;
				lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[currentEdgeIndexA_], flickerOn);
				lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[currentEdgeIndexB_], flickerOn);
				// 選ばれていない辺は常に即座に非表示維持
				for (int i = 0; i < 4; i++) {
					if ((i == currentEdgeIndexA_ || i == currentEdgeIndexB_) || frameEffectIds_[i] < 0) continue;
					lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], false);
				}
			}

			// 表示時間終了で後処理（全て即座に非表示へ）
			if (visibleTimer_ <= 0.0f) {
				for (int i = 0; i < 4; ++i) if (frameEffectIds_[i] >= 0) lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], false);
				currentEdgeIndexA_ = -1;
				currentEdgeIndexB_ = -1;
			}
		}
	}

	// 遊んでいない
	// 遷移中または決定演出中でなければ入力を受け付ける
	if (!isTransitioning_ && !isConfirmAnimating_) {
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
			switch (titleUI_->GetSelectionState()) {
			case TitleUI::SelectionState::Start:
				// 決定演出を開始
				isConfirmAnimating_ = true;
				confirmAnimationTimer_.Start(kConfirmAnimationDuration, false);

				// 選択されたモデルの初期スケールを保存
				if (titleUI_->GetStartModel()) {
					auto* startModel = titleUI_->GetStartModel();
					selectedModelInitialScale_ = startModel->GetTransform().scale.x;
					// 決定演出モードを有効化（呼吸アニメーションを停止）
					startModel->SetConfirmingMode(true);
				}

				// 非選択モデルの元の色を保存
				if (titleUI_->GetYameruModel() && titleUI_->GetYameruModel()->GetModel()) {
					auto* materialManager = titleUI_->GetYameruModel()->GetModel()->GetMaterialManager();
					if (materialManager) {
						unselectedModelOriginalColor_ = materialManager->GetColor();
					}
				}

				// 雷エフェクトを全辺一斉に強く点滅させる
				for (int i = 0; i < 4; ++i) {
					if (frameEffectIds_[i] >= 0) {
						lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], true);
					}
				}
				break;

			case TitleUI::SelectionState::Quit:
				// 決定演出を開始
				isConfirmAnimating_ = true;
				confirmAnimationTimer_.Start(kConfirmAnimationDuration, false);

				// 選択されたモデルの初期スケールを保存
				if (titleUI_->GetYameruModel()) {
					auto* yameruModel = titleUI_->GetYameruModel();
					selectedModelInitialScale_ = yameruModel->GetTransform().scale.x;
					// 決定演出モードを有効化（呼吸アニメーションを停止）
					yameruModel->SetConfirmingMode(true);
				}

				// 非選択モデルの元の色を保存
				if (titleUI_->GetStartModel() && titleUI_->GetStartModel()->GetModel()) {
					auto* materialManager = titleUI_->GetStartModel()->GetModel()->GetMaterialManager();
					if (materialManager) {
						unselectedModelOriginalColor_ = materialManager->GetColor();
					}
				}

				// 雷エフェクトを全辺一斉に強く点滅させる
				for (int i = 0; i < 4; ++i) {
					if (frameEffectIds_[i] >= 0) {
						lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], true);
					}
				}
				break;
			}
		}
	}

	// 決定演出の更新
	if (isConfirmAnimating_) {
		UpdateConfirmAnimation(deltaTime);
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

	// デモ演出の自動リセット（画面外に出た場合）
	if (demoPlayer_) {
		Vector3 playerPos = demoPlayer_->GetWorldPosition();
		// X座標が範囲外に出たらパターンを切り替え（背景の見える範囲に調整）
		bool shouldSwitch = false;
		if (isMovingRight_ && playerPos.x > 50.0f) {  // 右端を50.0fに修正
			shouldSwitch = true;
		} else if (!isMovingRight_ && playerPos.x < -50.0f) {  // 左端をさらに左にずらす
			shouldSwitch = true;
		}

		if (shouldSwitch) {
			SwitchDemoPattern();
		}
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

void TitleScene::SwitchDemoPattern() {
	if (!demoPlayer_ || !demoEnemy_) {
		return;
	}

	// カウンターを増加
	demoSwitchCounter_++;

	// カウンターが総サイクルに達したらリセット
	if (demoSwitchCounter_ >= kDemoTotalCycle_) {
		demoSwitchCounter_ = 0;
	}

	// 2回後ろ→1回前のパターン
	// カウンター 0, 1 → 背景の後ろ
	// カウンター 2 → 背景の前
	if (demoSwitchCounter_ < kDemoBehindCount_) {
		isDemoBehindBackground_ = true; // 背景の後ろ
	} else {
		isDemoBehindBackground_ = false; // 背景の前
	}

	// 移動方向を反転
	isMovingRight_ = !isMovingRight_;
	float direction = isMovingRight_ ? 1.0f : -1.0f;

	// パターンを切り替え
	if (currentDemoPattern_ == DemoPattern::EnemyChasePlayer) {
		SwitchToPlayerChaseEnemy(direction);
	} else {
		SwitchToEnemyChasePlayer(direction);
	}

	// 位置と回転を設定
	SetDemoPositions();

	// トランスフォームを更新
	demoPlayer_->GetTransform().TransferMatrix();
	demoEnemy_->GetTransform().TransferMatrix();
}

void TitleScene::SwitchToPlayerChaseEnemy(float direction) {
	// 次は自機が敵を追跡
	currentDemoPattern_ = DemoPattern::PlayerChaseEnemy;

	// 自機: 追跡モード（遅い）
	demoPlayer_->SetChasingMode(true);
	demoPlayer_->SetTarget(demoEnemy_);
	demoPlayer_->SetMoveDirection(direction);
	demoPlayer_->SetMoveSpeed(20.0f); // 速度を上げる（17.5f → 20.0f）

	// 敵: 通常移動モード（逃げる、速い）
	demoEnemy_->SetChasingMode(false);
	demoEnemy_->SetMoveDirection(direction);
	demoEnemy_->SetChaseSpeed(23.0f); // 速度を下げる（24.0f → 23.0f）
}

void TitleScene::SwitchToEnemyChasePlayer(float direction) {
	// 次は敵が自機を追跡
	currentDemoPattern_ = DemoPattern::EnemyChasePlayer;

	// 自機: 通常移動モード（逃げる、速い）
	demoPlayer_->SetChasingMode(false);
	demoPlayer_->SetMoveDirection(direction);
	demoPlayer_->SetMoveSpeed(23.0f); // 速度を下げる（25.0f → 23.0f）

	// 敵: 追跡モード（遅い）
	demoEnemy_->SetChasingMode(true);
	demoEnemy_->SetMoveDirection(direction);
	demoEnemy_->SetChaseSpeed(20.0f); // 速度を下げる（22.5f → 20.0f）
}

void TitleScene::SetDemoPositions() {
	constexpr float kDemoY = 24.0f;
	constexpr float kLeftStartX = -35.0f;
	constexpr float kLeftBackX = -45.0f;
	constexpr float kRightStartX = 45.0f;
	constexpr float kRightBackX = 55.0f;

	// Z座標を背景の前後で切り替え
	float demoZ = isDemoBehindBackground_ ? kDemoZBehind_ : kDemoZFront_;

	float rotation = isMovingRight_ 
		? std::numbers::pi_v<float> / 2.0f    // +X方向: 90度
		: -std::numbers::pi_v<float> / 2.0f;  // -X方向: -90度

	// 追跡パターンに応じて前後を決定
	bool isPlayerFront = (currentDemoPattern_ == DemoPattern::EnemyChasePlayer);
	
	if (isMovingRight_) {
		// 左から右へ
		if (isPlayerFront) {
			// 自機が前、敵が後ろ
			demoPlayer_->GetTransform().translate = { kLeftStartX, kDemoY, demoZ };
			demoPlayer_->GetTransform().rotate.y = rotation;
			demoPlayer_->SetInitialPosition({ kLeftStartX, kDemoY, demoZ }); // initialPositionも更新
			demoEnemy_->GetTransform().translate = { kLeftBackX, kDemoY, demoZ };
			demoEnemy_->GetTransform().rotate.y = rotation;
			demoEnemy_->SetInitialPosition({ kLeftBackX, kDemoY, demoZ }); // initialPositionも更新
		} else {
			// 敵が前、自機が後ろ
			demoEnemy_->GetTransform().translate = { kLeftStartX, kDemoY, demoZ };
			demoEnemy_->GetTransform().rotate.y = rotation;
			demoEnemy_->SetInitialPosition({ kLeftStartX, kDemoY, demoZ }); // initialPositionも更新
			demoPlayer_->GetTransform().translate = { kLeftBackX, kDemoY, demoZ };
			demoPlayer_->GetTransform().rotate.y = rotation;
			demoPlayer_->SetInitialPosition({ kLeftBackX, kDemoY, demoZ }); // initialPositionも更新
		}
	} else {
		// 右から左へ
		if (isPlayerFront) {
			// 自機が前、敵が後ろ
			demoPlayer_->GetTransform().translate = { kRightStartX, kDemoY, demoZ };
			demoPlayer_->GetTransform().rotate.y = rotation;
			demoPlayer_->SetInitialPosition({ kRightStartX, kDemoY, demoZ }); // initialPositionも更新
			demoEnemy_->GetTransform().translate = { kRightBackX, kDemoY, demoZ };
			demoEnemy_->GetTransform().rotate.y = rotation;
			demoEnemy_->SetInitialPosition({ kRightBackX, kDemoY, demoZ }); // initialPositionも更新
		} else {
			// 敵が前、自機が後ろ
			demoEnemy_->GetTransform().translate = { kRightStartX, kDemoY, demoZ };
			demoEnemy_->GetTransform().rotate.y = rotation;
			demoEnemy_->SetInitialPosition({ kRightStartX, kDemoY, demoZ }); // initialPositionも更新
			demoPlayer_->GetTransform().translate = { kRightBackX, kDemoY, demoZ };
			demoPlayer_->GetTransform().rotate.y = rotation;
			demoPlayer_->SetInitialPosition({ kRightBackX, kDemoY, demoZ }); // initialPositionも更新
		}
	}
}

void TitleScene::UpdateConfirmAnimation(float deltaTime) {
	if (!titleUI_) {
		return;
	}

	// タイマーの更新
	confirmAnimationTimer_.Update(deltaTime);

	// 進行度を取得（0.0～1.0）
	float progress = confirmAnimationTimer_.GetProgress();

	// 緩急のある山型イージング（0→1→0）
	float easedProgress;
	if (progress < 0.5f) {
		float t = progress * 3.0f;
		float eased = EasingUtil::Apply(t, EasingUtil::Type::EaseOutQuad);
		easedProgress = eased; // 0→1
	} else {
		float t = (progress - 0.5f) * 3.0f;
		float eased = EasingUtil::Apply(t, EasingUtil::Type::EaseInQuad);
		easedProgress = 1.0f - eased; // 1→0
	}

	// 選択状態に応じた処理
	if (titleUI_->GetSelectionState() == TitleUI::SelectionState::Start) {
		// スタート選択時の処理
		auto* startModel = titleUI_->GetStartModel();
		if (startModel) {
			float scaleMultiplier = 1.0f + (kSelectedScaleMax - 1.0f) * easedProgress;
			float newScale = selectedModelInitialScale_ * scaleMultiplier;
			startModel->GetTransform().scale = { newScale, newScale, newScale };
		}

		// 非選択モデル（Yameru）のフェードアウト（α値を下げて透明に）
		auto* yameruModel = titleUI_->GetYameruModel();
		if (yameruModel && yameruModel->GetModel()) {
			auto* materialManager = yameruModel->GetModel()->GetMaterialManager();
			if (materialManager) {
				// α値を徐々に下げて透明にする（RGB成分は維持）
				float alpha = 1.0f - progress;
				Vector4 fadedColor = {
					unselectedModelOriginalColor_.x,
					unselectedModelOriginalColor_.y,
					unselectedModelOriginalColor_.z,
					alpha
				};
				materialManager->SetColor(fadedColor);
			}
		}
	} else if (titleUI_->GetSelectionState() == TitleUI::SelectionState::Quit) {
		// やめる選択時の処理
		auto* yameruModel = titleUI_->GetYameruModel();
		if (yameruModel) {
		 float scaleMultiplier = 1.0f + (kSelectedScaleMax - 1.0f) * easedProgress;
			float newScale = selectedModelInitialScale_ * scaleMultiplier;
			yameruModel->GetTransform().scale = { newScale, newScale, newScale };
		}

		// 非選択モデル（Start）のフェードアウト（α値を下げて透明に）
		auto* startModel = titleUI_->GetStartModel();
		if (startModel && startModel->GetModel()) {
			auto* materialManager = startModel->GetModel()->GetMaterialManager();
			if (materialManager) {
				// α値を徐々に下げて透明にする（RGB成分は維持）
				float alpha = 1.0f - progress;
				Vector4 fadedColor = {
					unselectedModelOriginalColor_.x,
					unselectedModelOriginalColor_.y,
					unselectedModelOriginalColor_.z,
					alpha
				};
				materialManager->SetColor(fadedColor);
			}
		}
	}

	// 雷エフェクトの点滅演出（0.1秒間隔で高速点滅）
	if (progress < 0.5f) {
		static float flickerTimer = 0.0f;
		flickerTimer += deltaTime;
		if (flickerTimer >= 0.05f) {
			flickerTimer = 0.0f;
			static bool flickerOn = false;
			flickerOn = !flickerOn;
			for (int i = 0; i < 4; i++) {
				if (frameEffectIds_[i] >= 0) {
					lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], flickerOn);
				}
			}
		}
	}

	// 演出が終了したら
	if (confirmAnimationTimer_.IsFinished()) {
		isConfirmAnimating_ = false;

		// 雷エフェクトを非表示に
		for (int i = 0; i < 4; ++i) {
			if (frameEffectIds_[i] >= 0) {
				lightningManager_->SetEffectVisibleImmediate(frameEffectIds_[i], false);
			}
		}

		// 選択状態に応じた処理
		if (titleUI_->GetSelectionState() == TitleUI::SelectionState::Start) {
			// スタート選択時はシーン遷移を開始
		 isTransitioning_ = true;
			transitionTimer_ = 0.0f;
		} else if (titleUI_->GetSelectionState() == TitleUI::SelectionState::Quit) {
			// やめる選択時は待機時間を開始
			isWaitingForQuit_ = true;
			quitWaitTimer_ = 0.0f;
		}
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
