#include "Voxel.h"
#include <EngineSystem.h>

#ifdef _DEBUG
#include <Windows.h>  // OutputDebugStringW用
#endif

void Voxel::Initialize(ModelResource* modelResource, TextureManager::LoadedTexture texture)
{
	auto engine = GetEngineSystem();
	auto dxCommon = engine->GetComponent<DirectXCommon>();

	if (!modelResource) {
#ifdef _DEBUG
		OutputDebugStringW(L"[ERROR] Voxel: モデルリソースがnullptrです\n");
#endif
		return;
	}

	// モデルの作成（リソースから新しいインスタンスを作成）
	model_ = std::make_unique<Model>();
	model_->Initialize(modelResource);

#ifdef _DEBUG
	if (!model_) {
		OutputDebugStringW(L"[ERROR] Voxel: モデルの作成に失敗しました\n");
	}
#endif

	// トランスフォームの初期化
	transform_.Initialize(dxCommon->GetDevice());

	// テクスチャを設定
	texture_ = texture;

#ifdef _DEBUG
	if (texture_.gpuHandle.ptr == 0) {
		OutputDebugStringW(L"[WARNING] Voxel: テクスチャのGPUハンドルが無効です\n");
	}
#endif
}

void Voxel::Update()
{
	// トランスフォームの更新（親の変換も含めて計算）
	transform_.TransferMatrix();
}

void Voxel::Draw(const ICamera* camera)
{
	if (!model_ || !camera) {
		return;
	}

	// マテリアルに色を設定
	if (auto* materialManager = model_->GetMaterialManager()) {
		materialManager->SetColor(color_);
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

bool Voxel::DrawImGui()
{
	// 親クラスのImGuiを呼び出し
	return GameObject::DrawImGui();
}

void Voxel::SetColor(const Vector4& color)
{
	color_ = color;
}

Vector4 Voxel::GetColor() const
{
	return color_;
}
