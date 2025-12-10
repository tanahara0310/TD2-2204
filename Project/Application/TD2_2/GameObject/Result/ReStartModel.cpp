#include "ReStartModel.h"
#include <cmath>

void ReStartModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化を呼び出す
	GameObject::Initialize(std::move(model), texture);

	transform_.translate = {-4.0f, -4.5f, -47.0f};

	transform_.scale = {0.8f, 0.8f, 0.8f};
	baseScale_ = transform_.scale;

	transform_.TransferMatrix();
}

void ReStartModel::Update() {
	float deltaTime = GameUtils::GetDeltaTime();
	if (deltaTime <= 0.0f) {
		deltaTime = 1.0f / 60.0f;
	}

	// 非スケールアニメーション時
	if (!isScaleAnimation_) {
		// 呼吸アニメーションの更新
		UpdateBreathingAnimation(deltaTime);
	} else {
		// 回転アニメーションの更新
		UpdateRotateAnimation(deltaTime);
	}

	if (!isSelected_) {
		transform_.scale = {0.5f, 0.5f, 0.5f};
	}

	transform_.TransferMatrix();
}

void ReStartModel::Draw(const ICamera* camera) {
	if (!model_ || !camera) {
		return;
	}

	// カメラ位置を保持（UpdateではICameraが渡らないためDrawで保存）
	lastCameraPos_ = camera->GetPosition();

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
	// イージング付きで Z スケールを素早く拡大して戻すアニメーション（カメラ方向で強度変化）
	if (isScaleAnimation_) {
		// 初回開始処理
		if (!hasScaleLaunched_) {
			hasScaleLaunched_ = true;
			scaleTimer_ = 0.0f;
			// 現在の Z スケールを記録（呼吸等を踏まえた現在値）
			startScaleZ_ = transform_.scale.z;
		}

		// タイマー更新
		scaleTimer_ += deltaTime;
		float t = scaleTimer_ / kScaleDuration;
		if (t > 1.0f) t = 1.0f;

		// イージング関数（シンプルな cubic easing）
		auto easeOutCubic = [](float x) -> float { return 1.0f - std::pow(1.0f - x, 3.0f); };
		auto easeInCubic  = [](float x) -> float { return x * x * x; };

		// ピーク倍率差（1.0 -> kScalePeakMultiplier）
		const float peakDelta = kScalePeakMultiplier - 1.0f;

		// イーズ値（0..1..0 の形）
		float easeVal = 0.0f;
		if (t <= 0.5f) {
			float p = t / 0.5f; // 0..1
			easeVal = easeOutCubic(p); // 0..1
		} else {
			float p = (t - 0.5f) / 0.5f; // 0..1
			easeVal = 1.0f - easeInCubic(p); // 1..0
		}

		// カメラ方向との整合度を計算（0..1）
		// モデルのワールド位置 -> カメラへの単位ベクトル
		Vector3 modelWorldPos = transform_.GetWorldPosition();
		Vector3 toCamera = {
			lastCameraPos_.x - modelWorldPos.x,
			lastCameraPos_.y - modelWorldPos.y,
			lastCameraPos_.z - modelWorldPos.z
		};
		float lenToCam = MathCore::Vector::Length(toCamera);
		float align = 0.0f;
		if (lenToCam > 1e-6f) {
			toCamera = MathCore::Vector::Normalize(toCamera);
			// ローカル前方（Z軸）をワールドに回す
			Vector3 localForward = MathCore::QuaternionMath::RotateVector({0.0f, 0.0f, 1.0f}, transform_.quaternionRotate);
			localForward = MathCore::Vector::Normalize(localForward);
			// 内積で整合（背面は効果を弱める）
			align = MathCore::Vector::Dot(localForward, toCamera);
			if (align < 0.0f) align = 0.0f;
			if (align > 1.0f) align = 1.0f;
		}

		// Z スケールを startScaleZ_ を基準に決定（align と easeVal で重み付け）
		float newZ = startScaleZ_ * (1.0f + peakDelta * easeVal * align);

		// X,Y は基準スケールを維持（必要なら呼吸アニメ等と組み合わせる）
		transform_.scale.x = baseScale_.x;
		transform_.scale.y = baseScale_.y;
		transform_.scale.z = newZ;

		// 終了処理
		if (scaleTimer_ >= kScaleDuration) {
			// 完全に元に戻してフラグリセットし、呼吸等に戻るようにする
			transform_.scale.z = startScaleZ_;
			hasScaleLaunched_ = false;
			scaleTimer_ = 0.0f;
			isScaleAnimation_ = false;
		}
	} else {
		// スケールアニメ無効時は初期化
		hasScaleLaunched_ = false;
		scaleTimer_ = 0.0f;
	}
}