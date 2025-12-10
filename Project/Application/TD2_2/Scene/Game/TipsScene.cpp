#include "TipsScene.h"
#include "EngineSystem/EngineSystem.h"
#include "Scene/SceneManager.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "Engine/ObjectCommon/SpriteObject.h"

#include <string>
#include <Application/TD2_2/Utility/GameUtils.h>

void TipsScene::Initialize(EngineSystem* engine) {
	BaseScene::Initialize(engine);

	// ゲームユーティリティの初期化
	GameUtils::Initialize(engine_);

	// 背景スプライトの作成
	{
		auto sprite = std::make_unique<SpriteObject>();
		sprite->Initialize("Resources/SampleResources/white1x1.png");
		
		// 画面全体を覆うように大きくスケール
		sprite->GetTransform().translate = { 0.0f, 0.0f, 0.0f };
		sprite->GetTransform().scale = { 1280.0f, 720.0f, 1.0f }; // 画面サイズに合わせる
		sprite->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		
		gameObjects_.push_back(std::move(sprite));
	}

	// Tipsスプライトの作成
	{
		auto sprite = std::make_unique<SpriteObject>();
		sprite->Initialize("Resources/Textures/Tips/Tips.png");

		// 画面全体を覆うように大きくスケール
		sprite->GetTransform().translate = {0.0f, 90.0f, 0.0f};
		sprite->GetTransform().scale = {1.0f, 1.0f, 1.0f};
		sprite->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

		gameObjects_.push_back(std::move(sprite));
	}

	// 文字列スプライトの作成
	{
		for (int i = 1; i <= tipsSprite_.size();i++) {
			auto sprite = std::make_unique<SpriteObject>();
			sprite->Initialize("Resources/Textures/Tips/Tips" + std::to_string(i) + ".png");

			// 画面全体を覆うように大きくスケール
			sprite->GetTransform().translate = {0.0f, -65.0f, 0.0f};
			sprite->GetTransform().scale = {0.75f, 0.75f, 1.0f};
			sprite->SetColor({1.0f, 1.0f, 1.0f, 1.0f});

			tipsSprite_[i - 1] = sprite.get();

			gameObjects_.push_back(std::move(sprite));
		}
	}

	static int tipsNum = static_cast<int>(GameUtils::RandomFloat(0.f, 5.f));

	tipsNum++;

	if (tipsNum > tipsSprite_.size() - 1) {
		tipsNum = 0;
	}

	for (int i = 0; i < tipsSprite_.size();i++) {
		if (i != tipsNum) {
			tipsSprite_[i]->SetActive(false);
		}
	}

	changeTimer_.Start(2.0f);
}

void TipsScene::Update() {
	BaseScene::Update();

	changeTimer_.Update(GameUtils::GetDeltaTime());

	if (changeTimer_.IsFinished()) {
		sceneManager_->ChangeScene("GameScene");
	}
}

void TipsScene::Draw() {
	BaseScene::Draw();
}

void TipsScene::Finalize() {
	BaseScene::Finalize();
}

void TipsScene::SetupReleaseCameraParameters(Camera* camera) {
	if (!camera) return;
	
	camera->SetTranslate({ 0.0f, 0.0f, -10.0f });
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
}
