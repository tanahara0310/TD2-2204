#pragma once

#include <memory>
#include "Scene/BaseScene.h"

//ゲームオブジェクトのインクルード
#include "Object3d.h"
#include "../../GameObject/Voxel/Voxel.h"
#include "../../GameObject/TitleUI/TitleUI.h"
#include "../../GameObject/Background/Background.h"
#include "../../Utility/KeyConfig.h"
#include "../../Camera/TitleCameraController.h"


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
	// /// @brief 電気パーティクルエフェクトを作成
	// void CreateElectricParticleEffect();

	/// @brief シーン遷移の処理
	void UpdateSceneTransition(float deltaTime);

private:
	std::unique_ptr<TitleUI> titleUI_;
	
	// 背景
	Background* background_ = nullptr;
	
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

	Sound titleBGM_;
};
