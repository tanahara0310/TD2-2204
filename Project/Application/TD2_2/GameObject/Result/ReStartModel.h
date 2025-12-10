#pragma once
#include "../GameObject.h"
#include <memory>
#include <numbers>

class ReStartModel : public GameObject {
public:
	ReStartModel() = default;
	~ReStartModel() = default;
	void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);
	void Update() override;
	void Draw(const ICamera* camera) override;

	const char* GetObjectName() const override { return "ReStartModel"; }

	/// @brief 選択状態を設定
	/// @param isSelected 選択されているか
	void SetSelected(bool isSelected) { isSelected_ = isSelected; }

	/// @brief 選択状態を取得
	bool IsSelected() const { return isSelected_; }

	/// @brief 回転アニメーションのフラグを設定
	void SetIsRotateAnimation(bool isRotateAnimation) { isRotateAnimation_ = isRotateAnimation; }

	/// @brief スケールアニメーションのフラグを設定
	void SetScalAnimation(bool isScaleAnimation) { isScaleAnimation_ = isScaleAnimation; }

private:
	/// @brief 呼吸アニメーションの更新
	void UpdateBreathingAnimation(float deltaTime);

	/// @brief 回転アニメーションの更新
	void UpdateRotateAnimation(float deltaTime);

	/// @brief スケールアニメーションの更新
	void UpdateScaleAnimation(float deltaTime);

private:
	Vector3 baseScale_ = {1.0f, 1.0f, 1.0f}; // 基準スケール

	// 選択状態
	bool isSelected_ = false;

	// 呼吸アニメーション用
	float breathTimer_ = 0.0f;                      // 呼吸タイマー
	static constexpr float kBreathSpeed = 3.0f;     // 呼吸の速度
	static constexpr float kBreathAmplitude = 0.1f; // 呼吸の振幅（スケールの変化量）
	static constexpr float kBaseScale = 0.8f;       // 基本スケール

	// 回転アニメーションフラグ
	bool isRotateAnimation_ = false;

	// スケールアニメーションフラグ
	bool isScaleAnimation_ = false;

	// スケールアニメーション用変数
	bool hasScaleLaunched_ = false;                 // スケールアニメ開始済みフラグ
	float scaleTimer_ = 0.0f;                       // スケールアニメ経過時間
	float startScaleZ_ = 1.0f;                      // アニメ開始時の Z スケール
	static constexpr float kScaleDuration = 0.20f;  // アニメ総時間（秒）
	static constexpr float kScalePeakMultiplier = 1.6f; // ピーク時の倍率（Z方向）

	// カメラ方向に拡大するための直近カメラ位置（Drawで更新）
	Vector3 lastCameraPos_ = {0.0f, 0.0f, 0.0f};

	// 打ち上げアニメーション用変数
	bool hasLaunched_ = false;                                                // 既に打ち上げ済みか
	float launchTimer_ = 0.0f;                                                // 経過タイマー
	static constexpr float kLaunchDuration = 0.5f;                            // 打ち上げと戻りにかかる時間
	static constexpr float kLaunchHeight = 2.0f;                              // ピーク時の高さオフセット
	static constexpr float kTotalRotation = 4.0f * std::numbers::pi_v<float>; // 一回転
	float startY_ = 0.0f;                                                     // 打ち上げ開始時のY位置
	Quaternion startQuaternion_ = {0.0f, 0.0f, 0.0f, 1.0f};                   // 開始時の回転
};