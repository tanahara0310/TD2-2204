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
#include "Engine/Input/KeyboardInput.h"
#include "MathCore.h"

void ResultScene::Initialize(EngineSystem* engine) {
   BaseScene::Initialize(engine);

   InputSource::Initialize(engine);

   // クリアタイマーを文字列に変換
   timerDigits_ = FormatTime(currentClearTime_);

   // resultUI
   {
	   // UI初期化
	   resultUI_ = std::make_unique<ResultUI>();
	   auto sprites = resultUI_->Initialize(engine);

	   // スプライトをgameObjects_に追加
	   for (auto& sprite : sprites) {
		   gameObjects_.push_back(std::move(sprite));
	   }

	   // タイマー文字列を受け取る
	   resultUI_->SetTimerString(timerDigits_);
   }

   // KeyConfigの設定
   {
	   // 上方向の入力（キーボード上キー or ゲームパッドの十字キー上）
	   ActionBuilder(keyConfig_.AddAction("Right", ActionType::Bool))
		   .BindKey(DIK_RIGHT)
		   .BindGamepadButton(GamepadButton::DPadRight);

	   // 下方向の入力（キーボード下キー or ゲームパッドの十字キー下）
	   ActionBuilder(keyConfig_.AddAction("Left", ActionType::Bool))
		   .BindKey(DIK_LEFT)
		   .BindGamepadButton(GamepadButton::DPadLeft);

	   // 決定ボタン（キーボードスペース or ゲームパッドAボタン）
	   ActionBuilder(keyConfig_.AddAction("Confirm", ActionType::Bool))
		   .BindKey(DIK_SPACE)
		   .BindGamepadButton(GamepadButton::A);
   }
}

void ResultScene::Update() {
   BaseScene::Update();

   if (!resultUI_) {
	   return;
   }

   // 遷移中でなければ入力を受け付ける
   if (!isTitleTransitioning_ || !isGameTransitioning_) {
	   // 右キーでスタートを選択
	   if (keyConfig_.GetDown("Right")) {
		   resultUI_->SetSelectionState(ResultUI::SelectionState::ToTitle);
	   }

	   // 左キーでQuitを選択
	   if (keyConfig_.GetDown("Left")) {
		   resultUI_->SetSelectionState(ResultUI::SelectionState::ReStart);
	   }

	   // スペースキーで決定
	   if (keyConfig_.GetDown("Confirm")) {
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

std::array<int, 6> ResultScene::FormatTime(float time) {
	int totalMilliSeconds = static_cast<int>(time * 1000);

	int minutes = (totalMilliSeconds / 1000) / 60;
	int seconds = (totalMilliSeconds / 1000) % 60;
	int milliseconds = totalMilliSeconds % 1000;

	int m1 = minutes / 10;
	int m2 = minutes % 10;
	int s1 = seconds / 10;
	int s2 = seconds % 10;
	int ms1 = (milliseconds / 100) % 10;
	int ms2 = (milliseconds % 10) % 10;

	return {m1, m2, s1, s2, ms1, ms2};
}
