#include "YameruModel.h"

void YameruModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);

	// 初期位置とスケールの設定
	transform_.translate = { -2.4f, -5.9f, -57.6f };
	transform_.scale = { 1.0f, 1.0f, 1.0f };
	transform_.TransferMatrix();
}

void YameruModel::Update() {
	// トランスフォームを更新
	transform_.TransferMatrix();
}

void YameruModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}
