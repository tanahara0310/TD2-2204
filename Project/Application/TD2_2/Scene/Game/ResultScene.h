#pragma once

#include <memory>
#include "Scene/BaseScene.h"
#include "Engine/Graphics/Sprite/Sprite.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/ObjectCommon/SpriteObject.h"
#include "../../GameObject/Result/ResultUI.h"
#include "../../Utility/KeyConfig.h"
#include "../../GameObject/Background/Background.h"

class EngineSystem;
class CameraManager;
struct DirectionalLightData;

/// @brief リザルトシーンクラス
class ResultScene : public BaseScene {
public:
	/// @brief 初期化
	void Initialize(EngineSystem* engine) override;

	/// @brief 更新
	void Update() override;

	/// @brief 描画処理
	void Draw() override;

	/// @brief 解放
	void Finalize() override;

private:
	/// @brief シーン遷移の処理
	void UpdateSceneTransition(float deltaTime);

protected:
	/// @brief リリースカメラの初期設定をカスタマイズ
	void SetupReleaseCameraParameters(Camera* camera) override;

	/// @brief クリアタイマーを時:分:秒に分解する
	/// @param time クリアタイマー
	std::array<int, 6> FormatTime(float time); 

private:
	// クリア時間　上位3つ
	float clearTimes_[3]{};

	// 今回のクリアタイム
	float currentClearTime_ = 123.456f;

	// リザルト画像
	SpriteObject* resultSprite_ = nullptr;

	// リスタート画像
	std::unique_ptr<Sprite> restartSprite_;

	// 「タイトルへ」画像
	std::unique_ptr<Sprite> toTitleSprite_;

	// タイマー画像
	std::unique_ptr<Sprite> curretTimeSprite_;

	// リザルトUI
	std::unique_ptr<ResultUI> resultUI_;

	// 背景
	Background* background_ = nullptr;

	// キーコンフィグ
	KeyConfig keyConfig_;

	// シーン遷移フラグとタイマー
	bool isTitleTransitioning_ = false;
	bool isGameTransitioning_ = false;
	float transitionTimer_ = 0.0f;
	static constexpr float kTransitionDuration = 1.0f; // 1秒で遷移

	// タイマーの6要素(00:00:00)を格納する変数
	std::array<int, 6> timerDigits_;

	// BGM
	Sound mp3Resource_;
};
