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

   {
	   // UI初期化
	   resultUI_ = std::make_unique<ResultUI>();
	   auto sprites = resultUI_->Initialize(engine);

	   // スプライトをgameObjects_に追加
	   for (auto& sprite : sprites) {
		   gameObjects_.push_back(std::move(sprite));
	   }
   }
}

void ResultScene::Update() {
   BaseScene::Update();

   auto input = engine_->GetComponent<KeyboardInput>();
   if (!input || !resultUI_) {
	   return;
   }

   // 遷移中でなければ入力を受け付ける
   if (!isTitleTransitioning_ || !isGameTransitioning_) {
	   // 右キーでスタートを選択
	   if (input->IsKeyTriggered(DIK_RIGHT)) {
		   resultUI_->SetSelectionState(ResultUI::SelectionState::ToTitle);
	   }

	   // 左キーでQuitを選択
	   if (input->IsKeyTriggered(DIK_LEFT)) {
		   resultUI_->SetSelectionState(ResultUI::SelectionState::ReStart);
	   }

	   // スペースキーで決定
	   if (input->IsKeyTriggered(DIK_SPACE)) {
		   switch (resultUI_->GetSelectionState()) {
		   case ResultUI::SelectionState::ToTitle:
			   // 遷移開始
			   isTitleTransitioning_ = true;
			   transitionTimer_ = 0.0f;
			   break;
		   case ResultUI::SelectionState::ReStart:
			   // アプリケーション終了
			   isGameTransitioning_ = true;
			   transitionTimer_ = 0.0f;
			   break;
		   }
	   }
   }

   // 遷移処理
   if (isTitleTransitioning_ || isGameTransitioning_) {
	   UpdateSceneTransition(1.0f / 60.0f); // 仮のデルタタイム
   }

   // リザルト画像の更新
   if (resultUI_)
	   resultUI_->Update();
}

void ResultScene::UpdateSceneTransition(float deltaTime) {
	transitionTimer_ += deltaTime;

	// 遷移時間が経過したらシーン遷移
	if (transitionTimer_ >= kTransitionDuration) {
		if (isTitleTransitioning_) {
			sceneManager_->ChangeScene("TitleScene");
		} else if (isGameTransitioning_) {
			sceneManager_->ChangeScene("GameScene");
		}
	}
}

void ResultScene::Draw() {
   BaseScene::Draw();
}

void ResultScene::Finalize() {}
