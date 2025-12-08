#include "GekitotsuModel.h"

void GekitotsuModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);

	// 初期位置とスケールの設定
	transform_.translate = { -2.4f, -2.7f, -59.8f };
	baseScale_ = { 1.0f, 1.0f, 1.0f };
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

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}
