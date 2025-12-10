#include "YameruModel.h"
#include "Engine/Math/Easing/EasingUtil.h"
#include "../../Utility/GameUtils.h"
#include <cmath>

void YameruModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);

	// 目標位置とスケールの保存
	targetPosition_ = { 0.0f, -7.0f, -60.9f };
	targetScale_ = { 0.7f, 0.7f, 2.0f };
	baseScale_ = { 0.7f, 0.7f, 2.0f };
	startPosition_ = { 30.0f, -7.0f, -60.9f }; // 画面右端
	
	// イントロアニメーション用に初期位置を画面右端に設定
	transform_.translate = startPosition_;
	transform_.scale = targetScale_;
	breathTimer_ = 0.0f;
	isSelected_ = false;
	isIntroPlaying_ = false;
	transform_.TransferMatrix();
}

void YameruModel::Update() {
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}
	
	// イントロアニメーションの更新
	if (isIntroPlaying_) {
		introTimer_.Update(deltaTime);
		
		// EaseOutBackで目標位置へ移動
		float t = introTimer_.GetProgress();
		transform_.translate = EasingUtil::LerpVector3(
			startPosition_,
			targetPosition_,
			t,
			EasingUtil::Type::EaseOutBack
		);
		
		if (introTimer_.IsFinished()) {
			isIntroPlaying_ = false;
			transform_.translate = targetPosition_;
		}
	}
	
	// 呼吸アニメーションの更新（イントロ完了後のみ）
	if (!isIntroPlaying_) {
		UpdateBreathingAnimation(deltaTime);
	}
	
	// トランスフォームを更新
	transform_.TransferMatrix();
}

void YameruModel::UpdateBreathingAnimation(float deltaTime) {
	// 決定演出中は呼吸アニメーションをスキップ（外部からスケールが制御される）
	if (isConfirming_) {
		return;
	}
	
	if (isSelected_) {
		// 選択中は呼吸アニメーション
		breathTimer_ += deltaTime * kBreathSpeed;
		
		// sin波を0.0～1.0の範囲に変換（ベーススケールから拡大方向のみ）
		float breathWave = (std::sin(breathTimer_) + 1.0f) * 0.5f; // 0.0～1.0
		float breathScale = kBaseScale + breathWave * kBreathAmplitude;
		
		transform_.scale = {
			baseScale_.x * breathScale,
			baseScale_.y * breathScale,
			baseScale_.z * breathScale
		};
	} else {
		// 非選択時は通常スケールより少し小さく（0.9倍）
		breathTimer_ = 0.0f;
		static constexpr float kNonSelectedScale = 0.9f;
		transform_.scale = {
			baseScale_.x * kNonSelectedScale,
			baseScale_.y * kNonSelectedScale,
			baseScale_.z * kNonSelectedScale
		};
	}
}

void YameruModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

void YameruModel::StartIntroAnimation() {
	isIntroPlaying_ = true;
	introTimer_.Start(kIntroDuration, false);
	transform_.translate = startPosition_;
}
