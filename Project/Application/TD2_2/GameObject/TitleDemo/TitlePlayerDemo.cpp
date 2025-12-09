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

		// 距離が十分離れている場合のみ追跡移動
		if (distance > 0.1f) {
			transform_.translate.x += direction.x * moveSpeed_ * deltaTime;
			transform_.translate.y += direction.y * moveSpeed_ * deltaTime;
			transform_.translate.z += direction.z * moveSpeed_ * deltaTime;
		}

		// 追跡開始時に累積回転をリセット
		if (!wasChasing_) {
			accumulatedRotation_ = 0.0f;
		}

		// 累積回転を更新
		accumulatedRotation_ += rotationSpeed_ * deltaTime;

		// 移動方向（X成分）に応じて基本回転を決定し、累積回転を加算
		float baseRotationY;
		float baseRotationZ;
		if (direction.x > 0.0f) {
			// +X方向（右）に移動
			baseRotationY = -std::numbers::pi_v<float> / 2.0f;
			baseRotationZ = -0.15f;
		} else {
			// -X方向（左）に移動
			baseRotationY = std::numbers::pi_v<float> / 2.0f;
			baseRotationZ = 0.15f;
		}

		// 基本回転に累積回転を加算
		transform_.rotate.y = baseRotationY + accumulatedRotation_;
		transform_.rotate.z = baseRotationZ;
		
		wasChasing_ = true;
	} else {
		// 通常移動モード（X軸方向）
		transform_.translate.x += moveSpeed_ * moveDirection_ * deltaTime;

		// 移動方向に応じて回転を設定
		if (moveDirection_ > 0.0f) {
			transform_.rotate.y = -std::numbers::pi_v<float> / 2.0f;   // +X方向（右）= -90度
			transform_.rotate.z = -0.15f; // 前かがみ
		} else {
			transform_.rotate.y = std::numbers::pi_v<float> / 2.0f;    // -X方向（左）= 90度
			transform_.rotate.z = 0.15f; // 前かがみ（符号を反転）
		}
		
		wasChasing_ = false;
		accumulatedRotation_ = 0.0f;
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
