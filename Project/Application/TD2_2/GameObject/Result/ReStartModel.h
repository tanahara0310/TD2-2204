#pragma once
#include "../GameObject.h"
#include <memory>

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

private:
	/// @brief 呼吸アニメーションの更新
	void UpdateBreathingAnimation(float deltaTime);

	/// @brief 回転アニメーションの更新
	void UpdateRotateAnimation(float deltaTime);

	/// @brief 選択時のZ軸拡大アニメーションの更新
	void UpdateScaleAnimation(float deltaTime);

private:
	Vector3 baseScale_ = {1.0f,1.0f,1.0f}; // 基準スケール

	// 選択状態
	bool isSelected_ = false;

	// 呼吸アニメーション用
	float breathTimer_ =0.0f; // 呼吸タイマー
	static constexpr float kBreathSpeed =3.0f; // 呼吸の速度
	static constexpr float kBreathAmplitude =0.1f; // 呼吸の振幅（スケールの変化量）
	static constexpr float kBaseScale =1.0f; // 基本スケール

	// 回転アニメーションフラグ
	bool isRotateAnimation_ = false;

	// 打ち上げアニメーション用変数
	bool hasLaunched_ = false; //既に打ち上げ済みか
	float launchTimer_ =0.0f; // 経過タイマー
	static constexpr float kLaunchDuration =0.5f; // 打ち上げと戻りにかかる時間
	static constexpr float kLaunchHeight =2.0f; // ピーク時の高さオフセット
	static constexpr float kTotalRotation =2.0f * std::numbers::pi_v<float>; // 一回転
	float startY_ =0.0f; // 打ち上げ開始時のY位置
	Quaternion startQuaternion_ = {0.0f,0.0f,0.0f,1.0f}; // 開始時の回転

	// 選択によるZ軸拡大アニメーション
	bool isScaleAnimating_ = false;
	float scaleTimer_ =0.0f;
	static constexpr float kScaleDuration =0.5f; // 拡大→縮小にかかる時間
	static constexpr float kScalePeakMultiplier =1.6f; // ピーク時のZスケール倍率
	float scaleStartZ_ =1.0f; // 開始時のZスケール

	// 前フレームの選択状態（立ち上がり検出用）
	bool prevSelected_ = false;
};