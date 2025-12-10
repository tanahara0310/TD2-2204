#pragma once
#include "../GameObject.h"
#include "Engine/Utility/Timer/GameTimer.h"

/// @brief タイトル画面用のstartモデルクラス
class StartModel : public GameObject {
public:
	StartModel() = default;
	~StartModel() = default;

	/// @brief 初期化
	/// @param model モデル
	/// @param texture テクスチャ
	void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);

	/// @brief 更新処理
	void Update() override;

	/// @brief 描画処理
	/// @param camera カメラ
	void Draw(const ICamera* camera) override;
	
	/// @brief 選択状態を設定
	/// @param isSelected 選択されているか
	void SetSelected(bool isSelected) { isSelected_ = isSelected; }
	
	/// @brief 選択状態を取得
	bool IsSelected() const { return isSelected_; }
	
	/// @brief 決定演出モードを設定
	/// @param isConfirming 決定演出中かどうか
	void SetConfirmingMode(bool isConfirming) { isConfirming_ = isConfirming; }
	
	/// @brief オブジェクト名を取得
	const char* GetObjectName() const override { return "StartModel"; }
	
	/// @brief ロゴ登場演出を開始（遅延時間を指定可能）
	void StartIntroAnimation(float delayTime = 0.0f);
	
	/// @brief 演出中かどうかを取得
	bool IsAnimating() const { return isAnimating_; }

private:
	/// @brief 呼吸アニメーションの更新
	void UpdateBreathingAnimation(float deltaTime);
	
	/// @brief イントロアニメーションの更新
	void UpdateIntroAnimation(float deltaTime);

private:
	Vector3 baseScale_ = { };
	Vector3 targetPosition_ = {};
	Vector3 targetScale_ = {};
	
	// 選択状態
	bool isSelected_ = false;
	
	// 決定演出中フラグ
	bool isConfirming_ = false;
	
	// 呼吸アニメーション用
	float breathTimer_ = 0.0f;
	
	// イントロアニメーション用
	bool isAnimating_ = false;
	bool isDelaying_ = false;
	float delayTimer_ = 0.0f;
	float delayDuration_ = 0.0f;
	GameTimer animationTimer_;
	static constexpr float kAnimationDuration = 0.8f;
	
	Vector3 startPosition_ = { -40.0f, -5.5f, -60.9f }; // 左から登場
	
	// 呼吸アニメーション定数
	static constexpr float kBreathSpeed = 3.0f;
	static constexpr float kBreathAmplitude = 0.3f; // 振幅を大きく（より目立つ拡縮）
	static constexpr float kBaseScale = 1.0f;
};
