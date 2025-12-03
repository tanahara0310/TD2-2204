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
#include "Engine/WinApp/WinApp.h"
#include "MathCore.h"


void TitleScene::Initialize(EngineSystem* engine) {
	BaseScene::Initialize(engine);

	// 雷エフェクトマネージャーの初期化
	{
		auto modelManager = engine->GetComponent<ModelManager>();
		auto& textureManager = TextureManager::GetInstance();

		lightningManager_ = std::make_unique<LightningEffectManager>();
		lightningManager_->Initialize(modelManager, &textureManager);
	}

	// ゲームオブジェクトの初期化
	{
		// 雷エフェクト1本目（左側・青色）
		LightningEffectManager::LinearEffectConfig config1;
		config1.startOffset = { 0.0f, 5.0f, 0.0f };
		config1.endOffset = { 0.0f, 0.0f, 0.0f };
		config1.segmentCount = 15;
		config1.noiseScale = 0.6f;
		config1.noiseSpeed = 1.5f;
		config1.enableAnimation = true;
		config1.color = { 0.3f, 0.5f, 1.0f, 1.0f };
		config1.pathType = Lightning::PathType::Linear;
		lightningManager_->CreateLinearEffectAtPosition({ -3.0f, 0.0f, 0.0f }, config1, gameObjects_);

		// 雷エフェクト2本目（中央・赤色）
		LightningEffectManager::LinearEffectConfig config2;
		config2.startOffset = { 0.0f, 5.0f, 0.0f };
		config2.endOffset = { 0.0f, 0.0f, 0.0f };
		config2.segmentCount = 20;
		config2.noiseScale = 0.4f;
		config2.noiseSpeed = 2.5f;
		config2.enableAnimation = true;
		config2.color = { 1.0f, 0.3f, 0.3f, 1.0f };
		config2.pathType = Lightning::PathType::Linear;
		lightningManager_->CreateLinearEffectAtPosition({ 0.0f, 0.0f, 0.0f }, config2, gameObjects_);

		// 雷エフェクト3本目（右側・緑色）
		LightningEffectManager::LinearEffectConfig config3;
		config3.startOffset = { 0.0f, 5.0f, 0.0f };
		config3.endOffset = { 0.0f, 0.0f, 0.0f };
		config3.segmentCount = 12;
		config3.noiseScale = 0.8f;
		config3.noiseSpeed = 1.0f;
		config3.enableAnimation = true;
		config3.color = { 0.3f, 1.0f, 0.5f, 1.0f };
		config3.pathType = Lightning::PathType::Linear;
		lightningManager_->CreateLinearEffectAtPosition({ 3.0f, 0.0f, 0.0f }, config3, gameObjects_);
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

	// 電気パーティクルエフェクトを作成
	CreateElectricParticleEffect();
}

void TitleScene::Update() {
	BaseScene::Update();

	// 雷エフェクトの更新
	if (lightningManager_) {
		lightningManager_->UpdateAllEffects();
	}

	auto input = engine_->GetComponent<KeyboardInput>();
	if (!input || !titleUI_) {
		return;
	}

	// electricParticle_が有効かチェック（gameObjects_内に存在するか）
	bool isParticleValid = false;
	if (electricParticle_) {
		for (const auto& obj : gameObjects_) {
			if (obj.get() == electricParticle_) {
				isParticleValid = true;
				break;
			}
		}
		if (!isParticleValid) {
			electricParticle_ = nullptr; // 削除されていたらnullptrに
		}
	}

	// 遷移中でなければ入力を受け付ける
	if (!isTransitioning_) {
		// 上キーでスタートを選択
		if (input->IsKeyTriggered(DIK_UP)) {
			titleUI_->SetSelectionState(TitleUI::SelectionState::Start);
		}

		// 下キーでQuitを選択
		if (input->IsKeyTriggered(DIK_DOWN)) {
			titleUI_->SetSelectionState(TitleUI::SelectionState::Quit);
		}

		// スペースキーで決定
		if (input->IsKeyTriggered(DIK_SPACE)) {
			switch (titleUI_->GetSelectionState()) {
			case TitleUI::SelectionState::Start:
				// 遷移開始
				isTransitioning_ = true;
				transitionTimer_ = 0.0f;

				// 電気パーティクルを画面中央で再生
				if (electricParticle_ && isParticleValid) {
					electricParticle_->SetActive(true); // アクティブ化
					electricParticle_->SetEmitterPosition({ 0.0f, 0.0f, 0.0f });
					electricParticle_->Clear(); // 前回のパーティクルをクリア
					electricParticle_->Play(); // 再生開始
				}
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
		UpdateSceneTransition(1.0f / 60.0f); // 仮のデルタタイム
	}

	// UI更新（ステートマシーンも含む）
	if (titleUI_) {
		titleUI_->Update();
	}
}

void TitleScene::CreateElectricParticleEffect() {
	auto dxCommon = engine_->GetComponent<DirectXCommon>();
	auto resourceFactory = engine_->GetComponent<ResourceFactory>();

	if (!dxCommon || !resourceFactory) {
		return;
	}

	// パーティクルシステムを作成
	auto particleSystem = std::make_unique<ParticleSystem>();
	particleSystem->Initialize(dxCommon, resourceFactory);
	
	// 最初は非アクティブに設定（使用時にアクティブ化）
	particleSystem->SetActive(false);

	// テクスチャを設定（白い円のテクスチャを使用）
	particleSystem->SetTexture("Resources/SampleResources/circle.png");

	// エミッター位置を画面中央に設定
	particleSystem->SetEmitterPosition({ 0.0f, 0.0f, 0.0f });

	// ブレンドモードを加算合成に設定（光のエフェクト）
	particleSystem->SetBlendMode(BlendMode::kBlendModeAdd);

	// メインモジュールの設定
	{
		auto& mainModule = particleSystem->GetMainModule();
		auto mainData = mainModule.GetMainData();
		mainData.duration = 1.0f; // エフェクト継続時間
		mainData.looping = false; // ループなし（ワンショット）
		mainData.playOnAwake = false; // 起動時は再生しない（手動制御）
		mainData.startLifetime = 0.8f; // パーティクルの寿命
		mainData.startSpeed = 8.0f; // 初速を速く（電気の火花が飛び散る）
		mainData.startSize = { 0.3f, 0.3f, 0.3f }; // 開始サイズ
		mainData.startColor = { 0.5f, 0.8f, 1.0f, 1.0f }; // 青白い色（電気）
		mainData.maxParticles = 500; // 最大パーティクル数
		mainModule.SetMainData(mainData);
	}

	// エミッションモジュールの設定（一度に大量放出）
	{
		auto& emissionModule = particleSystem->GetEmissionModule();
		auto emissionData = emissionModule.GetEmissionData();
		emissionData.rateOverTime = 0; // 継続的な放出なし
		emissionData.burstCount = 200; // バーストで200個放出
		emissionData.burstTime = 0.0f; // 開始時に放出
		emissionModule.SetEmissionData(emissionData);
	}

	// 形状モジュールの設定（球状に放射）
	{
		auto& shapeModule = particleSystem->GetShapeModule();
		auto shapeData = shapeModule.GetShapeData();
		shapeData.shapeType = ShapeModule::ShapeType::Sphere;
		shapeData.radius = 0.1f; // 小さな球から放出
		shapeData.emitFromSurface = true; // 表面から放出
		shapeModule.SetShapeData(shapeData);
	}

	// 速度モジュールの設定（ランダムな方向）
	{
		auto& velocityModule = particleSystem->GetVelocityModule();
		auto velocityData = velocityModule.GetVelocityData();
		velocityData.useRandomDirection = true; // ランダムな方向に飛ぶ
		velocityModule.SetVelocityData(velocityData);
	}

	// サイズモジュールの設定（徐々に小さくなる）
	{
		auto& sizeModule = particleSystem->GetSizeModule();
		auto sizeData = sizeModule.GetSizeData();
		sizeData.endSize = 0.0f; // 消える時は0
		sizeData.sizeOverLifetime = true;
		sizeModule.SetSizeData(sizeData);
	}

	// 色モジュールの設定（青白→透明にフェード）
	{
		auto& colorModule = particleSystem->GetColorModule();
		auto colorData = colorModule.GetColorData();
		colorData.endColor = { 0.2f, 0.4f, 0.8f, 0.0f }; // 青→透明
		colorData.useGradient = true;
		colorModule.SetColorData(colorData);
	}

	// ポインタを保存してgameObjects_に追加
	electricParticle_ = particleSystem.get();
	gameObjects_.push_back(std::move(particleSystem));
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
