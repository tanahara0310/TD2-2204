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
#include "Engine/Utility/Random/RandomGenerator.h"
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

	// 雷エフェクトマネージャーの初期化
	{
		auto modelManager = engine_->GetComponent<ModelManager>();
		auto& textureManager = TextureManager::GetInstance();
		
		lightningManager_ = std::make_unique<LightningEffectManager>();
		lightningManager_->Initialize(modelManager, &textureManager);
		
		// 初回の雷発生タイミングをランダムに設定
		SetRandomLightningInterval();
		lightningIntervalTimer_.Start(lightningIntervalTimer_.GetDuration(), false);
	}

	{
		// UI初期化
		titleUI_ = std::make_unique<TitleUI>();
		auto sprites = titleUI_->Initialize(engine);

		// スプライトをgameObjects_に追加
		for (auto& sprite : sprites) {
			gameObjects_.push_back(std::move(sprite));
		}
		
		// StartModel用のライトニングエフェクトを作成
		if (titleUI_->GetStartModel()) {
			LightningEffectManager::EffectConfig startEffectConfig;
			startEffectConfig.useSphereDistribution = true;
			startEffectConfig.sphereRadius = 2.0f;
			startEffectConfig.sphereStartRadiusRatio = 0.5f;
			startEffectConfig.randomOffsetRange = 0.8f; // オフセット範囲をさらに拡大
			startEffectConfig.lightningCount = 3;
			startEffectConfig.color = { 0.3f, 0.9f, 1.0f, 1.0f };
			startEffectConfig.noiseScale = 0.8f;
			startEffectConfig.noiseSpeed = 20.0f; // アニメーション速度を上げる
			startEffectConfig.segmentCount = 10; // セグメント数を少し減らす
			startEffectConfig.voxelScale = { 0.8f, 0.8f, 0.8f };
			startEffectConfig.fadeInDuration = 0.05f; // フェードインを極めて速く
			startEffectConfig.fadeOutDuration = 0.1f; // フェードアウトも速く
			
			startLightningEffectId_ = lightningManager_->CreateEffect(
				titleUI_->GetStartModel()->GetWorldPosition(),
				startEffectConfig,
				gameObjects_
			);
		}
		
		// YameruModel用のライトニングエフェクトを作成
		if (titleUI_->GetYameruModel()) {
			LightningEffectManager::EffectConfig yameruEffectConfig;
			yameruEffectConfig.useSphereDistribution = true;
			yameruEffectConfig.sphereRadius = 2.0f;
			yameruEffectConfig.sphereStartRadiusRatio = 0.5f;
			yameruEffectConfig.randomOffsetRange = 0.8f; // オフセット範囲をさらに拡大
			yameruEffectConfig.lightningCount = 3;
			yameruEffectConfig.color = { 0.3f, 0.9f, 1.0f, 1.0f };
			yameruEffectConfig.noiseScale = 0.8f;
			yameruEffectConfig.noiseSpeed = 20.0f; // アニメーション速度を上げる
			yameruEffectConfig.segmentCount = 10; // セグメント数を少し減らす
			yameruEffectConfig.voxelScale = { 0.8f, 0.8f, 0.8f };
			yameruEffectConfig.fadeInDuration = 0.05f; // フェードインを極めて速く
			yameruEffectConfig.fadeOutDuration = 0.1f; // フェードアウトも速く
			
			yameruLightningEffectId_ = lightningManager_->CreateEffect(
				titleUI_->GetYameruModel()->GetWorldPosition(),
				yameruEffectConfig,
				gameObjects_
			);
		}
	}

	// BGM再生
	{
		auto audio = engine_->GetComponent<SoundManager>();
		if (audio) {
			titleBGM_ = audio->CreateSoundResource("Resources/Audio/BGM/Title.mp3");
			
			if (titleBGM_ && titleBGM_->IsValid()) {
				titleBGM_->Play(true); // ループ再生
				titleBGM_->SetVolume(0.0f); // 音量調整
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
	
	// ライトニングエフェクトの更新
	UpdateLightningEffect(deltaTime);
}



void TitleScene::UpdateSceneTransition(float deltaTime) {
	transitionTimer_ += deltaTime;

	// 遷移時間が経過したらシーン遷移
	if (transitionTimer_ >= kTransitionDuration) {
		sceneManager_->ChangeScene("GameScene");
	}
}

void TitleScene::UpdateLightningEffect(float deltaTime) {
	if (!lightningManager_) {
		return;
	}
	
	// エフェクトマネージャーの更新
	lightningManager_->UpdateAllEffects();
	
	// 選択中のUIモデルの位置を更新
	if (titleUI_->GetStartModel() && startLightningEffectId_ >= 0) {
		lightningManager_->SetEffectPosition(
			startLightningEffectId_,
			titleUI_->GetStartModel()->GetWorldPosition()
		);
	}
	
	if (titleUI_->GetYameruModel() && yameruLightningEffectId_ >= 0) {
		lightningManager_->SetEffectPosition(
			yameruLightningEffectId_,
			titleUI_->GetYameruModel()->GetWorldPosition()
		);
	}
	
	// 雷が表示中の場合
	if (isLightningActive_) {
		// 表示タイマーを更新
		lightningDisplayTimer_.Update(deltaTime);
		
		// 表示時間が終了したら非表示にする
		if (lightningDisplayTimer_.IsFinished()) {
			isLightningActive_ = false;
			
			// 全ての雷を非表示
			if (startLightningEffectId_ >= 0) {
				lightningManager_->SetEffectVisible(startLightningEffectId_, false);
			}
			if (yameruLightningEffectId_ >= 0) {
				lightningManager_->SetEffectVisible(yameruLightningEffectId_, false);
			}
			
			// 次の雷発生タイミングをランダムに設定
			SetRandomLightningInterval();
			lightningIntervalTimer_.Start(lightningIntervalTimer_.GetDuration(), false);
		}
	}
	// 雷が非表示の場合
	else {
		// 出現間隔タイマーを更新
		lightningIntervalTimer_.Update(deltaTime);
		
		// 出現タイミングになったら雷を表示
		if (lightningIntervalTimer_.IsFinished()) {
			isLightningActive_ = true;
			
			// 選択中のUIモデルにのみライトニングを表示
			if (titleUI_->GetSelectionState() == TitleUI::SelectionState::Start) {
				if (startLightningEffectId_ >= 0) {
					lightningManager_->SetEffectVisible(startLightningEffectId_, true);
				}
			} else if (titleUI_->GetSelectionState() == TitleUI::SelectionState::Quit) {
				if (yameruLightningEffectId_ >= 0) {
					lightningManager_->SetEffectVisible(yameruLightningEffectId_, true);
				}
			}
			
			// 表示時間をランダムに設定して開始
			SetRandomLightningDuration();
			lightningDisplayTimer_.Start(lightningDisplayTimer_.GetDuration(), false);
		}
	}
}

void TitleScene::SetRandomLightningInterval() {
	auto& random = RandomGenerator::GetInstance();
	float interval = random.GetFloat(kLightningIntervalMin, kLightningIntervalMax);
	lightningIntervalTimer_.SetDuration(interval);
}

void TitleScene::SetRandomLightningDuration() {
	// 表示時間は固定（0.05秒の一瞬の閃光）
	lightningDisplayTimer_.SetDuration(kLightningDisplayDuration);
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
