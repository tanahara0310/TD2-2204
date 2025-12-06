#include "PeriodModel.h"

void PeriodModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化を呼び出す
	GameObject::Initialize(std::move(model), texture);

	transform_.translate = {0.0f, 0.0f, -47.0f};

	transform_.TransferMatrix();
}

void PeriodModel::Update() { transform_.TransferMatrix(); }

void PeriodModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}