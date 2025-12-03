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

   // クリアタイマーを文字列に変換
   {
	   timerDigits_ = FormatTime(currentClearTime_);

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

std::string ResultScene::FormatTime(float time) {
	int totalSeconds = (int)time;

	int hours = totalSeconds / 3600;
	int minutes = (totalSeconds % 3600) / 60;
	int seconds = totalSeconds % 60;

	int h1 = hours / 10;
	int h2 = hours % 10;
	int m1 = minutes / 10;
	int m2 = minutes % 10;
	int s1 = seconds / 10;
	int s2 = seconds % 10;

	return std::to_string(h1) + std::to_string(h2) + std::to_string(m1) + std::to_string(m2) + std::to_string(s1) + std::to_string(s2);
}
