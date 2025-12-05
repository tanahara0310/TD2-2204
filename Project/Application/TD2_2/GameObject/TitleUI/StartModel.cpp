#include "StartModel.h"
#include "../../Utility/GameUtils.h"
#include <cmath>

void StartModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);
	
	// 初期位置とスケールの設定
	transform_.translate = {-2.1f, -4.5f, -57.6f };
	baseScale_ = { 1.0f, 1.0f, 1.0f };
	transform_.scale = baseScale_;
	breathTimer_ = 0.0f;
	isSelected_ = false;
	transform_.TransferMatrix();
}

void StartModel::Update() {
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}
	
	// 呼吸アニメーションの更新
	UpdateBreathingAnimation(deltaTime);
	
	// トランスフォームを更新
	transform_.TransferMatrix();
}

void StartModel::UpdateBreathingAnimation(float deltaTime) {
	if (isSelected_) {
		// 選択中は呼吸アニメーション
		breathTimer_ += deltaTime * kBreathSpeed;
		
		// sin波で滑らかな拡縮
		float breathScale = kBaseScale + std::sin(breathTimer_) * kBreathAmplitude;
		
		transform_.scale = {
			baseScale_.x * breathScale,
			baseScale_.y * breathScale,
			baseScale_.z * breathScale
		};
	} else {
		// 非選択時は通常スケール
		breathTimer_ = 0.0f;
		transform_.scale = baseScale_;
	}
}

void StartModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}
