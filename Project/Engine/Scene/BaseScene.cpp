#include "BaseScene.h"
#include "EngineSystem/EngineSystem.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/Debug/DebugCamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Light/LightManager.h"
#include "Engine/Graphics/Render/RenderManager.h"
#include "Engine/Graphics/LineRenderer.h"
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

}

void BaseScene::Update()
{

   // KeyboardInput を直接取得
   auto keyboard = engine_->GetComponent<KeyboardInput>();
   if (!keyboard) {
	  return; // キーボードは必須
   }
#ifdef _DEBUG

   // デバッグカメラへの切り替え
   if (keyboard->IsKeyTriggered(DIK_F1)) {
	  cameraManager_->SetActiveCamera("Debug");
   } else if (keyboard->IsKeyTriggered(DIK_F2)) {
	  cameraManager_->SetActiveCamera("Release");

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
}

void BaseScene::Draw()
{
   auto renderManager = engine_->GetComponent<RenderManager>();
   auto dxCommon = engine_->GetComponent<DirectXCommon>();
   ICamera* activeCamera = cameraManager_->GetActiveCamera();

   if (!renderManager || !dxCommon || !activeCamera) {
	  return;
   }

   ID3D12GraphicsCommandList* cmdList = dxCommon->GetCommandList();

   // ===== RenderManagerによる統一描画システム =====
   renderManager->SetCamera(activeCamera);
   renderManager->SetCommandList(cmdList);

   // 全てのゲームオブジェクトを描画キューに追加
   for (const auto& obj : gameObjects_) {
	  if (obj && obj->IsActive()) {
		 renderManager->AddDrawable(obj.get());
	  }
   }

   // 一括描画（自動的にパスごとにソート・グループ化）
   renderManager->DrawAll();

   // フレーム終了時にキューをクリア
   renderManager->ClearQueue();

   // デバッグ描画（派生クラスでオーバーライド可能）
   DrawDebug();
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
   cameraManager_->SetActiveCamera("Debug");
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
}

void BaseScene::DrawGameObjectsImGui()
{
#ifdef _DEBUG
   if (ImGui::Begin("オブジェクト制御")) {
	  // 全オブジェクトのImGuiデバッグUI
	  for (auto& obj : gameObjects_) {
		 if (obj && obj->IsActive()) {
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
   // デフォルトでは何もしない（派生クラスでオーバーライド可能）
   auto lineRenderer = engine_->GetComponent<LineRenderer>();
   auto dxCommon = engine_->GetComponent<DirectXCommon>();
   ICamera* activeCamera = cameraManager_->GetActiveCamera();

   if (!lineRenderer || !dxCommon || !activeCamera) {
	  return;
   }

   // ゲームオブジェクトのデバッグ描画
   std::vector<LineRenderer::Line> debugLines;

   if (!debugLines.empty()) {
	  ID3D12GraphicsCommandList* cmdList = dxCommon->GetCommandList();
	  lineRenderer->Draw(
		 cmdList,
		 activeCamera->GetViewMatrix(),
		 activeCamera->GetProjectionMatrix(),
		 debugLines
	  );
   }
}
