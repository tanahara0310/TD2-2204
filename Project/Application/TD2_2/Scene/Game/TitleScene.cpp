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
#include "Engine/Graphics/Resource/ResourceFactory.h"
#include "Engine/Graphics/Model/ModelManager.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Input/KeyboardInput.h"
#include "Engine/Input/GamepadInput.h"
#include "Engine/WinApp/WinApp.h"
#include "MathCore.h"
#include <dinput.h>


void TitleScene::Initialize(EngineSystem* engine) {
	BaseScene::Initialize(engine);

	// InputSourceの初期化（必須）
	InputSource::Initialize(engine);

	// KeyConfigの設定
	{
		// 上方向の入力（キーボード上キー or ゲームパッドの十字キー上）
		ActionBuilder(keyConfig_.AddAction("Up", ActionType::Bool))
			.BindKey(DIK_UP)
			.BindGamepadButton(GamepadButton::DPadUp);

		// 下方向の入力（キーボード下キー or ゲームパッドの十字キー下）
		ActionBuilder(keyConfig_.AddAction("Down", ActionType::Bool))
			.BindKey(DIK_DOWN)
			.BindGamepadButton(GamepadButton::DPadDown);

		// 決定ボタン（キーボードスペース or ゲームパッドAボタン）
		ActionBuilder(keyConfig_.AddAction("Confirm", ActionType::Bool))
			.BindKey(DIK_SPACE)
			.BindGamepadButton(GamepadButton::A);
	}


	{
		// UI初期化
		titleUI_ = std::make_unique<TitleUI>();
		auto sprites = titleUI_->Initialize(engine);

		// スプライトをgameObjects_に追加
		for (auto& sprite : sprites) {
			gameObjects_.push_back(std::move(sprite));
		}
	}

}

void TitleScene::Update() {
	BaseScene::Update();


	if (!titleUI_) {
		return;
	}


	// 遷移中でなければ入力を受け付ける
	if (!isTransitioning_) {
		// 上キーでスタートを選択（キーボード or ゲームパッド）
		if (keyConfig_.GetDown("Up")) {
			titleUI_->SetSelectionState(TitleUI::SelectionState::Start);
		}

		// 下キーでQuitを選択（キーボード or ゲームパッド）
		if (keyConfig_.GetDown("Down")) {
			titleUI_->SetSelectionState(TitleUI::SelectionState::Quit);
		}

		// 決定ボタン（キーボード or ゲームパッド）
		if (keyConfig_.GetDown("Confirm")) {
			// 決定アニメーション開始
			titleUI_->OnConfirm();

			switch (titleUI_->GetSelectionState()) {
			case TitleUI::SelectionState::Start:
				// 遷移開始
				isTransitioning_ = true;
				transitionTimer_ = 0.0f;

				break;
			case TitleUI::SelectionState::Quit:
				// アプリケーション終了
				PostQuitMessage(0);
				break;
			}
		}
	}

	// 遷移処理
	if (isTransitioning_) {
		UpdateSceneTransition(1.0f / 60.0f); // 仮のデルタタイム
	}

	// UI更新（ステートマシーンも含む）
	if (titleUI_) {
		titleUI_->Update();
	}
}



void TitleScene::UpdateSceneTransition(float deltaTime) {
	transitionTimer_ += deltaTime;

	// 遷移時間が経過したらシーン遷移
	if (transitionTimer_ >= kTransitionDuration) {
		sceneManager_->ChangeScene("GameScene");
	}
}

void TitleScene::Draw() {
	BaseScene::Draw();
}

void TitleScene::Finalize() {
}
