#pragma once

#include "Engine/Graphics/Render/Sprite/SpriteRenderer.h"
#include "MathCore.h"
#include <memory>
#include <d3d12.h>
#include <wrl.h>
#include <string>



class Sprite {
public:
	/// @brief デフォルトコンストラクタ
	Sprite() = default;

	/// @brief 初期化（手動サイズ指定）
	/// @param spriteRenderer スプライトレンダラー
	/// @note デフォルトスケールは1x1。SetScale()で任意のサイズを設定可能
	void Initialize(SpriteRenderer* spriteRenderer);

	/// @brief 初期化（テクスチャサイズ自動設定 - 推奨）
	/// @param spriteRenderer スプライトレンダラー
	/// @param textureFilePath テクスチャファイルパス
	/// @note テクスチャの実際の解像度がスプライトのスケールに自動設定される
	void Initialize(SpriteRenderer* spriteRenderer, const std::string& textureFilePath);


	/// @brief 単体描画
	/// @param commandList コマンドリスト
	/// @param textureHandle テクスチャハンドル
	void Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

	/// @brief ImGuiでスプライト情報を表示・編集
	/// @param label ラベル名
	/// @param uvTransform UV変換用のTransform参照
	/// @return 変更があった場合true
	bool DrawImGui(const std::string& label, EulerTransform* uvTransform = nullptr);

	// === Setter/Getter ===
	void SetPosition(const Vector3& pos) { position_ = pos; }
	Vector3 GetPosition() const { return position_; }

	void SetScale(const Vector3& scale) { scale_ = scale; }
	Vector3 GetScale() const { return scale_; }

	/// @brief テクスチャの実際のサイズを取得（ピクセル）
	/// @return テクスチャサイズ
	Vector2 GetTextureSize() const { return textureSize_; }

	/// @brief 実際の描画サイズを取得（テクスチャサイズ × スケール）
	/// @return 描画サイズ
	Vector2 GetActualSize() const { return { textureSize_.x * scale_.x, textureSize_.y * scale_.y }; }

	void SetRotate(const Vector3& rotate) { rotation_ = rotate; }
	Vector3 GetRotate() const { return rotation_; }

	void SetColor(const Vector4& color) { color_ = color; }
	Vector4 GetColor() const { return color_; }

	void SetUVTransform(const Matrix4x4& uvTransform) { uvTransform_ = uvTransform; }
	Matrix4x4 GetUVTransform() const { return uvTransform_; }

	/// @brief アンカーポイントを設定
	/// @param anchor アンカーポイント (0.0f, 0.0f)=左上、(0.5f, 0.5f)=中央、(1.0f, 1.0f)=右下
	void SetAnchor(const Vector2& anchor);
	Vector2 GetAnchor() const { return anchorPoint_; }


	/// @brief UV座標をオフセット（移動）
	/// @param offsetX X方向のオフセット
	/// @param offsetY Y方向のオフセット
	void SetUVOffset(float offsetX, float offsetY);

	/// @brief UV座標をスケール
	/// @param scaleX X方向のスケール
	/// @param scaleY Y方向のスケール
	void SetUVScale(float scaleX, float scaleY);

	/// @brief UV座標を回転
	/// @param rotation 回転角（ラジアン）
	void SetUVRotation(float rotation);

	/// @brief UV変換をリセット
	void ResetUVTransform();

	/// @brief デフォルト値にリセット
	void Reset();


	/// @brief テクスチャ範囲を設定（TextureManagerのメタデータを自動取得）
	/// @param texLeft テクスチャ左端（ピクセル座標）
	/// @param texTop テクスチャ上端（ピクセル座標）
	/// @param texWidth テクスチャ幅（ピクセル）
	/// @param texHeight テクスチャ高さ（ピクセル）
	/// @param textureFilePath テクスチャファイルパス（メタデータ取得用）
	void SetTextureRect(float texLeft, float texTop, float texWidth, float texHeight,
		const std::string& textureFilePath);

	/// @brief 正規化されたUV座標範囲を直接設定
	/// @param uvLeft UV左端（0.0-1.0）
	/// @param uvTop UV上端（0.0-1.0）
	/// @param uvRight UV右端（0.0-1.0）
	/// @param uvBottom UV下端（0.0-1.0）
	void SetUVRect(float uvLeft, float uvTop, float uvRight, float uvBottom);


private://メンバ関数
	/// @brief テクスチャサイズを自動設定
	/// @param textureFilePath テクスチャファイルパス
	/// @note 既に初期化済みのスプライトに対して後からサイズを設定する場合に使用
	void SetSizeFromTexture(const std::string& textureFilePath);

	/// @brief 頂点バッファを作成
	void CreateVertexBuffer();

	/// @brief 頂点データを更新
	void UpdateVertexData();

	/// @brief UV変換行列を計算（ImGui用ヘルパー）
	/// @param uvTransform UV変換パラメータ
	void UpdateUVTransformMatrix(const EulerTransform& uvTransform);

	/// @brief アンカーポイントを変更し、表示位置を維持（ImGui用ヘルパー）
	/// @param newAnchor 新しいアンカーポイント
	void ChangeAnchorKeepingPosition(const Vector2& newAnchor);

private:// メンバ変数

	SpriteRenderer* spriteRenderer_ = nullptr;

	// トランスフォーム
	Vector3 position_ = { 0.0f, 0.0f, 0.0f };
	Vector3 scale_ = { 1.0f, 1.0f, 1.0f };  // テクスチャサイズに対する倍率
	Vector3 rotation_ = { 0.0f, 0.0f, 0.0f };

	// テクスチャの実際のサイズ（ピクセル）
	Vector2 textureSize_ = { 1.0f, 1.0f };

	// マテリアル
	Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	Matrix4x4 uvTransform_ = MathCore::Matrix::Identity();

	// アンカーポイント（0.0f, 0.0f = 左上、0.5f, 0.5f = 中央、1.0f, 1.0f = 右下）
	Vector2 anchorPoint_ = { 0.0f, 0.0f };

	// 頂点バッファ
	ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	// UV座標範囲
	Vector2 uvMin_ = { 0.0f, 0.0f };
	Vector2 uvMax_ = { 1.0f, 1.0f };
};
