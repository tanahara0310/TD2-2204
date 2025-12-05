#pragma once
#include "../GameObject.h"

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
};
