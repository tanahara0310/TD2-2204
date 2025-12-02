#pragma once
#include "Application/TD2_2/GameObject/GameObject.h"

/// @brief ボクセルオブジェクト
class Voxel : public GameObject {
public:
	/// @brief デフォルトコンストラクタ
	Voxel() = default;

	/// @brief デストラクタ
	~Voxel() override = default;

	/// @brief 初期化
	/// @param model モデルリソース（共有）
	/// @param texture テクスチャ（共有）
	void Initialize(ModelResource* model, TextureManager::LoadedTexture texture);

	/// @brief 更新処理
	void Update() override;

	/// @brief 描画処理
	/// @param camera カメラ
	void Draw(const ICamera* camera) override;

	/// @brief ImGuiデバッグUI描画
	/// @return ImGuiで変更があった場合true
	bool DrawImGui() override;

	/// @brief オブジェクト名を取得
	/// @return オブジェクト名
	const char* GetObjectName() const override { return "Voxel"; }

	/// @brief 色を設定
	/// @param color 色（RGBA）
	void SetColor(const Vector4& color);

	/// @brief 色を取得
	/// @return 色（RGBA）
	Vector4 GetColor() const;

private:
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルトは白
};
