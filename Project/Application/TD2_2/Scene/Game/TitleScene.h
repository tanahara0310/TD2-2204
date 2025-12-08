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

	// 雷エフェクト
	std::unique_ptr<LightningEffectManager> lightningManager_;
	// 枠を構成する4本のエフェクトID（上,下,左,右）
	int frameEffectIds_[4] = { -1, -1, -1, -1 };

	// パルス演出（1セット）
	float pulseTimer_ = 0.0f;           // 次のパルスまでの待ち時間
	float visibleTimer_ = 0.0f;         // 現在のパルスの表示残時間
	int currentEdgeIndex_ = -1;         // 現在点滅中の辺
	float flickerTimer_ = 0.0f;         // 雷のような高速点滅用
	static constexpr float kPulseInterval_ = 1.2f; // 表示されるまでの間隔を長めに
	static constexpr float kPulseDuration_ = 0.25f; // パルス表示時間（少し長め）
	static constexpr float kFlickerInterval_ = 0.06f; // 点滅周波数（短いほど高速）

	// 選択状態のキャッシュ（選択変更検知用）
	bool lastIsStartSelected_ = true;
};
