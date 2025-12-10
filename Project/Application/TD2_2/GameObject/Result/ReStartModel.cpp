#include "ReStartModel.h"
#include "Engine/Math/Easing/EasingUtil.h"

void ReStartModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化を呼び出す
	GameObject::Initialize(std::move(model), texture);

	transform_.translate = {-4.0f, -4.5f, -47.0f};

	transform_.TransferMatrix();
}

void ReStartModel::Update() {
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}

	// 呼吸アニメーションの更新
	UpdateBreathingAnimation(deltaTime);

	// 回転アニメーションの更新
	//UpdateRotateAnimation(deltaTime);

	// 選択に応じたZ軸拡大アニメーションの更新
	UpdateScaleAnimation(deltaTime);

	transform_.TransferMatrix();
}

void ReStartModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// モデルの描画
	model_->Draw(transform_, camera, texture_.gpuHandle);
}

void ReStartModel::UpdateBreathingAnimation(float deltaTime) {
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

void ReStartModel::UpdateRotateAnimation(float deltaTime) {
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

void ReStartModel::UpdateScaleAnimation(float deltaTime) {
	// 選択が立ち上がった瞬間にアニメーション開始
	if (isSelected_ && !prevSelected_) {
		isScaleAnimating_ = true;
		scaleTimer_ = 0.0f;
		scaleStartZ_ = transform_.scale.z; // 現在のZスケールを開始値にする
	}

	prevSelected_ = isSelected_;

	if (!isScaleAnimating_) {
		return;
	}

	// タイマー更新
	scaleTimer_ += deltaTime;
	float t = scaleTimer_ / kScaleDuration;
	if (t > 1.0f) t = 1.0f;

	// 山なりのイージング：前半はEaseOutCubicで上昇、後半はEaseInCubicで降下
	float eased = EasingUtil::ApplyComposite(t, EasingUtil::Type::EaseOutCubic, EasingUtil::Type::EaseInCubic, 0.5f);

	// bump は0->1->0 の形（山なり）
	float bump = std::sin(3.14159265358979323846f * t);
	float peakFactor = eased * bump;

	float peakZ = baseScale_.z * kScalePeakMultiplier;
	float newZ = scaleStartZ_ + (peakZ - scaleStartZ_) * peakFactor;

	// Apply new Z scale while preserving X/Y from current scale
	transform_.scale.z = newZ;

	// 終了判定
	if (scaleTimer_ >= kScaleDuration) {
		isScaleAnimating_ = false;
		// Reset scale.z to base to avoid drift (keep breathing effect if any)
		transform_.scale.z = baseScale_.z;
	}
}