#pragma once
#include "../GameObject.h"
#include "Engine/Utility/Timer/GameTimer.h"

/// @brief タイトル画面用のGekitotsuモデルクラス
class GekitotsuModel : public GameObject {
public:
	GekitotsuModel() = default;
	~GekitotsuModel() = default;

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
	const char* GetObjectName() const override { return "GekitotsuModel"; }
	
	/// @brief 色を設定
	/// @param color 色（RGBA）
	void SetColor(const Vector4& color);
	
	/// @brief 色を取得
	/// @return 色（RGBA）
	Vector4 GetColor() const;
	
	/// @brief ロゴ登場演出を開始（遅延時間を指定可能）
	void StartIntroAnimation(float delayTime = 0.0f);
	
	/// @brief 演出中かどうかを取得
	bool IsAnimating() const { return isAnimating_; }

private:
	/// @brief イントロアニメーションの更新
	void UpdateIntroAnimation(float deltaTime);

private:
	Vector3 baseScale_ = { 1.0f, 1.0f, 1.0f };
	Vector3 targetPosition_ = { 0.0f, -1.9f, -59.9f };
	Vector3 targetScale_ = { 1.0f, 1.0f, 2.0f };
	
	// アニメーション制御
	bool isAnimating_ = false;
	bool isDelaying_ = false;
	float delayTimer_ = 0.0f;
	float delayDuration_ = 0.0f;
	GameTimer animationTimer_;
	static constexpr float kAnimationDuration = 1.2f;
	
	// 左右からの衝突演出用
	Vector3 leftStartPosition_ = { -80.0f, -1.9f, -59.9f };
	Vector3 rightStartPosition_ = { 80.0f, -1.9f, -59.9f };
	float splitOffset_ = 40.0f; // 左右に分離する距離
};
