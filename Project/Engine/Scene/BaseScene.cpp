#include "BaseScene.h"
#include "EngineSystem/EngineSystem.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/Debug/DebugCamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "Engine/Camera/Camera2D.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Light/LightManager.h"
#include "Engine/Graphics/Render/RenderManager.h"
#include "Engine/Graphics/LineRenderer.h"
#include "Engine/Graphics/Render/Line/LineRendererPipeline.h"
#include "Engine/Particle/ParticleSystem.h"
#include "WinApp/WinApp.h"
#include "Object3d.h"
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

	// LineRendererPipelineを取得
	auto renderManager = engine_->GetComponent<RenderManager>();
	if (renderManager) {
		auto* lineRendererPipeline = static_cast<class LineRendererPipeline*>(
			renderManager->GetRenderer(RenderPassType::Line));
		if (lineRendererPipeline) {
			// LineRendererを作成
			auto lineRenderer = std::make_unique<class LineRenderer>();
			lineRenderer->Initialize(lineRendererPipeline);
			lineRenderer_ = lineRenderer.get();
			gameObjects_.push_back(std::move(lineRenderer));
		}
	}
}

void BaseScene::Update()
{
	// 前フレームで削除可能になったオブジェクトを削除（描画コマンド実行後）
	CleanupGameObjects();

	// KeyboardInput を直接取得
	auto keyboard = engine_->GetComponent<KeyboardInput>();
	if (!keyboard) {
		return; // キーボードは必須
	}
#ifdef _DEBUG

	// デバッグカメラへの切り替え
	if (keyboard->IsKeyTriggered(DIK_F1)) {
		cameraManager_->SetActiveCamera("Debug", CameraType::Camera3D);
	} else if (keyboard->IsKeyTriggered(DIK_F2)) {
		cameraManager_->SetActiveCamera("Release", CameraType::Camera3D);

	}

#endif // _DEBUG


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
	// ゲームオブジェクトのImGuiデバッグUI表示
	DrawGameObjectsImGui();
#endif

	// ゲームオブジェクトの更新
	UpdateGameObjects();

#ifdef _DEBUG
	// エミッター形状のデバッグ描画
	if (lineRenderer_) {
		lineRenderer_->Clear();  // 毎フレームクリア

		ICamera* activeCamera3D = cameraManager_->GetActiveCamera(CameraType::Camera3D);
		if (activeCamera3D) {
			// 全てのParticleSystemの形状を描画
			for (const auto& obj : gameObjects_) {
				if (auto* particleSystem = dynamic_cast<ParticleSystem*>(obj.get())) {
					// ShapeModuleからデバッグ描画
					auto& shapeModule = particleSystem->GetShapeModule();
					if (shapeModule.IsDebugDrawEnabled()) {
						shapeModule.DrawEmitterShape(
							lineRenderer_,
							activeCamera3D,
							particleSystem->GetEmitterPosition()
						);
					}
				}
			}
		}
	}
#endif
}

void BaseScene::Draw()
{
	auto renderManager = engine_->GetComponent<RenderManager>();
	auto dxCommon = engine_->GetComponent<DirectXCommon>();
	ICamera* activeCamera3D = cameraManager_->GetActiveCamera(CameraType::Camera3D);

	if (!renderManager || !dxCommon || !activeCamera3D) {
		return;
	}

	ID3D12GraphicsCommandList* cmdList = dxCommon->GetCommandList();

	// ===== RenderManagerによる統一描画システム =====
	// カメラマネージャーを設定（タイプ別カメラを自動選択）
	renderManager->SetCameraManager(cameraManager_.get());
	renderManager->SetCommandList(cmdList);

	// 全てのゲームオブジェクトを描画キューに追加（子オブジェクトも再帰的に追加）
	for (const auto& obj : gameObjects_) {
		if (obj && obj->IsActive()) {
			AddDrawableRecursive(renderManager, obj.get());
		}
	}

	// 一括描画（自動的にパスごとにソート・グループ化）
	renderManager->DrawAll();

	// フレーム終了時にキューをクリア
	renderManager->ClearQueue();

	// デバッグ描画（派生クラスでオーバーライド可能）
	DrawDebug();

	// オブジェクトのクリーンアップは次のフレームのUpdate()の最初に移動
	// これにより、描画コマンド実行後に削除が行われる
}

void BaseScene::Finalize()
{
	// ゲームオブジェクトをクリア
	gameObjects_.clear();
}

void BaseScene::SetupCamera()
{
	auto dxCommon = engine_->GetComponent<DirectXCommon>();
	if (!dxCommon) {
		return;
	}

	// カメラマネージャーを作成
	cameraManager_ = std::make_unique<CameraManager>();

	// ===== 3Dカメラの設定=====

	// リリースカメラを作成して登録（斜め上から俯瞰する視点）
	auto releaseCamera = std::make_unique<Camera>();
	releaseCamera->Initialize(dxCommon->GetDevice());
	
	// 派生クラスでカスタマイズ可能なパラメータ設定
	SetupReleaseCameraParameters(releaseCamera.get());

	cameraManager_->RegisterCamera("Release", std::move(releaseCamera));

	// デバッグカメラを作成して登録
	auto debugCamera = std::make_unique<DebugCamera>();
	debugCamera->Initialize(engine_, dxCommon->GetDevice());
	cameraManager_->RegisterCamera("Debug", std::move(debugCamera));

#ifdef _DEBUG
	// デバッグビルドではデバッグカメラをアクティブに設定
	cameraManager_->SetActiveCamera("Release", CameraType::Camera3D);
#else
	// リリースビルドではリリースカメラをアクティブに設定
	cameraManager_->SetActiveCamera("Release", CameraType::Camera3D);
#endif

	// ===== 2Dカメラの設定 =====

	// 2Dカメラを作成して登録（スクリーンサイズは自動取得）
	auto camera2D = std::make_unique<Camera2D>();
	// 2Dカメラの初期位置：画面中央
	camera2D->SetPosition(Vector2{ 0.0f, 0.0f });
	camera2D->SetZoom(1.0f);

	cameraManager_->RegisterCamera("Camera2D", std::move(camera2D));

	// 2Dカメラをアクティブに設定
	cameraManager_->SetActiveCamera("Camera2D", CameraType::Camera2D);
}

void BaseScene::SetupReleaseCameraParameters(Camera* camera)
{
	// デフォルトのカメラパラメータ（GameScene用の斜め上から俯瞰する視点）
	camera->SetTranslate({ 0.0f, 12.0f, -15.0f });
	camera->SetRotate({ 0.6f, 0.0f, 0.0f });
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

void BaseScene::UpdateGameObjects()
{
	// 全ゲームオブジェクトの更新
	for (auto& obj : gameObjects_) {
		if (obj && obj->IsActive()) {
			obj->Update();
		}
	}
	// 削除処理はDrawの後に移動（CleanupGameObjects()で実行）
}

void BaseScene::CleanupGameObjects()
{
	// 削除可能なオブジェクトを削除（メモリリーク防止・パフォーマンス維持）
	// この処理は描画完了後に実行されるため、ダングリングポインタの問題を回避
	gameObjects_.erase(
		std::remove_if(gameObjects_.begin(), gameObjects_.end(),
			[](const std::unique_ptr<IDrawable>& obj) {
				if (!obj) return true;
				
				// IDrawable::CanBeDeleted()をチェック
				// 非アクティブで削除可能なオブジェクトを削除
				return obj->CanBeDeleted();
			}),
		gameObjects_.end()
	);
}

void BaseScene::DrawGameObjectsImGui()
{
#ifdef _DEBUG
	if (ImGui::Begin("オブジェクト制御")) {
		// 全オブジェクトのImGuiデバッグUI（非アクティブも表示）
		for (auto& obj : gameObjects_) {
			if (obj) {
				obj->DrawImGui();
			}
		}

		ImGui::Separator();
	}
	ImGui::End();
#endif // _DEBUG
}

void BaseScene::DrawDebug()
{

}

void BaseScene::AddDrawableRecursive(RenderManager* renderManager, IDrawable* drawable) {
	if (!drawable || !drawable->IsActive()) {
		return;
	}

	// 現在のオブジェクトを描画キューに追加
	renderManager->AddDrawable(drawable);

	// Object3dの場合は子オブジェクトも再帰的に追加
	if (auto* obj3d = dynamic_cast<Object3d*>(drawable)) {
		for (const auto& child : obj3d->GetChildren()) {
			if (child && child->IsActive()) {
				AddDrawableRecursive(renderManager, child.get());
			}
		}
	}
}
