#include "ToTitleModel.h"

void ToTitleModel::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
	// 基底クラスの初期化を呼び出す
	GameObject::Initialize(std::move(model), texture);

	transform_.translate = {4.0f, -4.0f, -47.0f};

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

	// Y軸回転（クォータニオンによる増分回転を適用）
	//{
	//	// 回転速度（ラジアン / 秒） -- 必要に応じて調整
	//	const float kYawSpeed = 1.0f; // 1 rad/s
	//	float angleDelta = kYawSpeed * deltaTime;

	//	// 増分クォータニオンを作成して既存の回転に合成する
	//	Quaternion dq = MathCore::QuaternionMath::MakeRotateAxisAngle({0.0f, 1.0f, 0.0f}, angleDelta);
	//	transform_.quaternionRotate = MathCore::QuaternionMath::Multiply(dq, transform_.quaternionRotate);

	//	// 数値安定化のため正規化
	//	transform_.quaternionRotate = MathCore::QuaternionMath::Normalize(transform_.quaternionRotate);
	//}

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