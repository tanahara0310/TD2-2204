#pragma once
#include "Object3d.h"

/// @brief ボクセルオブジェクト
class Voxel : public Object3d {
public:
	/// @brief デフォルトコンストラクタ
	Voxel() = default;

	/// @brief デストラクタ
	~Voxel() override = default;

	/// @brief 初期化
	void Initialize();

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

private:
};
