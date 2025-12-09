#include "GekitotsuModel.h"

#ifdef _DEBUG
#include <imgui.h>
#endif

void GekitotsuModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);

	// 初期位置とスケールの設定
	transform_.translate = { 0.0f, -1.9f, -59.9f };
	baseScale_ = { 1.0f, 1.0f, 2.0f };
	transform_.scale = baseScale_;
	transform_.TransferMatrix();
}

void GekitotsuModel::Update() {
	// トランスフォームを更新
	transform_.TransferMatrix();
}

void GekitotsuModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画（マテリアルの色はそのまま使用）
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

void GekitotsuModel::SetColor(const Vector4& color) {
	if (model_ && model_->GetMaterialManager()) {
		model_->GetMaterialManager()->SetColor(color);
	}
}

Vector4 GekitotsuModel::GetColor() const {
	if (model_ && model_->GetMaterialManager()) {
		return model_->GetMaterialManager()->GetColor();
	}
	return { 1.0f, 1.0f, 1.0f, 1.0f };
}
