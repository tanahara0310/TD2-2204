#pragma once

#include "Object3D.h"
#include <memory>

/// @brief SkyBoxオブジェクト
class SkyBoxObject : public Object3d {
public:
	/// @brief コンストラクタ
	SkyBoxObject() = default;

	/// @brief デストラクタ
	~SkyBoxObject() override = default;

	/// @brief 初期化
	/// @param engine エンジンシステム
	void Initialize();

	/// @brief 更新
	void Update() override;

	/// @brief 描画
	/// @param camera カメラ
	void Draw(const ICamera* camera) override;

	/// @brief デバッグUI描画
	/// @return 変更があった場合true
	bool DrawImGui() override;

	/// @brief オブジェクト名を取得
	/// @return オブジェクト名
	const char* GetObjectName() const override { return "SkyBox"; }

	/// @brief このオブジェクトの描画パスタイプを取得
	/// @return 描画パスタイプ（SkyBox）
	RenderPassType GetRenderPassType() const override { return RenderPassType::SkyBox; }

private:
	/// @brief 箱の頂点データを生成
	void CreateBoxVertices();

	/// @brief マテリアル用定数バッファを作成
	void CreateMaterialBuffer();

	/// @brief トランスフォーム用定数バッファを作成
	void CreateTransformBuffer();

	/// @brief 頂点バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;

	/// @brief 頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	/// @brief インデックスバッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_;

	/// @brief インデックスバッファビュー
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	/// @brief マテリアル用定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialBuffer_;

	/// @brief トランスフォーム用定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> transformBuffer_;

	/// @brief マテリアルデータ
	struct Material {
		Vector4 color;
	};
	Material* materialData_ = nullptr;

	/// @brief トランスフォームデータ
	struct TransformationMatrix {
		Matrix4x4 WVP;
	};
	TransformationMatrix* transformData_ = nullptr;

	/// @brief 頂点数
	static constexpr UINT kVertexCount = 24;

	/// @brief インデックス数
	static constexpr UINT kIndexCount = 36;
};
