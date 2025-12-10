#include "TitleEnemyDemo.h"
#include "TitlePlayerDemo.h"
#include "../../Utility/GameUtils.h"
#include "MathCore.h"
#include <numbers>

void TitleEnemyDemo::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化
	GameObject::Initialize(std::move(model), texture);

	// 回転モードをオイラー角に設定
	transform_.SetRotationMode(WorldTransform::RotationMode::Euler);

	// 初期位置の設定
	transform_.translate = initialPosition_;
	transform_.scale = { 2.0f, 2.0f, 2.0f };
	// 初期回転（+X方向 = 90度、前かがみ = Z軸に若干の回転）
	transform_.rotate = { 0.0f, std::numbers::pi_v<float> / 2.0f, -0.15f };
	baseRotationY_ = std::numbers::pi_v<float> / 2.0f;
	
	// 速度の初期化
	currentSpeed_ = chaseSpeed_;
	baseSpeed_ = chaseSpeed_;
	
	// プロペラモデルを登録（BossPropellerを使用）
	RegisterModelResource("Enemy1", "Resources/Models/Boss/Boss.obj");
	RegisterModelResource("Enemy2", "Resources/Models/BossPropeller/BossPropeller.obj");
	
	// プロペラの回転アニメーションを開始
	StartModelSwapAnimation("Enemy1", "Enemy2", 0.02f, true);
	
	transform_.TransferMatrix();
}

void TitleEnemyDemo::Update() {
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}

	if (isChasing_ && target_) {
		UpdateChaseMode(deltaTime);
	} else {
		UpdateNormalMode(deltaTime);
	}

	// モデル切り替えアニメーションの更新
	GameObject::UpdateModelSwapAnimation();

	// トランスフォームを更新
	transform_.TransferMatrix();
}

void TitleEnemyDemo::UpdateChaseMode(float deltaTime) {
	// ターゲット位置と方向の計算
	Vector3 targetPos = target_->GetWorldPosition();
	Vector3 direction = targetPos - transform_.translate;
	float distance = MathCore::Vector::Length(direction);

	// 方向ベクトルを正規化
	if (distance > 0.0001f) {
		direction = MathCore::Vector::Normalize(direction);
	}

	// 目標速度を距離に応じて計算
	float targetSpeed = baseSpeed_;
	
	// 追いつく瞬間の演出（最優先）
	if (distance <= catchUpDistance_) {
		targetSpeed = baseSpeed_ * catchUpSpeedBoost_;
		isCatchingUp_ = true;
	}
	// 近距離：減速（追いつきそうな緊張感）
	else if (distance <= closeDistance_) {
		float ratio = distance / closeDistance_; // 0.0～1.0
		targetSpeed = baseSpeed_ * (closeSpeedMultiplier_ + (1.0f - closeSpeedMultiplier_) * ratio);
		isCatchingUp_ = false;
	}
	// 遠距離：加速
	else if (distance >= farDistance_) {
		targetSpeed = baseSpeed_ * farSpeedMultiplier_;
		isCatchingUp_ = false;
	}
	// 中距離：通常速度
	else {
		float ratio = (distance - closeDistance_) / (farDistance_ - closeDistance_); // 0.0～1.0
		targetSpeed = baseSpeed_ * (1.0f + (farSpeedMultiplier_ - 1.0f) * ratio);
		isCatchingUp_ = false;
	}

	// 現在の速度を目標速度に向けて加速/減速
	if (currentSpeed_ < targetSpeed) {
		// 加速
		currentSpeed_ += acceleration_ * deltaTime;
		if (currentSpeed_ > targetSpeed) {
			currentSpeed_ = targetSpeed;
		}
	} else if (currentSpeed_ > targetSpeed) {
		// 減速
		currentSpeed_ -= deceleration_ * deltaTime;
		if (currentSpeed_ < targetSpeed) {
			currentSpeed_ = targetSpeed;
		}
	}

	// 追跡移動
	if (distance > 0.1f) {
		transform_.translate.x += direction.x * currentSpeed_ * deltaTime;
		// Y・Z座標は初期位置を維持
		transform_.translate.y = initialPosition_.y;
		transform_.translate.z = initialPosition_.z;
	}

	// 追跡開始時に回転状態をリセット
	if (!wasChasing_) {
		ResetRotationState();
	}

	// 移動方向に応じた回転を更新
	UpdateRotation(direction.x, deltaTime);
	wasChasing_ = true;
}

void TitleEnemyDemo::UpdateNormalMode(float deltaTime) {
	// 通常移動モードでは基本速度に戻す
	currentSpeed_ = baseSpeed_;
	isCatchingUp_ = false;
	
	// 通常移動（X軸方向）
	transform_.translate.x += currentSpeed_ * moveDirection_ * deltaTime;
	
	// Y・Z座標は初期位置を維持
	transform_.translate.y = initialPosition_.y;
	transform_.translate.z = initialPosition_.z;

	// 移動方向に応じた基本回転を設定
	SetBasicRotation(moveDirection_);
	wasChasing_ = false;
}

void TitleEnemyDemo::UpdateRotation(float directionX, float deltaTime) {
	// 移動方向に応じて基本回転を決定
	float baseRotationZ;
	if (directionX > 0.0f) {
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
		ProcessRotating(deltaTime);
	} else if (rotationState_ == RotationState::Waiting) {
		ProcessWaiting(deltaTime);
	}
	
	transform_.rotate.z = baseRotationZ;
}

void TitleEnemyDemo::ProcessRotating(float deltaTime) {
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
		
		// 回転角度を計算
		float totalRotation = 2.0f * std::numbers::pi_v<float> * rotationCount_;
		float currentRotation = totalRotation * easedT;
		
		transform_.rotate.y = baseRotationY_ + currentRotation;
	}
}

void TitleEnemyDemo::ProcessWaiting(float deltaTime) {
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

void TitleEnemyDemo::SetBasicRotation(float direction) {
	if (direction > 0.0f) {
		transform_.rotate.y = -std::numbers::pi_v<float> / 2.0f;   // +X方向（右）= -90度
		transform_.rotate.z = -0.15f; // 前かがみ
	} else {
		transform_.rotate.y = std::numbers::pi_v<float> / 2.0f;    // -X方向（左）= 90度
		transform_.rotate.z = 0.15f; // 前かがみ（符号を反転）
	}
}

void TitleEnemyDemo::ResetRotationState() {
	rotationState_ = RotationState::Rotating;
	rotationTimer_ = 0.0f;
	waitTimer_ = 0.0f;
}

void TitleEnemyDemo::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

void TitleEnemyDemo::ResetToInitialPosition() {
	transform_.translate = initialPosition_;
	SetBasicRotation(moveDirection_);
	transform_.TransferMatrix();
}
