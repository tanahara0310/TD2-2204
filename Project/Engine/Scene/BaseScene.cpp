#include "BaseScene.h"
#include "EngineSystem/EngineSystem.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/Debug/DebugCamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Light/LightManager.h"
#include <numbers>

#ifdef _DEBUG
#include <imgui.h>
#endif

void BaseScene::Initialize(EngineSystem* engine)
{
	engine_ = engine;

	//カメラ
	SetupCamera();

	//ライト
	SetupLight();

}

void BaseScene::Update()
{
	// カメラの更新
	if (cameraManager_) {
		cameraManager_->Update();
	}

	// ライトマネージャーの更新
	auto lightManager = engine_->GetComponent<LightManager>();
	if (lightManager) {
		lightManager->UpdateAll();
	}

#ifdef _DEBUG
	// カメラマネージャーのImGui
	if (cameraManager_) {
		cameraManager_->DrawImGui();
	}
#endif
}

void BaseScene::Draw()
{
}

void BaseScene::Finalize()
{

}

void BaseScene::SetupCamera()
{
	auto dxCommon = engine_->GetComponent<DirectXCommon>();
	if (!dxCommon) {
		return;
	}

	// カメラマネージャーを作成
	cameraManager_ = std::make_unique<CameraManager>();

	// リリースカメラを作成して登録（デフォルト設定）
	auto releaseCamera = std::make_unique<Camera>();
	releaseCamera->Initialize(dxCommon->GetDevice());
	releaseCamera->SetTranslate({ 0.0f, 4.0f, -10.0f });
	releaseCamera->SetRotate({ 0.26f, 0.0f, 0.0f });

	cameraManager_->RegisterCamera("Release", std::move(releaseCamera));

	// デバッグカメラを作成して登録
	auto debugCamera = std::make_unique<DebugCamera>();
	debugCamera->Initialize(engine_, dxCommon->GetDevice());
	cameraManager_->RegisterCamera("Debug", std::move(debugCamera));

	// デフォルトでリリースカメラをアクティブに設定
	cameraManager_->SetActiveCamera("Release");
}

void BaseScene::SetupLight()
{
	// デフォルトのディレクショナルライトを設定
	auto lightManager = engine_->GetComponent<LightManager>();
	if (lightManager) {
		directionalLight_ = lightManager->AddDirectionalLight();
		if (directionalLight_) {
			directionalLight_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
			directionalLight_->direction = MathCore::Vector::Normalize({ 0.0f, -1.0f, 0.5f });
			directionalLight_->intensity = 1.0f;
			directionalLight_->enabled = true;
		}
	}
}
