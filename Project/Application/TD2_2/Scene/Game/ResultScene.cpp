#include "ResultScene.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/Debug/DebugCamera.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Light/LightData.h"
#include "Engine/Graphics/Light/LightManager.h"
#include "Engine/Graphics/Render/RenderManager.h"
#include "Engine/Input/KeyboardInput.h"
#include "EngineSystem/EngineSystem.h"
#include "MathCore.h"
#include "Scene/SceneManager.h"

void ResultScene::Initialize(EngineSystem* engine) {
	BaseScene::Initialize(engine);

	InputSource::Initialize(engine);

	auto modelManager = engine_->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();

	// 勝敗をJsonから取得
	json resultData = JsonManager::GetInstance().LoadJson("Resources/Data/result.json");

	// "isWin"キーから勝敗情報を取得、存在しない場合は負け
	isWin_ = JsonManager::SafeGet<bool>(resultData, "isWin", false);

	// クリアタイムマネージャーの生成
	clearTimeManager_ = std::make_unique<ClearTimeManager>("Resources/ClearTimes/ClearTimes.txt");

	// クリアタイムの読み込み
	clearTimeManager_->LoadTimes();

	// 勝利時のみクリアタイムを登録
	if (isWin_) {
		// 今回のクリアタイムをJsonから取得
		json clearTimeData = JsonManager::GetInstance().LoadJson("Resources/Data/CurrentClearTime.json");

		// "CurrentClearTime"キーからクリアタイムを取得、存在しない場合はデフォルト値の9999.999を使用
		currentClearTime_ = JsonManager::SafeGet<float>(clearTimeData, "CurrentClearTime", 9999.999f);

		// クリアタイムの登録
		clearTimeManager_->RegisterTime(currentClearTime_);
	}

	// 上位3つのクリアタイムを取得
	for (int i = 0; i < 3; ++i) {
		clearTimes_[i] = clearTimeManager_->GetTimes()[i];
	}

	// クリアタイマーを文字列に変換
	timerDigits_ = FormatTime(currentClearTime_);

	// resultUI
	{
		// UI初期化
		resultUI_ = std::make_unique<ResultUI>();
		auto sprites = resultUI_->Initialize(engine, isWin_);

		// スプライトをgameObjects_に追加
		for (auto& sprite : sprites) {
			gameObjects_.push_back(std::move(sprite));
		}

		if (isWin_) {
			// タイマー文字列を受け取る
			resultUI_->SetTimerString(timerDigits_);
		}

		// クリアタイム上位三つを文字列に変換
		resultUI_->SetRankTimeStrings({FormatTime(clearTimes_[0]), FormatTime(clearTimes_[1]), FormatTime(clearTimes_[2])});
	}

	// KeyConfigの設定
	{
		// 上方向の入力（キーボード上キー or ゲームパッドの十字キー上）
		ActionBuilder(keyConfig_.AddAction("Right", ActionType::Bool)).BindKey(DIK_D).BindGamepadButton(GamepadButton::DPadRight).BindGamepadAxisBool(AxisComponent::X, true);

		// 下方向の入力（キーボード下キー or ゲームパッドの十字キー下）
		ActionBuilder(keyConfig_.AddAction("Left", ActionType::Bool)).BindKey(DIK_A).BindGamepadButton(GamepadButton::DPadLeft).BindGamepadAxisBool(AxisComponent::X, false);

		// 決定ボタン（キーボードスペース or ゲームパッドAボタン）
		ActionBuilder(keyConfig_.AddAction("Confirm", ActionType::Bool)).BindKey(DIK_SPACE).BindGamepadButton(GamepadButton::A);

		// 上下方向の移動入力（Vector2として取得）
		ActionBuilder(keyConfig_.AddAction("Move", ActionType::Vector2)).BindKeyboardWASD(DIK_W, DIK_S, DIK_A, DIK_D).BindGamepadLeftStick();
	}

	// 背景の生成と初期化
	{
		auto backgroundModel = modelManager->CreateStaticModel("Resources/Models/Background/Background2.obj");
		auto backgroundTexture = textureManager.Load("Resources/Textures/Background2.png");
		auto background = std::make_unique<Background>();
		background_ = background.get();
		background->Initialize(std::move(backgroundModel), backgroundTexture);
		gameObjects_.push_back(std::move(background));
	}

	{
		// サウンドリソースを取得
		auto soundManager = engine_->GetComponent<SoundManager>();
		if (soundManager) {
			mp3Resource_ = soundManager->CreateSoundResource("Resources/Audio/BGM/Funky_Magic.mp3");
			cursorSound_ = soundManager->CreateSoundResource("Resources/Audio/SE/cursor.mp3");
			decideSound_ = soundManager->CreateSoundResource("Resources/Audio/SE/decide.mp3");

			cursorSound_->SetVolume(1.0f);
			decideSound_->SetVolume(1.0f);
		}

		if (mp3Resource_ && mp3Resource_->IsValid()) {
			bool isPlaying = mp3Resource_->IsPlaying();
			if (!isPlaying) {
				mp3Resource_->Play(false);
			}
		}
	}

	// 雲
	{
		for (int i = 0; i < clouds_.size(); i++) {
			auto cloudModel = modelManager->CreateStaticModel("Resources/Models/Cloud/Cloud.obj");
			auto cloudTexture = textureManager.Load("Resources/Textures/Cloud.png");
			auto cloud = std::make_unique<Cloud>();
			clouds_[i] = cloud.get();
			cloud->Initialize(std::move(cloudModel), cloudTexture);
			gameObjects_.push_back(std::move(cloud));
		}
	}

	// パーティクル
	if (isWin_) {
		auto fireworkParticle = CreateParticleSystem("Resources/Presets/Particle/Firework.json");
		fireworkParticle_ = fireworkParticle.get();
		gameObjects_.push_back(std::move(fireworkParticle));
		EmitParticle(fireworkParticle_, {0.0f, 0.0f, 0.0f});
	}
}

void ResultScene::Update() {
	BaseScene::Update();

	if (!resultUI_) {
		return;
	}

	// クールダウンタイマーを減少
	if (stickInputCooldown_ > 0.0f) {
		stickInputCooldown_ -= GameUtils::GetDeltaTime();
	}

	// 遷移中でなければ入力を受け付ける
	if (!isTitleTransitioning_ && !isGameTransitioning_) {
		bool selectionChanged = false;
		// 右キーでスタートを選択
		if (keyConfig_.GetDown("Right")) {
			// SE再生
			if (cursorSound_ && cursorSound_->IsValid() && resultUI_->GetSelectionState() == ResultUI::SelectionState::ReStart) {
				cursorSound_->Play(false);
			}

			selectionChanged = true;
			resultUI_->SetSelectionState(ResultUI::SelectionState::ToTitle);
		}

		// 左キーでQuitを選択
		if (keyConfig_.GetDown("Left")) {
			// SE再生
			if (cursorSound_ && cursorSound_->IsValid() && resultUI_->GetSelectionState() == ResultUI::SelectionState::ToTitle) {
				cursorSound_->Play(false);
			}

			selectionChanged = true;
			resultUI_->SetSelectionState(ResultUI::SelectionState::ReStart);
		}

		// スペースキーで決定
		if (keyConfig_.GetDown("Confirm")) {
			if (mp3Resource_ && mp3Resource_->IsValid()) {
				bool isPlaying = mp3Resource_->IsPlaying();
				if (!isPlaying) {
					mp3Resource_->Stop();
				}
			}

			// SE再生
			if (decideSound_ && decideSound_->IsValid()) {
				decideSound_->Play(false);
			}

			switch (resultUI_->GetSelectionState()) {
			case ResultUI::SelectionState::ToTitle:
				// タイトル遷移開始
				isTitleTransitioning_ = true;
				transitionTimer_ = 0.0f;

				// アニメーションフラグを立てる
				resultUI_->SetIsAnimationToTitle(true);
				break;
			case ResultUI::SelectionState::ReStart:
				// リスタート遷移開始
				isGameTransitioning_ = true;
				resultUI_->SetIsScaleAnimationReStart(true);
				transitionTimer_ = 0.0f;

				// アニメーションフラグを立てる
				resultUI_->SetIsAnimationReStart(true);
				break;
			}
		}

		// スティック入力での選択（クールダウン中でなければ）
		if (!selectionChanged && stickInputCooldown_ <= 0.0f) {
			Vector2 moveInput = keyConfig_.Get<Vector2>("Move");

			// 左方向（X軸負）
			if (moveInput.x < -kStickThreshold) {
				// SE再生
				if (cursorSound_ && cursorSound_->IsValid() && resultUI_->GetSelectionState() == ResultUI::SelectionState::ToTitle) {
					cursorSound_->Play(false);
				}

				resultUI_->SetSelectionState(ResultUI::SelectionState::ReStart);
				stickInputCooldown_ = kStickInputDelay;
			}
			// 右方向（X軸正）
			else if (moveInput.x > kStickThreshold) {
				// SE再生
				if (cursorSound_ && cursorSound_->IsValid() && resultUI_->GetSelectionState() == ResultUI::SelectionState::ReStart) {
					cursorSound_->Play(false);
				}

				resultUI_->SetSelectionState(ResultUI::SelectionState::ToTitle);
				stickInputCooldown_ = kStickInputDelay;
			}
		}
	}

	// 遷移処理
	if (isTitleTransitioning_ || isGameTransitioning_) {
		UpdateSceneTransition(GameUtils::GetDeltaTime());
	}

	// リザルト画像の更新
	if (resultUI_)
		resultUI_->Update();

	// パーティクルの座標更新
	if (isWin_) {
		Vector3 emitPos;
		emitPos.x = RandomGenerator::GetInstance().GetFloat(-20.0f, 20.0f);
		emitPos.y = RandomGenerator::GetInstance().GetFloat(-8.0f, 13.0f);
		emitPos.z = 0.0f;

		fireworkParticle_->SetEmitterPosition(emitPos);

		CheckParticleAutoDeactivate(fireworkParticle_);
	}
}

void ResultScene::UpdateSceneTransition(float deltaTime) {
	transitionTimer_ += deltaTime;

	// 遷移時間が経過したらシーン遷移
	if (transitionTimer_ >= kTransitionDuration) {
		if (isTitleTransitioning_) {
			sceneManager_->ChangeScene("TitleScene");
		} else if (isGameTransitioning_) {
			sceneManager_->ChangeScene("GameScene");
		}
	}
}

void ResultScene::Draw() { BaseScene::Draw(); }

void ResultScene::Finalize() {}

std::array<int, 6> ResultScene::FormatTime(float time) {
	int totalMilliSeconds = static_cast<int>(time * 1000);

	int minutes = (totalMilliSeconds / 1000) / 60;
	int seconds = (totalMilliSeconds / 1000) % 60;
	int milliseconds = totalMilliSeconds % 1000;

	int m1 = minutes / 10;
	int m2 = minutes % 10;
	int s1 = seconds / 10;
	int s2 = seconds % 10;
	int ms1 = (milliseconds / 100) % 10;
	int ms2 = (milliseconds % 10) % 10;

	return {m1, m2, s1, s2, ms1, ms2};
}

void ResultScene::SetupReleaseCameraParameters(Camera* camera) {
	// リザルトシーン専用のカメラパラメータ
	// より引きの視点で全体を見渡せるように設定
	camera->SetTranslate({0.0f, 0.0f, -70.0f});
	camera->SetRotate({0.0f, 0.0f, 0.0f});
}

std::unique_ptr<ParticleSystem> ResultScene::CreateParticleSystem(const std::string& presetPath) {
	auto dxCommon = engine_->GetComponent<DirectXCommon>();
	auto resourceFactory = engine_->GetComponent<ResourceFactory>();
	auto modelManager = engine_->GetComponent<ModelManager>();

	// ModelResourceを取得（必要に応じてモデルを読み込む）
	auto* voxelModelResource = modelManager->GetModelResource("Resources/Models/Voxel/Voxel.obj");
	if (!voxelModelResource) {
		modelManager->LoadModelResource("Resources/Models/Voxel", "Voxel.obj");
		voxelModelResource = modelManager->GetModelResource("Resources/Models/Voxel/Voxel.obj");
	}

	// パーティクルシステムを作成
	auto particleSystem = std::make_unique<ParticleSystem>();
	particleSystem->Initialize(dxCommon, resourceFactory);

	if (voxelModelResource) {
		particleSystem->SetModelResource(voxelModelResource);
	}

	particleSystem->SetTexture("Resources/SampleResources/white1x1.png");

	// プリセットファイルから設定を読み込む
	ParticlePresetManager presetManager;
	presetManager.LoadPreset(particleSystem.get(), presetPath);

	// 初期状態を非アクティブに設定
	particleSystem->SetActive(false);

	return particleSystem;
}

void ResultScene::EmitParticle(ParticleSystem* particleSystem, const Vector3& position) {
	if (!particleSystem) {
		return;
	}

	particleSystem->SetActive(true);
	particleSystem->SetEmitterPosition(position);
	particleSystem->Clear();
	particleSystem->GetMainModule().Restart();
	particleSystem->Play();
}

void ResultScene::CheckParticleAutoDeactivate(ParticleSystem* particleSystem) {
	if (!particleSystem || !particleSystem->IsActive()) {
		return;
	}

	if (particleSystem->IsFinished()) {
		particleSystem->SetActive(false);
	}
}