#include "Sprite.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Resource/ResourceFactory.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Graphics/Material/MaterialManager.h"
#include "Engine/Graphics/Render/Sprite/SpriteRenderer.h"
#include "Engine/Graphics/Structs/VertexData.h"
#include "Engine/Graphics/Structs/SpriteMaterial.h"
#include "Engine/Math/Vector/Vector4.h"
#include <cmath>

using namespace MathCore;

// ImGuiのヘッダーをインクルード
#ifdef _DEBUG
#include <imgui.h>
#endif

void Sprite::Initialize(SpriteRenderer* spriteRenderer)
{
	spriteRenderer_ = spriteRenderer;
	// デフォルト値を設定
	Reset();
	// 頂点バッファを作成
	CreateVertexBuffer();
}

void Sprite::Initialize(SpriteRenderer* spriteRenderer, const std::string& textureFilePath)
{
	spriteRenderer_ = spriteRenderer;

	// デフォルト値を設定
	Reset();

	// テクスチャサイズを自動設定
	SetSizeFromTexture(textureFilePath);

	// 頂点バッファを作成
	CreateVertexBuffer();
}

void Sprite::SetSizeFromTexture(const std::string& textureFilePath)
{
	// シングルトンTextureManagerからメタデータを取得
	auto& textureManager = TextureManager::GetInstance();
	DirectX::TexMetadata metadata = textureManager.GetMetadata(textureFilePath);

	// テクスチャの解像度を保存
	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);

	// スケールはデフォルト1.0のまま（テクスチャサイズに対する倍率）
	// scale_ = { 1.0f, 1.0f, 1.0f }; // 既にResetで設定済み
}

void Sprite::CreateVertexBuffer()
{
	if (!spriteRenderer_) return;

	// SpriteRenderer経由でDirectXCommonとResourceFactoryを取得
	DirectXCommon* dxCommon = spriteRenderer_->GetDirectXCommon();
	ResourceFactory* resourceFactory = spriteRenderer_->GetResourceFactory();

	if (!dxCommon || !resourceFactory) return;

	// 個別頂点バッファの生成（4頂点のクワッド）
	vertexResource_ = resourceFactory->CreateBufferResource(
		dxCommon->GetDevice(),
		sizeof(VertexData) * 4);

	// 頂点バッファビューの設定
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 個別インデックスバッファの生成
	indexResource_ = resourceFactory->CreateBufferResource(
		dxCommon->GetDevice(),
		sizeof(uint32_t) * 6);

	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	// インデックスデータ設定（時計回り）
	indexData[0] = 0; indexData[1] = 1; indexData[2] = 2; // 一枚目
	indexData[3] = 1; indexData[4] = 3; indexData[5] = 2; // 二枚目

	indexResource_->Unmap(0, nullptr);

	// インデックスバッファビューの設定
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// 初期頂点データを設定
	UpdateVertexData();
}

void Sprite::UpdateVertexData()
{
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	// アンカーポイントを考慮した頂点位置計算
	// 0-1の正規化座標（アンカーポイント適用済み）
	float left = -anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = -anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;

	// 頂点データ設定（0=左下, 1=左上, 2=右下, 3=右上）
	vertexData[0].position = { left, bottom, 0.0f, 1.0f };   // 左下
	vertexData[0].texcoord = { uvMin_.x, uvMax_.y };          // UV左下
	vertexData[0].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[1].position = { left, top, 0.0f, 1.0f };       // 左上
	vertexData[1].texcoord = { uvMin_.x, uvMin_.y };          // UV左上
	vertexData[1].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[2].position = { right, bottom, 0.0f, 1.0f };   // 右下
	vertexData[2].texcoord = { uvMax_.x, uvMax_.y };          // UV右下
	vertexData[2].normal = { 0.0f, 0.0f, -1.0f };

	vertexData[3].position = { right, top, 0.0f, 1.0f };      // 右上
	vertexData[3].texcoord = { uvMax_.x, uvMin_.y };          // UV右上
	vertexData[3].normal = { 0.0f, 0.0f, -1.0f };

	vertexResource_->Unmap(0, nullptr);
}



void Sprite::Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle)
{
	if (!spriteRenderer_) return;

	// CommandListを一度だけ取得
	auto* commandList = spriteRenderer_->GetDirectXCommon()->GetCommandList();

	// 定数バッファを取得してデータを設定
	size_t bufferIndex = spriteRenderer_->GetAvailableConstantBuffer();

	// マテリアルデータ設定
	auto& materialData = spriteRenderer_->GetMaterialDataPool()[bufferIndex];
	materialData->color = color_;
	materialData->uvTransform = uvTransform_;

	// 実際の描画サイズを計算（テクスチャサイズ × スケール）
	Vector3 actualScale = {
		textureSize_.x * scale_.x,
		textureSize_.y * scale_.y,
		scale_.z
	};

	// 変換行列設定（アンカーポイント補正なし、頂点データ側で対応済み）
	auto& transformData = spriteRenderer_->GetTransformDataPool()[bufferIndex];
	Matrix4x4 worldMatrix = Matrix::MakeAffine(actualScale, rotation_, position_);
	transformData->WVP = spriteRenderer_->CalculateWVPMatrix(position_, actualScale, rotation_);
	transformData->world = worldMatrix;

	// 定数バッファ設定
	commandList->SetGraphicsRootConstantBufferView(0, spriteRenderer_->GetMaterialResource(bufferIndex)->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, spriteRenderer_->GetTransformResource(bufferIndex)->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureHandle);

	// 個別頂点バッファ・インデックスバッファを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);

	// 描画
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::Reset()
{
	position_ = { 0.0f, 0.0f, 0.0f };
	scale_ = { 1.0f, 1.0f, 1.0f };  // デフォルトは1x1（テクスチャサイズに対する倍率）
	rotation_ = { 0.0f, 0.0f, 0.0f };
	textureSize_ = { 1.0f, 1.0f };  // デフォルトテクスチャサイズ
	color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
	uvTransform_ = Matrix::Identity();
	anchorPoint_ = { 0.0f, 0.0f };  // デフォルトを左上に設定
	uvMin_ = { 0.0f, 0.0f };
	uvMax_ = { 1.0f, 1.0f };

	// 頂点データを更新
	if (vertexResource_) {
		UpdateVertexData();
	}
}

void Sprite::SetUVOffset(float offsetX, float offsetY)
{
	uvTransform_ = Matrix::Translation({ offsetX, offsetY, 0.0f });
}

void Sprite::SetUVScale(float scaleX, float scaleY)
{
	uvTransform_ = Matrix::Scale({ scaleX, scaleY, 1.0f });
}

void Sprite::SetUVRotation(float rotation)
{
	uvTransform_ = Matrix::RotationZ(rotation);
}

void Sprite::ResetUVTransform()
{
	uvTransform_ = Matrix::Identity();
}

void Sprite::UpdateUVTransformMatrix(const EulerTransform& uvTransform)
{
	// UV変換行列を再計算（正しい順序でSRT適用）
	Matrix4x4 scaleMatrix = Matrix::Scale(uvTransform.scale);
	Matrix4x4 rotateMatrix = Matrix::RotationZ(uvTransform.rotate.z);
	Matrix4x4 translateMatrix = Matrix::Translation(uvTransform.translate);
	// 正しい順序：Scale -> Rotate -> Translate
	uvTransform_ = Matrix::Multiply(Matrix::Multiply(scaleMatrix, rotateMatrix), translateMatrix);
}

void Sprite::ChangeAnchorKeepingPosition(const Vector2& newAnchor)
{
	Vector2 anchorDiff = { newAnchor.x - anchorPoint_.x, newAnchor.y - anchorPoint_.y };

	// 実際の描画サイズを計算
	float actualWidth = textureSize_.x * scale_.x;
	float actualHeight = textureSize_.y * scale_.y;

	// 精度を考慮した座標調整
	constexpr float changeThreshold = 1e-5f;
	if (std::abs(anchorDiff.x) > changeThreshold) {
		position_.x += anchorDiff.x * actualWidth;
	}
	if (std::abs(anchorDiff.y) > changeThreshold) {
		position_.y += anchorDiff.y * actualHeight;
	}

	anchorPoint_ = newAnchor;
	UpdateVertexData();
}

void Sprite::SetAnchor(const Vector2& anchor)
{
	anchorPoint_ = anchor;
	// 頂点データを更新（アンカーポイント変更を反映）
	if (vertexResource_) {
		UpdateVertexData();
	}
}


void Sprite::SetTextureRect(float texLeft, float texTop, float texWidth, float texHeight,
	const std::string& textureFilePath)
{
	// TextureManagerからメタデータを取得
	auto& textureManager = TextureManager::GetInstance();
	DirectX::TexMetadata metadata = textureManager.GetMetadata(textureFilePath);

	// メタデータの幅・高さを使用してUV座標を計算
	float textureWidth = static_cast<float>(metadata.width);
	float textureHeight = static_cast<float>(metadata.height);

	// ピクセル座標を0-1のUV座標に変換
	uvMin_.x = texLeft / textureWidth;
	uvMin_.y = texTop / textureHeight;
	uvMax_.x = (texLeft + texWidth) / textureWidth;
	uvMax_.y = (texTop + texHeight) / textureHeight;

	// 頂点データを更新
	UpdateVertexData();
}

void Sprite::SetUVRect(float uvLeft, float uvTop, float uvRight, float uvBottom)
{
	// 正規化されたUV座標を直接設定
	uvMin_.x = uvLeft;
	uvMin_.y = uvTop;
	uvMax_.x = uvRight;
	uvMax_.y = uvBottom;

	// 頂点データを更新
	UpdateVertexData();
}

bool Sprite::DrawImGui(const std::string& label, EulerTransform* uvTransform)
{
#ifdef _DEBUG
	bool changed = false;

	//  ID衝突回避：Spriteインスタンスのアドレスを使用してユニークID生成 
	ImGui::PushID(this);

	if (ImGui::CollapsingHeader((label + " Properties").c_str())) {
		ImGui::Text("座標 (Position):");


		// X座標 - より細かい制御と適切な精度
		if (ImGui::DragFloat("X##pos", &position_.x, 0.1f, -FLT_MAX, FLT_MAX, "%.2f")) {
			changed = true;
		}

		// Y座標 - より細かい制御と適切な精度
		if (ImGui::DragFloat("Y##pos", &position_.y, 0.1f, -FLT_MAX, FLT_MAX, "%.2f")) {
			changed = true;
		}

		// Z座標 - より細かい制御と適切な精度
		if (ImGui::DragFloat("Z##pos", &position_.z, 0.1f, -FLT_MAX, FLT_MAX, "%.2f")) {
			changed = true;
		}


		ImGui::Separator();

		// Scale 編集用
		ImGui::Text("スケール (Scale):");
		ImGui::Text("倍率（テクスチャサイズに対する）");
		if (ImGui::DragFloat3("##scale", &scale_.x, 0.01f, 0.0f, 10.0f)) {
			changed = true;
		}

		// デバッグ情報：テクスチャサイズと実際の描画サイズ
		ImGui::Text("テクスチャサイズ: %.0fx%.0f px", textureSize_.x, textureSize_.y);
		Vector2 actualSize = GetActualSize();
		ImGui::Text("実際の描画サイズ: %.0fx%.0f px", actualSize.x, actualSize.y);

		ImGui::Separator();

		// Rotate 編集用
		ImGui::Text("回転 (Rotation):");
		if (ImGui::DragFloat3("##rotate", &rotation_.x, 0.01f, -6.28f, 6.28f)) {
			changed = true;
		}

		ImGui::Separator();

		// Color 編集用
		ImGui::Text("色 (Color):");
		if (ImGui::ColorEdit4("##color", &color_.x)) {
			changed = true;
		}

		//アンカーポイント編集用
		ImGui::Separator();
		ImGui::Text("アンカーポイント (Anchor Point):");

		//アンカーポイント変更時の座標ワープを防ぐ
		Vector2 prevAnchor = anchorPoint_;
		if (ImGui::DragFloat2("##anchor", &anchorPoint_.x, 0.01f, 0.0f, 1.0f, "%.3f")) {
			//アンカーポイント変更時に表示位置を維持
			Vector2 anchorDiff = { anchorPoint_.x - prevAnchor.x, anchorPoint_.y - prevAnchor.y };

			// 実際の描画サイズを計算（テクスチャサイズ × スケール）
			float actualWidth = textureSize_.x * scale_.x;
			float actualHeight = textureSize_.y * scale_.y;

			// 微小な変化は無視して座標スキップを防ぐ
			constexpr float changeThreshold = 1e-5f;
			if (std::abs(anchorDiff.x) > changeThreshold) {
				position_.x += anchorDiff.x * actualWidth;
			}
			if (std::abs(anchorDiff.y) > changeThreshold) {
				position_.y += anchorDiff.y * actualHeight;
			}
			// 頂点データを更新
			UpdateVertexData();
			changed = true;
		}

		// 便利なアンカーポイント設定ボタン
		if (ImGui::Button("左上##topleft")) {
			ChangeAnchorKeepingPosition({ 0.0f, 0.0f });
			changed = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("中央##center")) {
			ChangeAnchorKeepingPosition({ 0.5f, 0.5f });
			changed = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("右下##bottomright")) {
			ChangeAnchorKeepingPosition({ 1.0f, 1.0f });
			changed = true;
		}

		// UV Transform (オプション)
		if (uvTransform) {
			ImGui::Separator();
			ImGui::Text("UV変換 (UV Transform):");

			if (ImGui::DragFloat3("UV Scale##uvscale", &uvTransform->scale.x, 0.01f, 0.0f, 3.0f)) {
				UpdateUVTransformMatrix(*uvTransform);
				changed = true;
			}
			if (ImGui::DragFloat3("UV Rotate##uvrotate", &uvTransform->rotate.x, 0.01f, -6.28f, 6.28f)) {
				UpdateUVTransformMatrix(*uvTransform);
				changed = true;
			}
			if (ImGui::DragFloat3("UV Translate##uvtranslate", &uvTransform->translate.x, 0.01f, -2.0f, 2.0f)) {
				UpdateUVTransformMatrix(*uvTransform);
				changed = true;
			}

			// 便利なUV操作ボタン
			ImGui::Separator();
			ImGui::Text("UV操作:");

			if (ImGui::Button("UV リセット##resetuv")) {
				ResetUVTransform();
				*uvTransform = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
				changed = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("X反転##flipx")) {
				uvTransform->scale.x = -uvTransform->scale.x;
				UpdateUVTransformMatrix(*uvTransform);
				changed = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Y反転##flipy")) {
				uvTransform->scale.y = -uvTransform->scale.y;
				UpdateUVTransformMatrix(*uvTransform);
				changed = true;
			}
		}

		ImGui::Separator();
		if (ImGui::Button(("リセット##reset" + label).c_str())) {
			Reset();

			if (uvTransform) {
				*uvTransform = { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
				uvTransform_ = Matrix::Identity();
			}
			changed = true;
		}
	}

	// ID衝突回避：PushIDに対応するPopID 
	ImGui::PopID();

	return changed;
#else
	// リリースビルドでは使用されていないパラメータを明示的に無視
	(void)label;
	(void)uvTransform;
	return false;
#endif
}
