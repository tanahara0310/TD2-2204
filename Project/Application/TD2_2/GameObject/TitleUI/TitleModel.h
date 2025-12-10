#pragma once
#include "../GameObject.h"
#include "Engine/Utility/Timer/GameTimer.h"

/// @brief タイトル画面用のtitleモデルクラス
class TitleModel : public GameObject {
public:
	TitleModel() = default;
	~TitleModel() = default;

	/// @brief 初期化
	/// @param model モデル
	/// @param texture テクスチャ
	void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture);

	/// @brief 更新処理
	void Update() override;

	/// @brief 描画処理
	/// @param camera カメラ
	void Draw(const ICamera* camera) override;
	
	/// @brief オブジェクト名を取得
	const char* GetObjectName() const override { return "TitleModel"; }
	
	/// @brief 色を設定
	/// @param color 色（RGBA）
	void SetColor(const Vector4& color);
	
	/// @brief 色を取得
	/// @return 色（RGBA）
	Vector4 GetColor() const;
	
	/// @brief ロゴ登場演出を開始
	void StartIntroAnimation();
	
	/// @brief 演出中かどうかを取得
	bool IsAnimating() const { return isAnimating_; }

private:
	/// @brief イントロアニメーションの更新
	void UpdateIntroAnimation(float deltaTime);

private:
	Vector3 baseScale_ = { 1.0f, 1.0f, 1.0f };
	Vector3 targetPosition_ = { 0.0f, -4.0f, -60.9f };
	Vector3 targetScale_ = { 1.4f, 1.4f, 2.0f };
	
	// アニメーション制御
	bool isAnimating_ = false;
	GameTimer animationTimer_;
	static constexpr float kAnimationDuration = 1.8f;
	
	Vector3 startPosition_ = { 0.0f, 50.0f, -60.9f };
	Vector3 startScale_ = { 3.0f, 3.0f, 3.0f };
	float startRotation_ = 0.0f;
};
