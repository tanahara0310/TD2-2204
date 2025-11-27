#include "CameraController.h"
#include "MathCore.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Math/Easing/EasingUtil.h"
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

	// ターゲット位置を計算（2つのオブジェクトの中点）
	Vector3 newTargetPos = CalculateTargetPosition();

	// オブジェクト間距離を計算
	float objectDistance = CalculateObjectDistance();

	// カメラの目標距離を計算
	float targetDistance = CalculateCameraDistance(objectDistance);

	// スムーズな補間速度を適用（デルタタイムベース）
	// smoothSpeed_ は通常 5.0f 程度の値
	// これにより 1.0 に達するまでの時間は約 0.2 秒となる
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

	// カメラに適用
	camera_->SetTranslate(currentCameraPos_);
	camera_->SetRotate(CalculateCameraRotation());
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

		// 現在の状態表示
		ImGui::Text("=== Current Status ===");
		ImGui::Text("Target Position: (%.2f, %.2f, %.2f)",
			targetPosition_.x, targetPosition_.y, targetPosition_.z);
		ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)",
			currentCameraPos_.x, currentCameraPos_.y, currentCameraPos_.z);
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
