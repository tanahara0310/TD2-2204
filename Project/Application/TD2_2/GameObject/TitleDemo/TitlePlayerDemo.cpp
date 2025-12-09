#include "TitlePlayerDemo.h"
#include "../../Utility/GameUtils.h"
#include "MathCore.h"
#include <numbers>

void TitlePlayerDemo::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);

	// 回転モードをオイラー角に設定
	transform_.SetRotationMode(WorldTransform::RotationMode::Euler);

	// 初期位置の設定
	transform_.translate = initialPosition_;
	transform_.scale = { 2.0f, 2.0f, 2.0f };
	// 初期回転（-X方向 = 90度、前かがみ = Z軸に若干の回転）
	transform_.rotate = { 0.0f, std::numbers::pi_v<float> / 2.0f, 0.15f };
	baseRotationY_ = std::numbers::pi_v<float> / 2.0f;
	transform_.TransferMatrix();
}

void TitlePlayerDemo::Update() {
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}

	if (isChasing_ && target_) {
		// 追跡モード
		Vector3 targetPos = target_->GetWorldPosition();
		Vector3 currentPos = transform_.translate;

		// ターゲットへの方向ベクトルを計算
		Vector3 direction = targetPos - currentPos;
		float distance = MathCore::Vector::Length(direction);

		// 方向ベクトルを正規化（ゼロ距離でも安全に扱う）
		if (distance > 0.0001f) {
			direction = MathCore::Vector::Normalize(direction);
		}

		// 距離が十分離れている場合のみ追跡移動（X座標のみ）
		// 常に初期速度を使用
		if (distance > 0.1f) {
			transform_.translate.x += direction.x * kInitialSpeed_ * deltaTime;
			// Y・Z座標は初期位置を維持
			transform_.translate.y = initialPosition_.y;
			transform_.translate.z = initialPosition_.z;
		}

		// 追跡開始時に状態をリセット
		if (!wasChasing_) {
			rotationState_ = RotationState::Rotating;
			rotationTimer_ = 0.0f;
			waitTimer_ = 0.0f;
		}

		// 移動方向（X成分）に応じて基本回転を決定
		float baseRotationZ;
		if (direction.x > 0.0f) {
			// +X方向（右）に移動
			baseRotationY_ = -std::numbers::pi_v<float> / 2.0f;
			baseRotationZ = -0.15f;
		} else {
			// -X方向（左）に移動
			baseRotationY_ = std::numbers::pi_v<float> / 2.0f;
			baseRotationZ = 0.15f;
		}

		// 回転状態管理
		if (rotationState_ == RotationState::Rotating) {
			// 回転中
			rotationTimer_ += deltaTime;
			
			if (rotationTimer_ >= rotationDuration_) {
				// 回転完了、待機状態へ
				rotationState_ = RotationState::Waiting;
				rotationTimer_ = 0.0f;
				waitTimer_ = 0.0f;
			} else {
				// イージングを適用した回転計算
				float t = rotationTimer_ / rotationDuration_;
				float easedT = EasingUtil::Apply(t, rotationEasing_);
				
				// 回転角度を計算（rotationCount_回転分）
				float totalRotation = 2.0f * std::numbers::pi_v<float> * rotationCount_;
				float currentRotation = totalRotation * easedT;
				
				transform_.rotate.y = baseRotationY_ + currentRotation;
			}
		} else if (rotationState_ == RotationState::Waiting) {
			// 待機中
			waitTimer_ += deltaTime;
			
			if (waitTimer_ >= rotationWaitTime_) {
				// 待機完了、回転状態へ
				rotationState_ = RotationState::Rotating;
				rotationTimer_ = 0.0f;
				waitTimer_ = 0.0f;
			}
			
			// 待機中は基本回転のみ
			transform_.rotate.y = baseRotationY_;
		}
		
		transform_.rotate.z = baseRotationZ;
		wasChasing_ = true;
	} else {
		// 通常移動モード（X軸方向）
		// 常に初期速度を使用
		transform_.translate.x += kInitialSpeed_ * moveDirection_ * deltaTime;
		
		// Y・Z座標は初期位置を維持
		transform_.translate.y = initialPosition_.y;
		transform_.translate.z = initialPosition_.z;

		// 移動方向に応じて回転を設定
		if (moveDirection_ > 0.0f) {
			transform_.rotate.y = -std::numbers::pi_v<float> / 2.0f;   // +X方向（右）= -90度
			transform_.rotate.z = -0.15f; // 前かがみ
		} else {
			transform_.rotate.y = std::numbers::pi_v<float> / 2.0f;    // -X方向（左）= 90度
			transform_.rotate.z = 0.15f; // 前かがみ（符号を反転）
		}
		
		wasChasing_ = false;
	}

	// トランスフォームを更新
	transform_.TransferMatrix();
}

void TitlePlayerDemo::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

void TitlePlayerDemo::ResetToInitialPosition() {
	transform_.translate = initialPosition_;
	// 移動方向に応じて回転を設定
	if (moveDirection_ > 0.0f) {
		transform_.rotate.y = -std::numbers::pi_v<float> / 2.0f;
		transform_.rotate.z = -0.15f; // 前かがみ
	} else {
		transform_.rotate.y = std::numbers::pi_v<float> / 2.0f;
		transform_.rotate.z = 0.15f; // 前かがみ（符号を反転）
	}
	transform_.TransferMatrix();
}
