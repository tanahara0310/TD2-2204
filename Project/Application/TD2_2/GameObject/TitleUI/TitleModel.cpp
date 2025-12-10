#include "TitleModel.h"
#include "Engine/Math/Easing/EasingUtil.h"
#include "../../Utility/GameUtils.h"
#include <numbers>
#include <cmath>

namespace {
	// カスタムEaseOutBack関数（オーバーシュートを少なくする）
	float CustomEaseOutBack(float t) {
		const float c1 = 0.85f; // 標準は1.70158f、これを小さくするとオーバーシュートが減る
		const float c3 = c1 + 1.0f;
		return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
	}
}

void TitleModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);

	// 目標位置とスケールの保存
	targetPosition_ = { 0.0f, -4.0f, -60.9f };
	targetScale_ = { 1.4f, 1.4f, 2.0f };
	baseScale_ = targetScale_;
	startPosition_ = { 0.0f, 20.0f, -60.9f }; // 画面外上
	
	// イントロアニメーション用に初期位置を画面外上に設定
	transform_.translate = startPosition_;
	transform_.scale = targetScale_;
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	isIntroPlaying_ = false;
	
	// シェーディングモードをトゥーンに設定
	if (model_ && model_->GetMaterialManager()) {
		model_->GetMaterialManager()->SetShadingMode(3); // 3 = Toon
	}
	
	transform_.TransferMatrix();
}

void TitleModel::Update() {
	// イントロアニメーションの更新
	if (isIntroPlaying_) {
		float deltaTime = GameUtils::GetDeltaTime();
		if (deltaTime <= 0.0f) {
			deltaTime = 1.0f / 60.0f;
		}
		introTimer_.Update(deltaTime);
		
		// カスタムEaseOutBackで目標をほんの少し通り過ぎてから戻る
		float t = introTimer_.GetProgress();
		float easedT = CustomEaseOutBack(t);
		
		transform_.translate = {
			startPosition_.x + (targetPosition_.x - startPosition_.x) * easedT,
			startPosition_.y + (targetPosition_.y - startPosition_.y) * easedT,
			startPosition_.z + (targetPosition_.z - startPosition_.z) * easedT
		};
		
		if (introTimer_.IsFinished()) {
			isIntroPlaying_ = false;
			transform_.translate = targetPosition_;
		}
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
	isIntroPlaying_ = true;
	introTimer_.Start(kIntroDuration, false);
	transform_.translate = startPosition_;
}
