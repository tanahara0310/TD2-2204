#include "ToTitleModel.h"

void ToTitleModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化を呼び出す
	GameObject::Initialize(std::move(model), texture);

	transform_.translate = {4.0f, -4.5f, -47.0f};

	// クォータニオン回転モードに切り替え（Quaternion を使う）
	transform_.SetRotationMode(WorldTransform::RotationMode::Quaternion);

	transform_.TransferMatrix();
}

void ToTitleModel::Update() {
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}

	// 呼吸アニメーションの更新
	UpdateBreathingAnimation(deltaTime);

	// 回転アニメーションの更新
	UpdateRotateAnimation(deltaTime);

	transform_.TransferMatrix();
}

void ToTitleModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

void ToTitleModel::UpdateBreathingAnimation(float deltaTime) {
	if (isSelected_) {
		// 選択中は呼吸アニメーション
		breathTimer_ += deltaTime * kBreathSpeed;

		// sin波で滑らかな拡縮
		float breathScale = kBaseScale + std::sin(breathTimer_) * kBreathAmplitude;

		transform_.scale = {baseScale_.x * breathScale, baseScale_.y * breathScale, baseScale_.z * breathScale};
	} else {
		// 非選択時は通常スケール
		breathTimer_ = 0.0f;
		transform_.scale = baseScale_;
	}
}

void ToTitleModel::UpdateRotateAnimation(float deltaTime) {
	if (isRotateAnimation_) {
		// まだ打ち上げ開始していなければ初期化
		if (!hasLaunched_) {
			hasLaunched_ = true;
			launchTimer_ = 0.0f;
			startY_ = transform_.translate.y;
			startQuaternion_ = transform_.quaternionRotate;
		}

		// タイマー更新
		launchTimer_ += deltaTime;
		// 正規化された進行
		float t = launchTimer_ / kLaunchDuration;
		if (t > 1.0f)
			t = 1.0f;

		// 上下移動
		const float kPi = 3.14159265358979323846f;
		float verticalEase = std::sin(kPi * t);
		transform_.translate.y = startY_ + kLaunchHeight * verticalEase;

		// 回転
		float angle = kTotalRotation * t;
		Quaternion axisRot = MathCore::QuaternionMath::MakeRotateAxisAngle({0.0f, 1.0f, 0.0f}, angle);
		transform_.quaternionRotate = MathCore::QuaternionMath::Normalize(MathCore::QuaternionMath::Multiply(startQuaternion_, axisRot));

		// 終了処理
		if (launchTimer_ >= kLaunchDuration) {
			transform_.translate.y = startY_;
			transform_.quaternionRotate = startQuaternion_;
		}
	} else {
		// 選択フラグが false の間は次回の発動を許可する
		hasLaunched_ = false;
		launchTimer_ = 0.0f;
	}
}