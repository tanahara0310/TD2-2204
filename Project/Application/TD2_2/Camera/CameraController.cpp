#include "CameraController.h"
#include "MathCore.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Math/Easing/EasingUtil.h"
#include "Engine/Utility/Random/RandomGenerator.h"
#include <algorithm>
#include <cmath>

#ifdef _DEBUG
#include <imgui.h>
#endif

using namespace MathCore;

void CameraController::Initialize(Camera* camera, GameObject* object1, GameObject* object2)
{
	camera_ = camera;
	object1_ = object1;
	object2_ = object2;

	if (!camera_ || !object1_ || !object2_) {
		return;
	}

	// 初期位置を計算
	targetPosition_ = CalculateTargetPosition();
	float objectDistance = CalculateObjectDistance();
	currentDistance_ = CalculateCameraDistance(objectDistance);
	currentCameraPos_ = CalculateCameraPosition(targetPosition_, currentDistance_);

	// カメラに初期設定を適用
	camera_->SetTranslate(currentCameraPos_);
	camera_->SetRotate(CalculateCameraRotation());
}

void CameraController::Update()
{
	if (!camera_ || !object1_ || !object2_) {
		return;
	}

	float deltaTime = GameUtils::GetDeltaTime();

	// シェイクの更新
	UpdateShake(deltaTime);

	// ターゲット位置を計算（2つのオブジェクトの中点）
	Vector3 newTargetPos = CalculateTargetPosition();

	// オブジェクト間距離を計算
	float objectDistance = CalculateObjectDistance();

	// カメラの目標距離を計算
	float targetDistance = CalculateCameraDistance(objectDistance);

	// スムーズな補間速度を適用（デルタタイムベース）
	float lerpFactor = std::clamp(1.0f - std::exp(-smoothSpeed_ * deltaTime), 0.0f, 1.0f);

	// EaseOutQuad を使用して減速カーブを適用（より滑らかな停止）
	float easedFactor = EasingUtil::Apply(lerpFactor, EasingUtil::Type::EaseOutQuad);

	// 注視点の補間（スムーズに追従）
	targetPosition_ = EasingUtil::LerpVector3(targetPosition_, newTargetPos, easedFactor);

	// 距離の補間（スムーズにズーム）
	currentDistance_ = EasingUtil::Lerp(currentDistance_, targetDistance, easedFactor, EasingUtil::Type::EaseOutQuad);

	// カメラ位置を計算
	Vector3 targetCameraPos = CalculateCameraPosition(targetPosition_, currentDistance_);

	// カメラ位置の補間（最も重要：急激な移動を防ぐ）
	currentCameraPos_ = EasingUtil::LerpVector3(currentCameraPos_, targetCameraPos, easedFactor);

	// シェイクオフセットを適用
	Vector3 finalCameraPos = {
		currentCameraPos_.x + shakeOffset_.x,
		currentCameraPos_.y + shakeOffset_.y,
		currentCameraPos_.z + shakeOffset_.z
	};

	// カメラに適用
	camera_->SetTranslate(finalCameraPos);
	camera_->SetRotate(CalculateCameraRotation());
}

void CameraController::StartShake(float duration, float magnitude, float frequency, float damping)
{
	shakeTimer_.Start(duration, false);
	shakeMagnitude_ = magnitude;
	shakeFrequency_ = frequency;
	shakeDamping_ = damping;
	shakeTime_ = 0.0f;
}

void CameraController::StartShake(ShakeIntensity intensity)
{
	// プリセットパラメータの設定（継続時間も含む）
	switch (intensity) {
	case ShakeIntensity::Small:
		// 軽い揺れ: 短時間（0.3秒）、小さい振幅、高周波数、速い減衰
		StartShake(0.3f, 0.1f, 25.0f, 0.85f);
		break;

	case ShakeIntensity::Medium:
		// 中程度の揺れ: 中時間（0.5秒）、中程度の振幅、中周波数、中速減衰
		StartShake(0.5f, 0.3f, 20.0f, 0.8f);
		break;

	case ShakeIntensity::Large:
		// 激しい揺れ: 長時間（0.8秒）、大きい振幅、低周波数、遅い減衰
		StartShake(0.8f, 0.6f, 15.0f, 0.75f);
		break;
	}
}

void CameraController::StopShake()
{
	shakeTimer_.Stop();
	shakeOffset_ = { 0.0f, 0.0f, 0.0f };
	shakeTime_ = 0.0f;
}

bool CameraController::IsShaking() const
{
	return shakeTimer_.IsActive();
}

void CameraController::UpdateShake(float deltaTime)
{
	if (!shakeTimer_.IsActive()) {
		shakeOffset_ = { 0.0f, 0.0f, 0.0f };
		return;
	}

	// タイマーの更新
	shakeTimer_.Update(deltaTime);
	shakeTime_ += deltaTime;

	// シェイクが終了したらオフセットをリセット
	if (shakeTimer_.IsFinished()) {
		shakeOffset_ = { 0.0f, 0.0f, 0.0f };
		return;
	}

	// シェイクオフセットを計算
	shakeOffset_ = CalculateShakeOffset();
}

Vector3 CameraController::CalculateShakeOffset() const
{
	if (!shakeTimer_.IsActive()) {
		return { 0.0f, 0.0f, 0.0f };
	}

	// 進行度（0.0～1.0）
	float progress = shakeTimer_.GetProgress();

	// 減衰カーブを適用（指数関数的減衰）
	float dampingFactor = std::pow(1.0f - progress, 1.0f / (1.0f - shakeDamping_));

	// 現在の振幅
	float currentMagnitude = shakeMagnitude_ * dampingFactor;

	// ランダムな方向ベクトルを生成
	auto& random = RandomGenerator::GetInstance();
	
	// パーリンノイズ風の滑らかなランダム値
	float angleX = shakeTime_ * shakeFrequency_ * 2.0f;
	float angleY = shakeTime_ * shakeFrequency_ * 2.5f;
	float angleZ = shakeTime_ * shakeFrequency_ * 3.0f;

	// 三角関数を組み合わせて滑らかな揺れを生成
	float offsetX = std::sin(angleX) * std::cos(angleY * 0.5f) * currentMagnitude;
	float offsetY = std::cos(angleY) * std::sin(angleZ * 0.3f) * currentMagnitude;
	float offsetZ = std::sin(angleZ) * std::cos(angleX * 0.7f) * currentMagnitude * 0.5f;

	// ランダムなノイズを少し加える
	offsetX += random.GetFloat(-1.0f, 1.0f) * currentMagnitude * 0.1f;
	offsetY += random.GetFloat(-1.0f, 1.0f) * currentMagnitude * 0.1f;
	offsetZ += random.GetFloat(-1.0f, 1.0f) * currentMagnitude * 0.05f;

	return { offsetX, offsetY, offsetZ };
}

void CameraController::SetTargets(GameObject* object1, GameObject* object2)
{
	object1_ = object1;
	object2_ = object2;
}

Vector3 CameraController::CalculateTargetPosition() const
{
	if (!object1_ || !object2_) {
		return { 0.0f, 0.0f, 0.0f };
	}

	// 2つのオブジェクトの中点を計算
	Vector3 pos1 = object1_->GetWorldPosition();
	Vector3 pos2 = object2_->GetWorldPosition();

	return {
		(pos1.x + pos2.x) * 0.5f,
		(pos1.y + pos2.y) * 0.5f,
		(pos1.z + pos2.z) * 0.5f
	};
}

float CameraController::CalculateObjectDistance() const
{
	if (!object1_ || !object2_) {
		return 0.0f;
	}

	Vector3 pos1 = object1_->GetWorldPosition();
	Vector3 pos2 = object2_->GetWorldPosition();

	// 2つのオブジェクト間の距離を計算
	Vector3 diff = {
		pos2.x - pos1.x,
		pos2.y - pos1.y,
		pos2.z - pos1.z
	};

	return Vector::Length(diff);
}

float CameraController::CalculateHorizontalDistance() const
{
	if (!object1_ || !object2_) {
		return 0.0f;
	}

	Vector3 pos1 = object1_->GetWorldPosition();
	Vector3 pos2 = object2_->GetWorldPosition();

	// X軸方向の距離のみを計算（横幅）
	return std::abs(pos2.x - pos1.x);
}

float CameraController::CalculateVerticalDistance() const
{
	if (!object1_ || !object2_) {
		return 0.0f;
	}

	Vector3 pos1 = object1_->GetWorldPosition();
	Vector3 pos2 = object2_->GetWorldPosition();

	// Y軸方向の距離のみを計算（縦幅）
	return std::abs(pos2.y - pos1.y);
}

float CameraController::CalculateRequiredDistance(float objectDistance, float horizontalDistance, float verticalDistance) const
{
	// マージン距離を適用
	if (objectDistance < marginDistance_) {
		objectDistance = marginDistance_;
	}
	if (horizontalDistance < marginDistance_) {
		horizontalDistance = marginDistance_;
	}
	if (verticalDistance < marginDistance_) {
		verticalDistance = marginDistance_;
	}

	// 前後方向の必要距離を計算
	float depthDistance = objectDistance * distanceScale_;

	// 視野角の計算
	float halfFovY = kFovY * 0.5f;
	float halfFovX = std::atan(std::tan(halfFovY) * kAspectRatio);

	// 画面パディングを考慮した有効視野
	float effectiveHalfFovX = halfFovX * (1.0f - screenPadding_);
	float effectiveHalfFovY = halfFovY * (1.0f - screenPadding_);

	// 横幅を画面内に収めるために必要な距離
	float requiredHorizontalDistance = (horizontalDistance * 0.5f) / std::tan(effectiveHalfFovX);

	// 縦幅を画面内に収めるために必要な距離
	float requiredVerticalDistance = (verticalDistance * 0.5f) / std::tan(effectiveHalfFovY);

	// 俯角を考慮した補正
	float cosAngle = std::cos(pitchAngle_);
	float sinAngle = std::sin(pitchAngle_);

	// 横幅の補正（俯角により見える横幅が変わる）
	if (cosAngle > 0.01f) {
		requiredHorizontalDistance /= cosAngle;
	}

	// 縦幅の補正（俯角により見える縦幅が変わる）
	// カメラが斜めから見るため、縦方向により多くの距離が必要
	if (cosAngle > 0.01f) {
		// 俯角の影響を考慮した縦方向の補正
		float verticalCorrection = 1.0f + (sinAngle * 0.5f);
		requiredVerticalDistance = requiredVerticalDistance * verticalCorrection / cosAngle;
	}

	// 前後・横幅・縦幅の全てを考慮して最大値を採用
	float requiredDistance = depthDistance;
	if (requiredHorizontalDistance > requiredDistance) {
		requiredDistance = requiredHorizontalDistance;
	}
	if (requiredVerticalDistance > requiredDistance) {
		requiredDistance = requiredVerticalDistance;
	}

	// 最小・最大距離でクランプ
	return std::clamp(requiredDistance, minDistance_, maxDistance_);
}

float CameraController::CalculateCameraDistance(float objectDistance) const
{
	// 横幅と縦幅も計算
	float horizontalDistance = CalculateHorizontalDistance();
	float verticalDistance = CalculateVerticalDistance();

	// 必要な距離を計算（前後・横幅・縦幅すべてを考慮）
	return CalculateRequiredDistance(objectDistance, horizontalDistance, verticalDistance);
}

Vector3 CameraController::CalculateCameraPosition(const Vector3& targetPos, float distance) const
{
	// カメラの方向ベクトルを計算（俯角を考慮）
	float cosAngle = std::cos(pitchAngle_);
	float sinAngle = std::sin(pitchAngle_);

	// カメラをターゲットの後方上方に配置
	Vector3 cameraPos;
	cameraPos.x = targetPos.x;
	cameraPos.y = targetPos.y + heightOffset_ + distance * sinAngle;
	cameraPos.z = targetPos.z - distance * cosAngle;

	return cameraPos;
}

Vector3 CameraController::CalculateCameraRotation() const
{
	// カメラの回転（俯角のみ、X軸回転）
	return { pitchAngle_, 0.0f, 0.0f };
}

#ifdef _DEBUG
void CameraController::DrawImGui()
{
	if (ImGui::Begin("Camera Controller")) {
		ImGui::Text("=== Camera Parameters ===");

		// 距離設定
		ImGui::DragFloat("Min Distance", &minDistance_, 0.1f, 1.0f, 50.0f);
		ImGui::DragFloat("Max Distance", &maxDistance_, 0.1f, 1.0f, 100.0f);
		ImGui::DragFloat("Distance Scale", &distanceScale_, 0.01f, 0.5f, 5.0f);
		ImGui::DragFloat("Margin Distance", &marginDistance_, 0.1f, 0.0f, 20.0f);

		ImGui::Separator();

		// カメラ位置・角度
		ImGui::DragFloat("Height Offset", &heightOffset_, 0.1f, -10.0f, 20.0f);
		ImGui::SliderAngle("Pitch Angle", &pitchAngle_, 0.0f, 90.0f);

		ImGui::Separator();

		// 画面パディング
		ImGui::SliderFloat("Screen Padding", &screenPadding_, 0.0f, 0.4f, "%.2f");
		ImGui::TextWrapped("画面端からの余白（0.15 = 15%%）");

		ImGui::Separator();

		// スムーズ設定
		ImGui::DragFloat("Smooth Speed", &smoothSpeed_, 0.1f, 0.1f, 20.0f);
		ImGui::TextWrapped("推奨値: 3.0-8.0 (低いほど滑らか、高いほど反応が速い)");

		ImGui::Separator();

		// カメラシェイク設定
		if (ImGui::CollapsingHeader("Camera Shake")) {
			ImGui::Text("状態: %s", IsShaking() ? "シェイク中" : "停止中");

			if (IsShaking()) {
				float progress = shakeTimer_.GetProgress();
				ImGui::ProgressBar(progress, ImVec2(-1, 0), "進行度");
				ImGui::Text("残り時間: %.2f秒", shakeTimer_.GetRemainingTime());
			}

			ImGui::Separator();
			ImGui::Text("プリセット版:");
			ImGui::TextWrapped("ボタンをクリックするだけでシェイクが開始されます");

			if (ImGui::Button("小シェイク (0.3秒)", ImVec2(150, 0))) {
				StartShake(ShakeIntensity::Small);
			}
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "軽い衝撃用");

			if (ImGui::Button("中シェイク (0.5秒)", ImVec2(150, 0))) {
				StartShake(ShakeIntensity::Medium);
			}
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "通常攻撃用");

			if (ImGui::Button("大シェイク (0.8秒)", ImVec2(150, 0))) {
				StartShake(ShakeIntensity::Large);
			}
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "強力攻撃用");

			ImGui::Separator();
			ImGui::Text("カスタム版:");

			static float customDuration = 1.0f;
			static float customMagnitude = 0.3f;
			static float customFrequency = 20.0f;
			static float customDamping = 0.8f;

			ImGui::DragFloat("継続時間", &customDuration, 0.1f, 0.1f, 5.0f);
			ImGui::DragFloat("揺れの大きさ", &customMagnitude, 0.01f, 0.0f, 2.0f);
			ImGui::DragFloat("周波数", &customFrequency, 1.0f, 1.0f, 60.0f);
			ImGui::SliderFloat("減衰率", &customDamping, 0.0f, 1.0f);

			if (ImGui::Button("カスタムシェイク開始", ImVec2(200, 0))) {
				StartShake(customDuration, customMagnitude, customFrequency, customDamping);
			}

			ImGui::Separator();

			if (ImGui::Button("シェイク停止", ImVec2(100, 0))) {
				StopShake();
			}
		}

		ImGui::Separator();

		// 現在の状態表示
		ImGui::Text("=== Current Status ===");
		ImGui::Text("Target Position: (%.2f, %.2f, %.2f)",
			targetPosition_.x, targetPosition_.y, targetPosition_.z);
		ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)",
			currentCameraPos_.x, currentCameraPos_.y, currentCameraPos_.z);
		
		if (IsShaking()) {
			ImGui::Text("Shake Offset: (%.3f, %.3f, %.3f)",
				shakeOffset_.x, shakeOffset_.y, shakeOffset_.z);
		}

		ImGui::Text("Current Distance: %.2f", currentDistance_);

		if (object1_ && object2_) {
			float objDist = CalculateObjectDistance();
			float horzDist = CalculateHorizontalDistance();
			float vertDist = CalculateVerticalDistance();
			ImGui::Text("Object Distance (3D): %.2f", objDist);
			ImGui::Text("Horizontal Distance (X): %.2f", horzDist);
			ImGui::Text("Vertical Distance (Y): %.2f", vertDist);
			
			ImGui::Separator();
			ImGui::Text("Aspect Ratio: 16:9 (%.2f)", kAspectRatio);
			ImGui::Text("FOV Y: %.2f rad (%.1f deg)", kFovY, kFovY * 180.0f / 3.14159f);
		}
	}
	ImGui::End();
}
#endif
