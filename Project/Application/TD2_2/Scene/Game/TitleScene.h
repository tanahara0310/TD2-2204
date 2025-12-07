#pragma once

#include <memory>
#include "Scene/BaseScene.h"

// ゲームオブジェクトのインクルード
#include "Object3d.h"
#include "../../GameObject/Voxel/Voxel.h"
#include "../../GameObject/TitleUI/TitleUI.h"
#include "../../GameObject/Background/Background.h"
#include "../../Utility/KeyConfig.h"
#include "../../Camera/TitleCameraController.h"
#include "../../Effect/Lightning/LightningEffectManager.h"
#include "Engine/Utility/Timer/GameTimer.h"


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
	
	/// @brief 雷エフェクトの更新
	void UpdateLightningEffect(float deltaTime);
	
	/// @brief 次の雷発生タイミングをランダムに設定
	void SetRandomLightningInterval();
	
	/// @brief 雷の表示時間をランダムに設定
	void SetRandomLightningDuration();

private:
	std::unique_ptr<TitleUI> titleUI_;
	
	// 背景
	Background* background_ = nullptr;
	
	// キーコンフィグ
	std::unique_ptr<KeyConfig> keyConfig_;
	
	// タイトルカメラコントローラー
	std::unique_ptr<TitleCameraController> cameraController_;
	
	// 雷エフェクトマネージャー
	std::unique_ptr<LightningEffectManager> lightningManager_;
	
	// UIライトニングエフェクトID
	int startLightningEffectId_ = -1;
	int yameruLightningEffectId_ = -1;
	
	// ライトニングエフェクト用タイマー
	GameTimer lightningIntervalTimer_;   // 雷の出現間隔タイマー
	GameTimer lightningDisplayTimer_;    // 雷の表示時間タイマー
	bool isLightningActive_ = false;     // 現在雷が表示中かどうか
	
	// ライトニングエフェクトの時間設定（定数）
	static constexpr float kLightningIntervalMin = 2.0f;         // 最小出現間隔（長めに設定）
	static constexpr float kLightningIntervalMax = 4.0f;         // 最大出現間隔（長めに設定）
	static constexpr float kLightningDisplayDuration = 0.05f;    // 雷の表示時間（固定：一瞬の閃光）
	
	// スティック入力のクールダウン
	float stickInputCooldown_ = 0.0f;
	static constexpr float kStickInputDelay = 0.2f; // スティック入力の遅延時間（秒）
	static constexpr float kStickThreshold = 0.5f;   // スティック入力の閾値
	
	// シーン遷移フラグとタイマー
	bool isTransitioning_ = false;
	float transitionTimer_ = 0.0f;
	static constexpr float kTransitionDuration = 0.3f; // 0.3秒で遷移（決定アニメーションと同時）

	Sound titleBGM_;
};
