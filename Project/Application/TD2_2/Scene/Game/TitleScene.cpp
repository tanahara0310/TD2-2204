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
	  // 球体オブジェクトの生成と初期化
	  auto sphere = std::make_unique<Sphere>();
	  sphere->Initialize(engine_);
	  gameObjects_.push_back(std::move(sphere));

   }
}

void TitleScene::Update() {
   BaseScene::Update();

   // ゲームオブジェクトの更新
   for (auto& obj : gameObjects_) {
	  if (obj->IsActive()) {
		 obj->Update();
	  }
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

	  // 描画対象オブジェクトをキューに追加
	  for (auto& obj : gameObjects_) {
		 if (obj->IsActive()) {
			renderManager->AddDrawable(obj.get());
		 }
	  }

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
