#include "ResultScene.h"
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

void ResultScene::Initialize(EngineSystem* engine) {
   BaseScene::Initialize(engine);

}

void ResultScene::Update() {
   BaseScene::Update();

#ifdef _DEBUG
   // カメラマネージャーのImGui
   cameraManager_->DrawImGui();
#endif
}

void ResultScene::Draw() {
   // コンポーネント取得
   auto renderManager = engine_->GetComponent<RenderManager>();
   auto dxCommon = engine_->GetComponent<DirectXCommon>();
   ICamera* activeCamera = cameraManager_->GetActiveCamera();

   // 必須コンポーネントのチェック
   if (!renderManager || !dxCommon || !activeCamera) {
	  return;
   }

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

void ResultScene::Finalize() {

   // このシーン専用のライトを削除
   directionalLight_ = nullptr;
}
