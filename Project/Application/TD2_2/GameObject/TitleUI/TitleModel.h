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
	
	/// @brief イントロアニメーションを開始
	void StartIntroAnimation();
	
	/// @brief イントロアニメーションが完了したかを取得
	/// @return 完了していればtrue
	bool IsIntroAnimationFinished() const { return !isIntroPlaying_; }

private:
	Vector3 baseScale_ = { 1.0f, 1.0f, 1.0f };
	Vector3 targetPosition_ = { 0.0f, -4.0f, -60.9f };
	Vector3 targetScale_ = { 1.4f, 1.4f, 2.0f };
	
	// イントロアニメーション用
	GameTimer introTimer_;
	bool isIntroPlaying_ = false;
	Vector3 startPosition_ = { 0.0f, 20.0f, -60.9f }; // 画面外上
	static constexpr float kIntroDuration = 0.8f; // アニメーション時間を短縮して速度を上げる
};
