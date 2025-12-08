#pragma once
#include "../GameObject.h"

/// @brief コントローラー推奨画面用のコントローラーモデルクラス
class ControllerModel : public GameObject {
public:
	ControllerModel() = default;
	~ControllerModel() = default;

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
	const char* GetObjectName() const override { return "ControllerModel"; }

private:
	/// @brief 点滅アニメーションの更新
	void UpdateBlinkAnimation(float deltaTime);

private:
	// 点滅アニメーション用
	float blinkTimer_ = 0.0f;                     // 点滅タイマー
	static constexpr float kBlinkInterval = 0.5f; // 点滅の間隔（秒）
	bool isHighAlpha_ = true;                     // 現在のアルファ値が高いかどうか
	static constexpr float kAlphaHigh = 1.0f;     // 高アルファ値（255/255）
	static constexpr float kAlphaLow = 0.5f;      // 低アルファ値（127/255）
};
