#include "PlayerPresetModel.h"
#include "../../Utility/GameUtils.h"
#include <cmath>

void PlayerPresetModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture, PresetType presetType, float yPosition) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);
	
	presetType_ = presetType;
	
	// 初期位置とスケールの設定
	// X座標は右側に配置、Y座標は引数で指定、Z座標は奥行き
	transform_.translate = { 8.0f, yPosition, -57.6f };
	// Y軸に55度回転（45度 + 10度）
	transform_.rotate = { 0.0f, std::numbers::pi_v<float> / 4.0f + std::numbers::pi_v<float> / 18.0f, 0.0f };
	transform_.SetRotationMode(WorldTransform::RotationMode::Euler); // オイラー角モードを明示的に設定
	baseScale_ = { 0.8f, 0.8f, 0.8f }; // スケールを半分に
	transform_.scale = baseScale_;
	breathTimer_ = 0.0f;
	isSelected_ = false;
	transform_.TransferMatrix();
}

void PlayerPresetModel::Update() {
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}
	
	// 呼吸アニメーションの更新
	UpdateBreathingAnimation(deltaTime);
	
	// トランスフォームを更新
	transform_.TransferMatrix();
}

void PlayerPresetModel::UpdateBreathingAnimation(float deltaTime) {
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

void PlayerPresetModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}
