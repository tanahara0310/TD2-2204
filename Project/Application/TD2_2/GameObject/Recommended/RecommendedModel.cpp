#include "RecommendedModel.h"
#include "../../Utility/GameUtils.h"
#include <algorithm>

void RecommendedModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture, const Vector3& position) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);
	
	// 目標位置を保存
	targetPosition_ = position;
	
	// 開始位置（左側にオフセット）
	startPosition_ = position;
	startPosition_.x += kOffsetX;
	
	// 初期位置とスケールの設定（最初は見えない位置に配置）
	transform_.translate = startPosition_;
	transform_.scale = { 1.0f, 1.0f, 1.0f };
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	transform_.TransferMatrix();
	
	// アニメーションの初期化
	appearTimer_ = 0.0f;
	isAnimating_ = false;
}

void RecommendedModel::StartAppearAnimation(float delay) {
	appearDelay_ = delay;
	appearTimer_ = -delay; // 遅延時間分マイナスに設定
	isAnimating_ = true;
}

void RecommendedModel::Update() {
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}
	
	// 出現アニメーションの更新
	UpdateAppearAnimation(deltaTime);
	
	// トランスフォームを更新
	transform_.TransferMatrix();
}

void RecommendedModel::UpdateAppearAnimation(float deltaTime) {
	if (!isAnimating_) {
		return;
	}
	
	appearTimer_ += deltaTime;
	
	// 遅延時間中は何もしない
	if (appearTimer_ < 0.0f) {
		transform_.translate = startPosition_;
		return;
	}
	
	// アニメーション時間内
	if (appearTimer_ < kAppearDuration) {
		// 進行度を計算（0.0 〜 1.0）
		float progress = appearTimer_ / kAppearDuration;
		
		// イージング（ease-out）を適用
		float easedProgress = 1.0f - (1.0f - progress) * (1.0f - progress);
		
		// 開始位置から目標位置へ補間
		transform_.translate.x = startPosition_.x + (targetPosition_.x - startPosition_.x) * easedProgress;
		transform_.translate.y = startPosition_.y + (targetPosition_.y - startPosition_.y) * easedProgress;
		transform_.translate.z = startPosition_.z + (targetPosition_.z - startPosition_.z) * easedProgress;
	}
	else {
		// アニメーション完了
		transform_.translate = targetPosition_;
		isAnimating_ = false;
	}
}

void RecommendedModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}
	
	// アニメーション開始前は描画しない
	if (isAnimating_ && appearTimer_ < 0.0f) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}
