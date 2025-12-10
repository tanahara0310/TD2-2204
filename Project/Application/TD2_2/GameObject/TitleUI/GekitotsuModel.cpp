#include "GekitotsuModel.h"
#include "Engine/Math/Easing/EasingUtil.h"
#include "../../Utility/GameUtils.h"
#include <cmath>

#ifdef _DEBUG
#include <imgui.h>
#endif

namespace {
	// カスタムEaseOutBack関数（大きくオーバーシュートする）
	float CustomEaseOutBackLarge(float t) {
		const float c1 = 6.5f; // 標準は1.70158f、これを大きくするとオーバーシュートが増える
		const float c3 = c1 + 1.0f;
		return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
	}
}

void GekitotsuModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);

	// 目標位置とスケールの保存
	targetPosition_ = { 0.0f, -1.9f, -59.9f };
	targetScale_ = { 1.0f, 1.0f, 2.0f };
	baseScale_ = targetScale_;
	
	// 初期位置と回転を設定
	transform_.translate = targetPosition_;
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	
	// イントロアニメーション用に初期スケールを0に設定
	transform_.scale = { 0.0f, 0.0f, 0.0f };
	isIntroPlaying_ = false;
	
	// シェーディングモードをトゥーンに設定
	if (model_ && model_->GetMaterialManager()) {
		model_->GetMaterialManager()->SetShadingMode(3); // 3 = Toon
	}
	
	transform_.TransferMatrix();
}

void GekitotsuModel::Update() {
	// イントロアニメーションの更新
	if (isIntroPlaying_) {
		float deltaTime = GameUtils::GetDeltaTime();
		if (deltaTime <= 0.0f) {
			deltaTime = 1.0f / 60.0f;
		}
		introTimer_.Update(deltaTime);
		
		// カスタムEaseOutBackで大きくオーバーしてから目標スケールに戻る
		float t = introTimer_.GetProgress();
		float easedT = CustomEaseOutBackLarge(t);
		
		transform_.scale = {
			easedT * targetScale_.x,
			easedT * targetScale_.y,
			easedT * targetScale_.z
		};
		
		if (introTimer_.IsFinished()) {
			isIntroPlaying_ = false;
			transform_.scale = targetScale_;
		}
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

void GekitotsuModel::StartIntroAnimation() {
	isIntroPlaying_ = true;
	introTimer_.Start(kIntroDuration, false);
	transform_.scale = { 0.0f, 0.0f, 0.0f };
}
