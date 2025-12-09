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
#include "../../Effect/Lightning/LightningEffectManager.h"
#include "Engine/Utility/Random/RandomGenerator.h"

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
		Vector4 quitColor  = { 0.9f, 1.0f, 1.0f, 1.0f };

		// 矩形サイズ（共通）
		float halfWidth = 1.9f;
		float halfHeight = 0.5f;
		float frameYOffset = 0.8f; // 中心から少し上にずらす

		// 初期中心はStartの位置（上にオフセット）
		Vector3 startCenter = { -2.1f, -4.5f + frameYOffset, -57.6f };

		config.color = startColor;
		// 上辺（index 0）
		config.startOffset = { -halfWidth,  halfHeight, 0.0f };
		config.endOffset   = {  halfWidth,  halfHeight, 0.0f };
		frameEffectIds_[0] = lightningManager_->CreateEffect(startCenter, config, gameObjects_);
		// 下辺（index 1）
		config.startOffset = { -halfWidth, -halfHeight, 0.0f };
		config.endOffset   = {  halfWidth, -halfHeight, 0.0f };
		frameEffectIds_[1] = lightningManager_->CreateEffect(startCenter, config, gameObjects_);
		// 左辺（index 2）
		config.startOffset = { -halfWidth, -halfHeight, 0.0f };
		config.endOffset   = { -halfWidth,  halfHeight, 0.0f };
		frameEffectIds_[2] = lightningManager_->CreateEffect(startCenter, config, gameObjects_);
		// 右辺（index 3）
		config.startOffset = {  halfWidth, -halfHeight, 0.0f };
		config.endOffset   = {  halfWidth,  halfHeight, 0.0f };
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
		float frameYOffset = 0.5f; // 中心から少し上にずらす
		Vector3 startCenter = { -2.1f, -4.5f + frameYOffset, -57.6f };
		Vector3 quitCenter  = { -2.4f, -5.9f + frameYOffset, -57.6f };
		Vector3 target = isStartSelected ? startCenter : quitCenter;

		for (int i = 0; i < 4; ++i) {
			if (frameEffectIds_[i] >= 0) {
				lightningManager_->SetEffectPosition(frameEffectIds_[i], target);
			}
		}

		if (lastIsStartSelected_ != isStartSelected) {
			Vector4 startColor = { 0.6f, 0.9f, 1.0f, 1.0f };
			Vector4 quitColor  = { 0.9f, 1.0f, 1.0f, 1.0f };
			Vector4 color = isStartSelected ? startColor : quitColor;
			for (int i = 0; i < 4; ++i) {
				if (frameEffectIds_[i] >= 0) {
					lightningManager_->SetEffectColor(frameEffectIds_[i], color);
				}
			}
			lastIsStartSelected_ = isStartSelected;
		}
	}

	// 選択中に一定間隔で一瞬だけ表示するパルス制御（ランダムな2辺を雷っぽく点滅）
	{
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
			for (int i = 0; i < 4; ++i) {
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
				for (int i = 0; i < 4; ++i) {
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
