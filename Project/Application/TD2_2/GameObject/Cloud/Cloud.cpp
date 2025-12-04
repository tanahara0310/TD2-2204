#include "Cloud.h"
void Cloud::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture, CloudDirection direction) {
	// 基底クラスの初期化を呼び出す
	GameObject::Initialize(std::move(model), texture);

	direction_ = direction;
}

void Cloud::Update() { 
	// 進行方向に移動
	if (direction_ == CloudDirection::LEFT) {
		transform_.translate.x -= velocityX_;
	} else if (direction_ == CloudDirection::RIGHT) {
		transform_.translate.x += velocityX_;
	}

	transform_.TransferMatrix(); 
}

void Cloud::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}
