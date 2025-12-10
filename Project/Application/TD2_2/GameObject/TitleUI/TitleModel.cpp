#include "TitleModel.h"
#include "Engine/Math/Easing/EasingUtil.h"
#include <numbers>

void TitleModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);

	// 目標位置とスケールの保存
	targetPosition_ = { 0.0f, -4.0f, -60.9f };
	targetScale_ = { 1.4f, 1.4f, 2.0f };
	baseScale_ = targetScale_;
	
	// 初期位置を画面上に設定
	transform_.translate = startPosition_;
	transform_.scale = startScale_;
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	
	// シェーディングモードをトゥーンに設定
	if (model_ && model_->GetMaterialManager()) {
		model_->GetMaterialManager()->SetShadingMode(3); // 3 = Toon
	}
	
	transform_.TransferMatrix();
}

void TitleModel::Update() {
	float deltaTime = 1.0f / 60.0f;
	
	// イントロアニメーション中の更新
	if (isAnimating_) {
		UpdateIntroAnimation(deltaTime);
	}
	
	// トランスフォームを更新
	transform_.TransferMatrix();
}

void TitleModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画（マテリアルの色はそのまま使用）
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

void TitleModel::SetColor(const Vector4& color) {
	if (model_ && model_->GetMaterialManager()) {
		model_->GetMaterialManager()->SetColor(color);
	}
}

Vector4 TitleModel::GetColor() const {
	if (model_ && model_->GetMaterialManager()) {
		return model_->GetMaterialManager()->GetColor();
	}
	return { 1.0f, 1.0f, 1.0f, 1.0f };
}

void TitleModel::StartIntroAnimation() {
	isAnimating_ = true;
	animationTimer_.Start(kAnimationDuration, false);
	
	// 開始位置とスケールを設定
	transform_.translate = startPosition_;
	transform_.scale = startScale_;
	startRotation_ = std::numbers::pi_v<float> * 4.0f; // 2回転
}

void TitleModel::UpdateIntroAnimation(float deltaTime) {
	animationTimer_.Update(deltaTime);
	
	float t = animationTimer_.GetProgress();
	
	if (t >= 1.0f) {
		// アニメーション終了
		isAnimating_ = false;
		transform_.translate = targetPosition_;
		transform_.scale = targetScale_;
		transform_.rotate.z = 0.0f;
		return;
	}
	
	// バウンドイージングで降下（EaseOutBounce）
	float bounceT = EasingUtil::Apply(t, EasingUtil::Type::EaseOutBounce);
	transform_.translate.y = EasingUtil::Lerp(startPosition_.y, targetPosition_.y, bounceT);
	transform_.translate.x = targetPosition_.x;
	transform_.translate.z = targetPosition_.z;
	
	// スケールは指数関数的に収束（EaseOutExpo）
	float scaleT = EasingUtil::Apply(t, EasingUtil::Type::EaseOutExpo);
	transform_.scale.x = EasingUtil::Lerp(startScale_.x, targetScale_.x, scaleT);
	transform_.scale.y = EasingUtil::Lerp(startScale_.y, targetScale_.y, scaleT);
	transform_.scale.z = EasingUtil::Lerp(startScale_.z, targetScale_.z, scaleT);
	
	// 回転（EaseOutCubic）
	float rotateT = EasingUtil::Apply(t, EasingUtil::Type::EaseOutCubic);
	transform_.rotate.z = EasingUtil::Lerp(startRotation_, 0.0f, rotateT);
}
