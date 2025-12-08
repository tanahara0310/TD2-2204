#pragma once
#include "../GameObject.h"

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

private:
	Vector3 baseScale_ = { 1.0f, 1.0f, 1.0f }; // 基準スケール
};
