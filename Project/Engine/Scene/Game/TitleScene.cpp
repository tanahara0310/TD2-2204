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
	engine_ = engine;

	// コンポーネントを直接取得
	auto dxCommon = engine_->GetComponent<DirectXCommon>();
	if (!dxCommon) {
		return; // 必須コンポーネントがない場合は終了
	}

	// カメラマネージャーの初期化とカメラの登録
	{
		// リリースカメラを作成して登録
		auto releaseCamera = std::make_unique<Camera>();
		releaseCamera->Initialize(dxCommon->GetDevice());
		cameraManager_->RegisterCamera("Release", std::move(releaseCamera));

		// デバッグカメラを作成して登録（デバイスを渡す）
		auto debugCamera = std::make_unique<DebugCamera>();
		debugCamera->Initialize(engine_, dxCommon->GetDevice());
		cameraManager_->RegisterCamera("Debug", std::move(debugCamera));

		// デフォルトでデバッグカメラをアクティブに設定
		cameraManager_->SetActiveCamera("Debug");

#ifdef _DEBUG
		auto console = engine_->GetConsole();
		if (console) {
			console->LogInfo("TitleScene: カメラマネージャーを初期化しました");
		}
#endif
	}

	// このシーン専用のディレクショナルライトを作成
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

#ifdef _DEBUG
	auto console = engine_->GetConsole();
	if (console) {
		console->LogInfo("TitleScene: 初期化完了");
	}
#endif
}

void TitleScene::Update() {
	// カメラマネージャーの更新
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
	cameraManager_->DrawImGui();
#endif
}

void TitleScene::Draw() {
	// コンポーネント取得
	auto renderManager = engine_->GetComponent<RenderManager>();
	auto dxCommon = engine_->GetComponent<DirectXCommon>();
	ICamera* activeCamera = cameraManager_->GetActiveCamera();

	// 必須コンポーネントのチェック
	if (!renderManager || !dxCommon || !activeCamera) {
		return;
	}

	// ===== RenderManagerによる統一描画システム =====
	if (renderManager) {
		// フレーム開始時に描画コンテキストを設定（1回のみ）
		renderManager->SetCamera(activeCamera);
		renderManager->SetCommandList(dxCommon->GetCommandList());
		
		// 一括描画（自動的にパスごとにソート・グループ化）
		renderManager->DrawAll();
		
		// フレーム終了時にキューをクリア
		renderManager->ClearQueue();
	}
}

void TitleScene::Finalize() {

	// このシーン専用のライトを削除
	directionalLight_ = nullptr;
	
	engine_ = nullptr;
}
