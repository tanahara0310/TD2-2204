#include "ToTitleModel.h"

void ToTitleModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化を呼び出す
	GameObject::Initialize(std::move(model), texture);

	transform_.translate = {4.0f, -4.0f, -47.0f};

	transform_.TransferMatrix();
}

void ToTitleModel::Update() { 
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}

	// 呼吸アニメーションの更新
	UpdateBreathingAnimation(deltaTime);

	transform_.TransferMatrix(); 
}

void ToTitleModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

void ToTitleModel::UpdateBreathingAnimation(float deltaTime) {
	if (isSelected_) {
		// 選択中は呼吸アニメーション
		breathTimer_ += deltaTime * kBreathSpeed;

		// sin波で滑らかな拡縮
		float breathScale = kBaseScale + std::sin(breathTimer_) * kBreathAmplitude;

		transform_.scale = {baseScale_.x * breathScale, baseScale_.y * breathScale, baseScale_.z * breathScale};
	} else {
		// 非選択時は通常スケール
		breathTimer_ = 0.0f;
		transform_.scale = baseScale_;
	}
}