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
	
	/// @brief イントロアニメーションを開始
	void StartIntroAnimation();
	
	/// @brief イントロアニメーションが完了したかを取得
	/// @return 完了していればtrue
	bool IsIntroAnimationFinished() const { return !isIntroPlaying_; }

private:
	Vector3 baseScale_ = { 1.0f, 1.0f, 1.0f };
	Vector3 targetPosition_ = { 0.0f, -1.9f, -59.9f };
	Vector3 targetScale_ = { 1.0f, 1.0f, 2.0f };
	
	// イントロアニメーション用
	GameTimer introTimer_;
	bool isIntroPlaying_ = false;
	static constexpr float kIntroDuration = 0.3f; // 短時間で急に出現
};
