#pragma once

#include <memory>
#include "Scene/BaseScene.h"

// ゲームオブジェクトのインクルード
#include "Object3d.h"
#include "../../../GameObject/Voxel/Voxel.h"
#include "../../../GameObject/TitleUI/TitleUI.h"
#include "../../../GameObject/Background/Background.h"
#include "../../../GameObject/Cloud/Cloud.h"
#include "../../../GameObject/TitleDemo/TitlePlayerDemo.h"
#include "../../../GameObject/TitleDemo/TitleEnemyDemo.h"
#include "../../../Utility/KeyConfig.h"
#include "../../../Camera/TitleCameraController.h"
#include "../../../Effect/Lightning/LightningEffectManager.h"
#include "TitleDemoManager.h"
#include "TitleLightningFrameManager.h"
#include "TitleLogoLightningManager.h"
#include "TitleConfirmAnimationManager.h"
#include "../../../Utility/StateMachine.h"


class EngineSystem;
class CameraManager;
class Camera;

/// @brief タイトルシーンクラス
class TitleScene : public BaseScene {
public:
	/// @brief 初期化
	void Initialize(EngineSystem* engine) override;

	/// @brief 更新
	void Update() override;

	/// @brief 描画処理
	void Draw() override;

	/// @brief 解放
	void Finalize() override;

protected:
	/// @brief リリースカメラの初期設定をカスタマイズ
	void SetupReleaseCameraParameters(Camera* camera) override;

private:
	/// @brief シーン遷移の処理
	void UpdateSceneTransition(float deltaTime);
	
	/// @brief フェードアウト処理の更新
	void UpdateFadeOut(float deltaTime);
	
	/// @brief イントロステートマシーンの初期化
	void InitializeIntroStateMachine();
	
	/// @brief イントロスキップ処理
	void SkipIntroAnimation();

private:
	std::unique_ptr<TitleUI> titleUI_;

	// 背景
	Background* background_ = nullptr;

	// 雲
	std::array<Cloud*, 4> clouds_;

	// デモ演出用の自機と敵
	TitlePlayerDemo* demoPlayer_ = nullptr;
	TitleEnemyDemo* demoEnemy_ = nullptr;

	// キーコンフィグ
	std::unique_ptr<KeyConfig> keyConfig_;

	// タイトルカメラコントローラー
	std::unique_ptr<TitleCameraController> cameraController_;

	// 各種マネージャー
	std::unique_ptr<TitleDemoManager> demoManager_;
	std::unique_ptr<LightningEffectManager> lightningManager_;
	std::unique_ptr<TitleLightningFrameManager> lightningFrameManager_;
	std::unique_ptr<TitleLogoLightningManager> logoLightningManager_;
	std::unique_ptr<TitleConfirmAnimationManager> confirmAnimationManager_;
	
	// イントロステートマシーン
	StateMachine introStateMachine_;
	bool introSkipped_ = false;

	// スティック入力のクールダウン
	float stickInputCooldown_ = 0.0f;
	static constexpr float kStickInputDelay = 0.2f;
	static constexpr float kStickThreshold = 0.5f;

	// シーン遷移フラグとタイマー
	bool isTransitioning_ = false;
	float transitionTimer_ = 0.0f;
	static constexpr float kTransitionDuration = 0.3f;

	// 終了待機用
	bool isWaitingForQuit_ = false;
	float quitWaitTimer_ = 0.0f;
	static constexpr float kQuitWaitDuration = 0.5f;

	// フェードアウト用
	bool isFadingOut_ = false;
	GameTimer fadeOutTimer_;
	static constexpr float kFadeOutDuration = 0.5f;

	Sound titleBGM_;
	Sound confirmSE_;
	Sound cursorSE_;

	// 枠エフェクトID（LightningFrameManagerから参照するため保持）
	int frameEffectIds_[4] = { -1, -1, -1, -1 };
	
	// 雷エフェクト表示フラグ
	bool lightningEffectShown_ = false;
	
	// 白フラッシュ演出用
	bool isWhiteFlashing_ = false;
	GameTimer whiteFlashTimer_;
	static constexpr float kWhiteFlashDuration = 0.3f;
};
