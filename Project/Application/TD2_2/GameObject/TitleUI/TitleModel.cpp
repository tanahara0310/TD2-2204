#include "TitleModel.h"

void TitleModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);

	// 初期位置とスケールの設定
	transform_.translate = { -2.8f, -4.0f, -62.3f };
	baseScale_ = { 1.0f, 1.0f, 1.0f };
	transform_.scale = baseScale_;
	transform_.TransferMatrix();
}

void TitleModel::Update() {
	// トランスフォームを更新
	transform_.TransferMatrix();
}

void TitleModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}
