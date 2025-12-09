#pragma once

#include <memory>
#include "Scene/BaseScene.h"

// ゲームオブジェクトのインクルード
#include "Object3d.h"
#include "../../GameObject/Voxel/Voxel.h"
#include "../../GameObject/TitleUI/TitleUI.h"
#include "../../GameObject/Background/Background.h"
#include "../../GameObject/TitleDemo/TitlePlayerDemo.h"
#include "../../GameObject/TitleDemo/TitleEnemyDemo.h"
#include "../../Utility/KeyConfig.h"
#include "../../Camera/TitleCameraController.h"
#include "../../Effect/Lightning/LightningEffectManager.h"


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

	/// @brief 決定時のリアクション演出の更新
	void UpdateConfirmAnimation(float deltaTime);
	
	/// @brief フェードアウト処理の更新
	void UpdateFadeOut(float deltaTime);

	/// @brief デモパターンを切り替える
	void SwitchDemoPattern();

private:
	std::unique_ptr<TitleUI> titleUI_;

	// 背景
	Background* background_ = nullptr;

	// デモ演出用の自機と敵
	TitlePlayerDemo* demoPlayer_ = nullptr;
	TitleEnemyDemo* demoEnemy_ = nullptr;

	// デモパターン管理
	enum class DemoPattern {
		EnemyChasePlayer,  // 敵が自機を追跡
		PlayerChaseEnemy   // 自機が敵を追跡
	};
	DemoPattern currentDemoPattern_ = DemoPattern::EnemyChasePlayer;
	bool isMovingRight_ = true; // true = +X方向, false = -X方向

	// キーコンフィグ
	std::unique_ptr<KeyConfig> keyConfig_;

	// タイトルカメラコントローラー
	std::unique_ptr<TitleCameraController> cameraController_;

	// スティック入力のクールダウン
	float stickInputCooldown_ = 0.0f;
	static constexpr float kStickInputDelay = 0.2f; // スティック入力の遅延時間（秒）
	static constexpr float kStickThreshold = 0.5f;   // スティック入力の閾値

	// シーン遷移フラグとタイマー
	bool isTransitioning_ = false;
	float transitionTimer_ = 0.0f;
	static constexpr float kTransitionDuration = 0.3f; // 0.3秒で遷移（決定アニメーションと同時）

	// 決定時のリアクション演出用
	bool isConfirmAnimating_ = false;
	GameTimer confirmAnimationTimer_;
	static constexpr float kConfirmAnimationDuration = 0.3f; // 決定演出の時間
	static constexpr float kSelectedScaleMax = 1.5f; // 選択されたモデルの最大スケール
	float selectedModelInitialScale_ = 1.0f; // 選択されたモデルの初期スケール
	Vector4 unselectedModelOriginalColor_ = { 1.0f, 1.0f, 1.0f, 1.0f }; // 非選択モデルの元の色

	// 終了待機用
	bool isWaitingForQuit_ = false;
	float quitWaitTimer_ = 0.0f;
	static constexpr float kQuitWaitDuration = 0.5f; // 終了前の待機時間

	// フェードアウト用
	bool isFadingOut_ = false;
	GameTimer fadeOutTimer_;
	static constexpr float kFadeOutDuration = 0.5f; // フェードアウトの時間

	Sound titleBGM_;

	// 雷エフェクト
	std::unique_ptr<LightningEffectManager> lightningManager_;
	// 枠を構成する4本のエフェクトID（上,下,左,右）
	int frameEffectIds_[4] = { -1, -1, -1, -1 };

	// パルス演出（1セット）
	float pulseTimer_ = 0.0f;           // 次のパルスまでの待ち時間
	float visibleTimer_ = 0.0f;         // 現在のパルスの表示残時間
	int currentEdgeIndexA_ = -1;        // 現在点滅中の辺A
	int currentEdgeIndexB_ = -1;        // 現在点滅中の辺B
	float flickerTimer_ = 0.0f;         // 雷のような高速点滅用
	static constexpr float kPulseDuration_ = 0.25f; // パルス表示時間（少し長め）
	static constexpr float kFlickerInterval_ = 0.03f; // 高頻度点滅（短いほど高速）
	static constexpr float kPulseIntervalMin_ = 0.2f; // パルス間隔の最小（さらに短く）
	static constexpr float kPulseIntervalMax_ = 1.0f; // パルス間隔の最大（さらに短く）
	float nextPulseInterval_ = 0.5f;    // 次のパルスまでの間隔（乱数で更新）

	// 選択状態のキャッシュ（選択変更検知用）
	bool lastIsStartSelected_ = true;
};
