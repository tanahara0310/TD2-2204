#include "TipsScene.h"
#include "EngineSystem/EngineSystem.h"
#include "Scene/SceneManager.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "Engine/ObjectCommon/SpriteObject.h"

void TipsScene::Initialize(EngineSystem* engine) {
	BaseScene::Initialize(engine);

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
}

void TipsScene::Update() {
	BaseScene::Update();
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
