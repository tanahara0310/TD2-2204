#include "StartModel.h"
#include "../../Utility/GameUtils.h"
#include "Engine/Math/Easing/EasingUtil.h"
#include <cmath>

void StartModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);
	
	// 目標位置とスケールの保存
	targetPosition_ = { 0.0f, -5.8f, -60.9f };
	targetScale_ = { 0.7f, 0.7f, 2.0f };
	baseScale_ = { 0.7f, 0.7f, 2.0f };
	
	// 初期位置を画面外（左）に設定
	transform_.translate = startPosition_;
	transform_.scale = targetScale_;
	breathTimer_ = 0.0f;
	isSelected_ = false;
	transform_.TransferMatrix();
}

void StartModel::Update() {
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}
	
	// 遅延中の処理
	if (isDelaying_) {
		delayTimer_ += deltaTime;
		if (delayTimer_ >= delayDuration_) {
			isDelaying_ = false;
			animationTimer_.Start(kAnimationDuration, false);
		}
		transform_.TransferMatrix();
		return;
	}
	
	// イントロアニメーション中の更新
	if (isAnimating_) {
		UpdateIntroAnimation(deltaTime);
	} else {
		// 呼吸アニメーションの更新
		UpdateBreathingAnimation(deltaTime);
	}
	
	// トランスフォームを更新
	transform_.TransferMatrix();
}

void StartModel::UpdateBreathingAnimation(float deltaTime) {
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

void StartModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

void StartModel::StartIntroAnimation(float delayTime) {
	isAnimating_ = true;
	
	if (delayTime > 0.0f) {
		isDelaying_ = true;
		delayTimer_ = 0.0f;
		delayDuration_ = delayTime;
	} else {
		isDelaying_ = false;
		animationTimer_.Start(kAnimationDuration, false);
	}
	
	// 開始位置を画面外左に設定
	transform_.translate = startPosition_;
	transform_.scale = targetScale_;
}

void StartModel::UpdateIntroAnimation(float deltaTime) {
	animationTimer_.Update(deltaTime);
	
	float t = animationTimer_.GetProgress();
	
	if (t >= 1.0f) {
		// アニメーション終了
		isAnimating_ = false;
		transform_.translate = targetPosition_;
		baseScale_ = targetScale_; // baseScaleを更新
		
		// 選択状態に応じたスケールを設定
		if (isSelected_) {
			// 選択されている場合は呼吸アニメーションの初期値（ベーススケール）
			transform_.scale = baseScale_;
		} else {
			// 非選択の場合は縮小したスケール
			static constexpr float kNonSelectedScale = 0.9f;
			transform_.scale = {
				baseScale_.x * kNonSelectedScale,
				baseScale_.y * kNonSelectedScale,
				baseScale_.z * kNonSelectedScale
			};
		}
		return;
	}
	
	// 左から右にスライドイン（EaseOutBack）
	float easedT = EasingUtil::Apply(t, EasingUtil::Type::EaseOutBack);
	transform_.translate.x = EasingUtil::Lerp(startPosition_.x, targetPosition_.x, easedT);
	transform_.translate.y = targetPosition_.y;
	transform_.translate.z = targetPosition_.z;
	
	// スケールも選択状態に応じて設定
	if (isSelected_) {
		// 選択されている場合は通常スケール
		transform_.scale = targetScale_;
	} else {
		// 非選択の場合は縮小したスケール
		static constexpr float kNonSelectedScale = 0.9f;
		transform_.scale = {
			targetScale_.x * kNonSelectedScale,
			targetScale_.y * kNonSelectedScale,
			targetScale_.z * kNonSelectedScale
		};
	}
}
