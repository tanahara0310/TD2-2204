#pragma once
#include "../GameObject.h"

/// @brief コントローラー推奨画面用の「推奨」テキストモデルクラス
class RecommendedModel : public GameObject {
public:
	RecommendedModel() = default;
	~RecommendedModel() = default;

	/// @brief 初期化
	/// @param model モデル
	/// @param texture テクスチャ
	/// @param position 初期位置
	void Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture, const Vector3& position);

	/// @brief 更新処理
	void Update() override;

	/// @brief 描画処理
	/// @param camera カメラ
	void Draw(const ICamera* camera) override;
	
	/// @brief 出現アニメーションを開始
	/// @param delay 開始遅延時間（秒）
	void StartAppearAnimation(float delay);
	
	/// @brief オブジェクト名を取得
	const char* GetObjectName() const override { return "RecommendedModel"; }

private:
	/// @brief 出現アニメーションの更新
	void UpdateAppearAnimation(float deltaTime);

private:
	Vector3 targetPosition_;                        // 最終的な位置
	Vector3 startPosition_;                         // 開始位置（左側）
	
	// 出現アニメーション用
	float appearTimer_ = 0.0f;                      // アニメーションタイマー
	float appearDelay_ = 0.0f;                      // 開始遅延時間
	bool isAnimating_ = false;                      // アニメーション中かどうか
	static constexpr float kAppearDuration = 0.5f;  // 出現アニメーションの時間（秒）
	static constexpr float kOffsetX = -10.0f;       // 左側のオフセット距離
};
