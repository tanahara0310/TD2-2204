#pragma once
#include "../GameObject.h"
#include <memory>

class ToTitleModel : public GameObject {
public:
	ToTitleModel() = default;
	~ToTitleModel() = default;
	void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);
	void Update() override;
	void Draw(const ICamera* camera) override;

	const char* GetObjectName() const override { return "ToTitleModel"; }

    /// @brief 選択状態を設定
	/// @param isSelected 選択されているか
	void SetSelected(bool isSelected) { isSelected_ = isSelected; }

	/// @brief 選択状態を取得
	bool IsSelected() const { return isSelected_; }

private:
	/// @brief 呼吸アニメーションの更新
	void UpdateBreathingAnimation(float deltaTime);

private:
	Vector3 baseScale_ = {1.0f, 1.0f, 1.0f}; // 基準スケール

	// 選択状態
	bool isSelected_ = false;

	// 呼吸アニメーション用
	float breathTimer_ = 0.0f;                      // 呼吸タイマー
	static constexpr float kBreathSpeed = 3.0f;     // 呼吸の速度
	static constexpr float kBreathAmplitude = 0.1f; // 呼吸の振幅（スケールの変化量）
	static constexpr float kBaseScale = 1.0f;       // 基本スケール
};