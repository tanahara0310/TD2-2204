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
#include "MathCore.h"

void TitleScene::Initialize(EngineSystem* engine) {
	BaseScene::Initialize(engine);

	// ゲームオブジェクトの初期化
	{
		auto lightning = std::make_unique<Lightning>();
		Lightning::LightningConfig config;
		config.startPoint = { 0.0f, 5.0f, 0.0f };
		config.endPoint = { 0.0f, 0.0f, 0.0f };
		config.noiseStrength = 0.5f;
		config.noiseFrequency = 2.0f;
		config.segmentCount = 10;

		lightning->Initialize(config);
		gameObjects_.push_back(std::move(lightning));


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

	auto input = engine_->GetComponent<KeyboardInput>();
	// Enterキーでゲームシーンへ切り替え
	if (input && input->IsKeyTriggered(DIK_RETURN)) {
		sceneManager_->ChangeScene("GameScene");
	}

	// UI更新
	if (titleUI_) {
		titleUI_->Update();
	}
}

void TitleScene::Draw() {
	BaseScene::Draw();
}

void TitleScene::Finalize() {
}
