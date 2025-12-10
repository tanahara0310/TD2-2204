#include "GekitotsuModel.h"
#include "Engine/Math/Easing/EasingUtil.h"

#ifdef _DEBUG
#include <imgui.h>
#endif

void GekitotsuModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);

	// 目標位置とスケールの保存
	targetPosition_ = { 0.0f, -1.9f, -59.9f };
	targetScale_ = { 1.0f, 1.0f, 2.0f };
	baseScale_ = targetScale_;
	
	// 初期位置は目標位置、スケールは0
	transform_.translate = targetPosition_;
	transform_.scale = startScale_; // スケール0から開始
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	
	// シェーディングモードをトゥーンに設定
	if (model_ && model_->GetMaterialManager()) {
		model_->GetMaterialManager()->SetShadingMode(3); // 3 = Toon
	}
	
	transform_.TransferMatrix();
}

void GekitotsuModel::Update() {
	float deltaTime = 1.0f / 60.0f;
	
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
	}
	
	// トランスフォームを更新
	transform_.TransferMatrix();
}

void GekitotsuModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画（マテリアルの色はそのまま使用）
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

void GekitotsuModel::SetColor(const Vector4& color) {
	if (model_ && model_->GetMaterialManager()) {
		model_->GetMaterialManager()->SetColor(color);
	}
}

Vector4 GekitotsuModel::GetColor() const {
	if (model_ && model_->GetMaterialManager()) {
		return model_->GetMaterialManager()->GetColor();
	}
	return { 1.0f, 1.0f, 1.0f, 1.0f };
}

void GekitotsuModel::StartIntroAnimation(float delayTime) {
	isAnimating_ = true;
	
	if (delayTime > 0.0f) {
		isDelaying_ = true;
		delayTimer_ = 0.0f;
		delayDuration_ = delayTime;
	} else {
		isDelaying_ = false;
		animationTimer_.Start(kAnimationDuration, false);
	}
	
	// 開始位置は目標位置、スケールは0
	transform_.translate = targetPosition_;
	transform_.scale = startScale_;
}

void GekitotsuModel::SkipIntroAnimation() {
	if (!isAnimating_ && !isDelaying_) {
		return;
	}
	
	// アニメーションを即座に終了
	isAnimating_ = false;
	isDelaying_ = false;
	transform_.translate = targetPosition_;
	transform_.scale = targetScale_;
}

void GekitotsuModel::UpdateIntroAnimation(float deltaTime) {
	animationTimer_.Update(deltaTime);
	
	float t = animationTimer_.GetProgress();
	
	if (t >= 1.0f) {
		// アニメーション終了
		isAnimating_ = false;
		transform_.translate = targetPosition_;
		transform_.scale = targetScale_;
		return;
	}
	
	// 位置は固定
	transform_.translate = targetPosition_;
	
	// スケールを0から目標スケールまで拡大（EaseOutBack - 少しオーバーシュートして戻る）
	float easedT = EasingUtil::Apply(t, EasingUtil::Type::EaseOutBack);
	transform_.scale.x = EasingUtil::Lerp(startScale_.x, targetScale_.x, easedT);
	transform_.scale.y = EasingUtil::Lerp(startScale_.y, targetScale_.y, easedT);
	transform_.scale.z = EasingUtil::Lerp(startScale_.z, targetScale_.z, easedT);
}
