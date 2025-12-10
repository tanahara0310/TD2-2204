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
	
	/// @brief イントロアニメーションを開始
	void StartIntroAnimation();
	
	/// @brief イントロアニメーションが完了したかを取得
	/// @return 完了していればtrue
	bool IsIntroAnimationFinished() const { return !isIntroPlaying_; }
	
	/// @brief イントロアニメーションの進行度を取得
	/// @return 進行度（0.0～1.0）
	float GetIntroAnimationProgress() const { 
		return isIntroPlaying_ ? introTimer_.GetProgress() : 1.0f; 
	}
	
	/// @brief 現在のスケール倍率を取得（ベーススケール基準）
	/// @return スケール倍率
	float GetCurrentScaleRatio() const {
		return transform_.scale.y / baseScale_.y;
	}

private:
	/// @brief 呼吸アニメーションの更新
	void UpdateBreathingAnimation(float deltaTime);

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
	
	// 呼吸アニメーション定数
	static constexpr float kBreathSpeed = 5.0f;
	static constexpr float kBreathAmplitude = 0.15f; // 振幅を大きく（より目立つ拡縮）
	static constexpr float kBaseScale = 1.0f;
	
	// イントロアニメーション用
	GameTimer introTimer_;
	bool isIntroPlaying_ = false;
	Vector3 startPosition_ = { -30.0f, -5.8f, -60.9f }; // 画面左端
	static constexpr float kIntroDuration = 0.6f; // アニメーション時間
};
