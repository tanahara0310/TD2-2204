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
	
	// 初期位置を画面外（左）に設定
	transform_.translate = leftStartPosition_;
	transform_.scale = targetScale_;
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
	
	// 開始位置を画面外左に設定
	transform_.translate = leftStartPosition_;
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
	
	// フェーズを3つに分割
	// フェーズ1 (0.0 ~ 0.4): 左右から高速接近
	// フェーズ2 (0.4 ~ 0.5): 衝突の瞬間（スケール拡大）
	// フェーズ3 (0.5 ~ 1.0): バウンドして定位置に収まる
	
	if (t < 0.4f) {
		// フェーズ1: 左右から衝突するように接近
		float phase1T = t / 0.4f;
		float easedT = EasingUtil::Apply(phase1T, EasingUtil::Type::EaseInCubic);
		
		// 左半分は左から、右半分は右から来るように見せる
		// 実際には1つのモデルなので中心に向かって移動
		float leftX = EasingUtil::Lerp(leftStartPosition_.x, targetPosition_.x - splitOffset_, easedT);
		float rightX = EasingUtil::Lerp(rightStartPosition_.x, targetPosition_.x + splitOffset_, easedT);
		
		// 中間地点を計算（左右の平均）
		transform_.translate.x = (leftX + rightX) * 0.5f;
		transform_.translate.y = targetPosition_.y;
		transform_.translate.z = targetPosition_.z;
		
		// 通常スケール
		transform_.scale = targetScale_;
		
	} else if (t < 0.5f) {
		// フェーズ2: 衝突の瞬間（スケールが瞬間的に拡大）
		float phase2T = (t - 0.4f) / 0.1f;
		
		// 衝突位置に到達
		transform_.translate = targetPosition_;
		
		// スケールを一瞬大きくする（インパクト演出）
		float impactScale = 1.0f + (1.0f - phase2T) * 0.8f; // 最大1.8倍
		transform_.scale.x = targetScale_.x * impactScale;
		transform_.scale.y = targetScale_.y * impactScale;
		transform_.scale.z = targetScale_.z;
		
	} else {
		// フェーズ3: バウンドして定位置に収まる
		float phase3T = (t - 0.5f) / 0.5f;
		
		// 定位置
		transform_.translate = targetPosition_;
		
		// スケールをバウンドさせながら元に戻す
		float bounceT = EasingUtil::Apply(phase3T, EasingUtil::Type::EaseOutBounce);
		float bounceScale = 1.8f + (1.0f - 1.8f) * bounceT; // 1.8倍から1.0倍に
		transform_.scale.x = targetScale_.x * bounceScale;
		transform_.scale.y = targetScale_.y * bounceScale;
		transform_.scale.z = targetScale_.z;
	}
}
