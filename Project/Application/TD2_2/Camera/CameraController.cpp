#include "CameraController.h"
#include "CinematicSequence.h"
#include "CinematicPresetManager.h"
#include "MathCore.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Math/Easing/EasingUtil.h"
#include "Engine/Utility/Random/RandomGenerator.h"
#include "Engine/Utility/JsonManager/JsonManager.h"
#include <algorithm>
#include <cmath>

#ifdef _DEBUG
#include <imgui.h>
#include "CameraControllerEditor.h"
#endif

using namespace MathCore;

CameraController::~CameraController() {
#ifdef _DEBUG
	// 手動でメモリ解放
	delete editor_;
	editor_ = nullptr;
#endif
}

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
	
	// ステージ境界制限を適用
	if (useStageBounds_) {
		targetPosition_ = ClampTargetToStageBounds(targetPosition_, currentDistance_);
	}
	
	currentCameraPos_ = CalculateCameraPosition(targetPosition_, currentDistance_);

	// カメラに初期設定を適用
	camera_->SetTranslate(currentCameraPos_);
	camera_->SetRotate(CalculateCameraRotation());

#ifdef _DEBUG
	// エディターの初期化
	editor_ = new CameraControllerEditor(this);
#endif
}

void CameraController::Update()
{
	if (!camera_ || !object1_ || !object2_) {
		return;
	}

	float deltaTime = GameUtils::GetDeltaTime();

	// シェイクの更新
	UpdateShake(deltaTime);

	// シーケンスが実行中の場合
	if (activeSequence_ && activeSequence_->IsActive()) {
		UpdateSequence(deltaTime);
		return;
	}

	// カメラ演出が実行中の場合
	if (cinematicActive_) {
		UpdateCinematic(deltaTime);
		return;
	}

	// 通常の追従モード
	UpdateNormalMode();
}

void CameraController::UpdateNormalMode()
{
	float deltaTime = GameUtils::GetDeltaTime();

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

	// 距離の補間（スムーズにズーム）
	float interpolatedDistance = EasingUtil::Lerp(currentDistance_, targetDistance, easedFactor, EasingUtil::Type::EaseOutQuad);

	// 補間後の距離で両者が画面内に収まるかチェックし、収まらない場合はプレイヤーを優先
	if (useStageBounds_) {
		newTargetPos = CalculatePlayerPriorityTargetPosition(newTargetPos, interpolatedDistance);
	}

	// 注視点の補間（スムーズに追従）
	targetPosition_ = EasingUtil::LerpVector3(targetPosition_, newTargetPos, easedFactor);

	// 距離を更新
	currentDistance_ = interpolatedDistance;

	// ステージ境界制限を適用
	if (useStageBounds_) {
		targetPosition_ = ClampTargetToStageBounds(targetPosition_, currentDistance_);
	}

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

	// シェイク適用後もステージ境界を超えないようにクランプ
	if (useStageBounds_) {
		finalCameraPos = ClampCameraToStageBounds(finalCameraPos, currentDistance_);
	}

	// カメラに適用
	camera_->SetTranslate(finalCameraPos);
	camera_->SetRotate(CalculateCameraRotation());
}

void CameraController::UpdateCinematic(float deltaTime)
{
	if (!cinematicActive_) {
		return;
	}

	// タイマー更新
	cinematicTimer_.Update(deltaTime);

	// 演出が終了した場合
	if (cinematicTimer_.IsFinished()) {
		StopCinematic();
		return;
	}

	float progress = cinematicTimer_.GetProgress();

	// イージング適用
	float t = progress;
	if (cinematicConfig_.useEasing) {
		EasingUtil::Type easingType = GetEasingTypeFromString(cinematicConfig_.easingType);
		t = EasingUtil::Apply(progress, easingType);
	}

	Vector3 cameraPos;
	Vector3 cameraRotation;

	switch (cinematicConfig_.type) {
	case CinematicType::FixedPosition:
		// 固定位置
		cameraPos = cinematicConfig_.startPosition;
		cameraRotation = cinematicConfig_.startRotation;
		break;

	case CinematicType::LookAt:
		// 特定位置を注視
		cameraPos = cinematicConfig_.startPosition;
		// 注視点方向を向く回転を計算
		{
			Vector3 direction = Vector::Normalize({
				cinematicConfig_.targetPosition.x - cameraPos.x,
				cinematicConfig_.targetPosition.y - cameraPos.y,
				cinematicConfig_.targetPosition.z - cameraPos.z
			});
			
			float yaw = std::atan2(direction.x, direction.z);
			float pitch = std::asin(-direction.y);
			cameraRotation = { pitch, yaw, 0.0f };
		}
		break;

	case CinematicType::Dolly:
		// 移動演出
		cameraPos = EasingUtil::LerpVector3(
			cinematicConfig_.startPosition,
			cinematicConfig_.endPosition,
			t
		);
		cameraRotation = EasingUtil::LerpVector3(
			cinematicConfig_.startRotation,
			cinematicConfig_.endRotation,
			t
		);
		break;

	case CinematicType::Arc:
		// 円弧移動
		{
			Vector3 midPoint = {
				(cinematicConfig_.startPosition.x + cinematicConfig_.endPosition.x) * 0.5f,
				(cinematicConfig_.startPosition.y + cinematicConfig_.endPosition.y) * 0.5f,
				(cinematicConfig_.startPosition.z + cinematicConfig_.endPosition.z) * 0.5f
			};
			
			float arcHeight = cinematicConfig_.orbitRadius;
			midPoint.y += arcHeight;
			
			// ベジェ曲線風の補間
			Vector3 p0 = cinematicConfig_.startPosition;
			Vector3 p1 = midPoint;
			Vector3 p2 = cinematicConfig_.endPosition;
			
			float s = 1.0f - t;
			cameraPos = {
				s * s * p0.x + 2.0f * s * t * p1.x + t * t * p2.x,
				s * s * p0.y + 2.0f * s * t * p1.y + t * t * p2.y,
				s * s * p0.z + 2.0f * s * t * p1.z + t * t * p2.z
			};
			
			cameraRotation = EasingUtil::LerpVector3(
				cinematicConfig_.startRotation,
				cinematicConfig_.endRotation,
				t
			);
		}
		break;

	case CinematicType::Orbit:
		// 対象の周りを回転
		{
			orbitAngle_ += deltaTime * cinematicConfig_.orbitSpeed;
			
			float radius = cinematicConfig_.orbitRadius;
			cameraPos = {
				cinematicConfig_.targetPosition.x + std::cos(orbitAngle_) * radius,
				cinematicConfig_.targetPosition.y + cinematicConfig_.startPosition.y,
				cinematicConfig_.targetPosition.z + std::sin(orbitAngle_) * radius
			};
			
			// ターゲットを見る回転
			Vector3 direction = Vector::Normalize({
				cinematicConfig_.targetPosition.x - cameraPos.x,
				cinematicConfig_.targetPosition.y - cameraPos.y,
				cinematicConfig_.targetPosition.z - cameraPos.z
			});
			
			float yaw = std::atan2(direction.x, direction.z);
			float pitch = std::asin(-direction.y);
			cameraRotation = { pitch, yaw, 0.0f };
		}
		break;

	default:
		cameraPos = cinematicConfig_.startPosition;
		cameraRotation = cinematicConfig_.startRotation;
		break;
	}

	// シェイクオフセットを適用
	Vector3 finalCameraPos = {
		cameraPos.x + shakeOffset_.x,
		cameraPos.y + shakeOffset_.y,
		cameraPos.z + shakeOffset_.z
	};

	// カメラに適用
	camera_->SetTranslate(finalCameraPos);
	camera_->SetRotate(cameraRotation);
	
	// 現在の状態を更新（演出終了後のスムーズな切り替え用）
	currentCameraPos_ = cameraPos;
	
	// カメラの向きから注視点を計算（通常追従モードへの移行時に使用）
	// ただし、演出中のカメラが通常追従モードの俯角と異なる場合があるため、
	// 演出終了後の最初のフレームで targetPosition_ が急激に変化しないように、
	// 実際のオブジェクトの中点に近い値を使用
	if (object1_ && object2_) {
		// 実際のオブジェクトの中点を計算
		Vector3 objectMidpoint = CalculateTargetPosition();
		
		// 演出中の注視点と実際の中点をブレンド
		// 演出の進行度に応じて、徐々に実際の中点に近づける
		float blendFactor = cinematicTimer_.GetProgress();
		// 演出の後半（50%以降）で徐々に実際の中点に近づける
		if (blendFactor > 0.5f) {
			float transitionT = (blendFactor - 0.5f) * 2.0f; // 0.5～1.0 を 0.0～1.0 にマップ
			transitionT = EasingUtil::Apply(transitionT, EasingUtil::Type::EaseInQuad);
			targetPosition_ = EasingUtil::LerpVector3(
				CalculateLookAtTarget(cameraPos, cameraRotation),
				objectMidpoint,
				transitionT
			);
		} else {
			targetPosition_ = CalculateLookAtTarget(cameraPos, cameraRotation);
		}
	} else {
		targetPosition_ = CalculateLookAtTarget(cameraPos, cameraRotation);
	}
}

void CameraController::StartCinematic(const CinematicConfig& config)
{
	cinematicConfig_ = config;
	cinematicActive_ = true;
	cinematicTimer_.Start(config.duration, false);
	orbitAngle_ = 0.0f;
}

void CameraController::StopCinematic()
{
	if (!cinematicActive_) {
		return;
	}

	cinematicActive_ = false;
	cinematicTimer_.Stop();

	// 演出終了時に通常追従モードへスムーズに移行するため、
	// 現在のカメラ位置と状態を維持したまま、
	// 通常追従モードの計算に必要な情報を適切に初期化

	// 現在のターゲット位置を維持（既に UpdateCinematic で更新済み）
	// targetPosition_ はそのまま使用

	// 現在のカメラ位置も維持（既に UpdateCinematic で更新済み）
	// currentCameraPos_ はそのまま使用

	// 現在の距離を計算し直す（通常追従モードの基準に合わせる）
	// カメラ位置からターゲットへの実際の距離を計算
	Vector3 cameraToTarget = {
		targetPosition_.x - currentCameraPos_.x,
		targetPosition_.y - currentCameraPos_.y,
		targetPosition_.z - currentCameraPos_.z
	};
	
	float actualDistance = Vector::Length(cameraToTarget);
	
	// 俯角を考慮した補正
	// 通常追従モードと同じ計算方法で距離を調整
	float cosAngle = std::cos(pitchAngle_);
	if (cosAngle > 0.01f) {
		// Z軸方向の距離成分から本来の距離を逆算
		currentDistance_ = std::abs(cameraToTarget.z) / cosAngle;
	} else {
		currentDistance_ = actualDistance;
	}

	// 最小・最大距離でクランプ
	currentDistance_ = std::clamp(currentDistance_, minDistance_, maxDistance_);
	
	// 演出終了時の距離が通常追従モードの理想距離から大きく離れている場合、
	// オブジェクト間の距離から理想的なカメラ距離を計算し、
	// 現在の距離をその理想距離に近づける（急激な変化を防ぐ）
	if (object1_ && object2_) {
		float objectDistance = CalculateObjectDistance();
		float idealDistance = CalculateCameraDistance(objectDistance);
		
		// 演出終了時の距離と理想距離の差が大きい場合（5以上）、
		// 中間の値を使用して急激な変化を緩和
		float distanceDiff = std::abs(currentDistance_ - idealDistance);
		if (distanceDiff > 5.0f) {
			// 差が大きいほど、より積極的に理想距離に近づける
			float blendFactor = std::clamp(distanceDiff / 30.0f, 0.3f, 0.7f);
			currentDistance_ = currentDistance_ * (1.0f - blendFactor) + idealDistance * blendFactor;
		}
	}
}

bool CameraController::IsCinematicActive() const
{
	return cinematicActive_;
}

float CameraController::GetCinematicProgress() const
{
	if (!cinematicActive_) {
		return 0.0f;
	}
	return cinematicTimer_.GetProgress();
}

bool CameraController::StartCinematicFromJson(const std::string& jsonPath)
{
	try {
		auto& jsonManager = JsonManager::GetInstance();
		json cinematicData = jsonManager.LoadJson(jsonPath);

		CinematicConfig config;
		
		// タイプの読み込み
		std::string typeStr = JsonManager::SafeGet<std::string>(cinematicData, "type", "None");
		if (typeStr == "FixedPosition") {
			config.type = CinematicType::FixedPosition;
		} else if (typeStr == "LookAt") {
			config.type = CinematicType::LookAt;
		} else if (typeStr == "Dolly") {
			config.type = CinematicType::Dolly;
		} else if (typeStr == "Arc") {
			config.type = CinematicType::Arc;
		} else if (typeStr == "Orbit") {
			config.type = CinematicType::Orbit;
		} else {
			config.type = CinematicType::None;
		}

		// 基本パラメータ
		config.duration = JsonManager::SafeGet<float>(cinematicData, "duration", 3.0f);
		config.startPosition = JsonManager::SafeGetVector3(cinematicData, "startPosition", {0, 0, 0});
		config.endPosition = JsonManager::SafeGetVector3(cinematicData, "endPosition", {0, 0, 0});
		config.targetPosition = JsonManager::SafeGetVector3(cinematicData, "targetPosition", {0, 0, 0});
		config.startRotation = JsonManager::SafeGetVector3(cinematicData, "startRotation", {0, 0, 0});
		config.endRotation = JsonManager::SafeGetVector3(cinematicData, "endRotation", {0, 0, 0});
		config.orbitRadius = JsonManager::SafeGet<float>(cinematicData, "orbitRadius", 10.0f);
		config.orbitSpeed = JsonManager::SafeGet<float>(cinematicData, "orbitSpeed", 1.0f);
		config.useEasing = JsonManager::SafeGet<bool>(cinematicData, "useEasing", true);
		config.easingType = JsonManager::SafeGet<std::string>(cinematicData, "easingType", "EaseInOutQuad");

		StartCinematic(config);
		return true;
	}
	catch (...) {
		return false;
	}
}

bool CameraController::SaveCinematicToJson(const std::string& jsonPath) const
{
	try {
		auto& jsonManager = JsonManager::GetInstance();
		json cinematicData;

		// タイプの保存
		std::string typeStr = "None";
		switch (cinematicConfig_.type) {
		case CinematicType::FixedPosition: typeStr = "FixedPosition"; break;
		case CinematicType::LookAt: typeStr = "LookAt"; break;
		case CinematicType::Dolly: typeStr = "Dolly"; break;
		case CinematicType::Arc: typeStr = "Arc"; break;
		case CinematicType::Orbit: typeStr = "Orbit"; break;
		default: typeStr = "None"; break;
		}
		cinematicData["type"] = typeStr;

		// パラメータの保存
		cinematicData["duration"] = cinematicConfig_.duration;
		cinematicData["startPosition"] = JsonManager::Vector3ToJson(cinematicConfig_.startPosition);
		cinematicData["endPosition"] = JsonManager::Vector3ToJson(cinematicConfig_.endPosition);
		cinematicData["targetPosition"] = JsonManager::Vector3ToJson(cinematicConfig_.targetPosition);
		cinematicData["startRotation"] = JsonManager::Vector3ToJson(cinematicConfig_.startRotation);
		cinematicData["endRotation"] = JsonManager::Vector3ToJson(cinematicConfig_.endRotation);
		cinematicData["orbitRadius"] = cinematicConfig_.orbitRadius;
		cinematicData["orbitSpeed"] = cinematicConfig_.orbitSpeed;
		cinematicData["useEasing"] = cinematicConfig_.useEasing;
		cinematicData["easingType"] = cinematicConfig_.easingType;

		return jsonManager.SaveJson(jsonPath, cinematicData);
	}
	catch (...) {
		return false;
	}
}

Vector3 CameraController::CalculateLookAtTarget(const Vector3& position, const Vector3& rotation) const
{
	// カメラの前方向ベクトルを計算
	float distance = 10.0f; // 適当な距離
	
	float cosPitch = std::cos(rotation.x);
	float sinPitch = std::sin(rotation.x);
	float cosYaw = std::cos(rotation.y);
	float sinYaw = std::sin(rotation.y);
	
	Vector3 forward = {
		sinYaw * cosPitch,
		-sinPitch,
		cosYaw * cosPitch
	};
	
	return {
		position.x + forward.x * distance,
		position.y + forward.y * distance,
		position.z + forward.z * distance
	};
}

EasingUtil::Type CameraController::GetEasingTypeFromString(const std::string& typeStr) const
{
	if (typeStr == "Linear") return EasingUtil::Type::Linear;
	if (typeStr == "EaseInQuad") return EasingUtil::Type::EaseInQuad;
	if (typeStr == "EaseOutQuad") return EasingUtil::Type::EaseOutQuad;
	if (typeStr == "EaseInOutQuad") return EasingUtil::Type::EaseInOutQuad;
	if (typeStr == "EaseInCubic") return EasingUtil::Type::EaseInCubic;
	if (typeStr == "EaseOutCubic") return EasingUtil::Type::EaseOutCubic;
	if (typeStr == "EaseInOutCubic") return EasingUtil::Type::EaseInOutCubic;
	if (typeStr == "EaseInQuart") return EasingUtil::Type::EaseInQuart;
	if (typeStr == "EaseOutQuart") return EasingUtil::Type::EaseOutQuart;
	if (typeStr == "EaseInOutQuart") return EasingUtil::Type::EaseInOutQuart;
	if (typeStr == "EaseInQuint") return EasingUtil::Type::EaseInQuint;
	if (typeStr == "EaseOutQuint") return EasingUtil::Type::EaseOutQuint;
	if (typeStr == "EaseInOutQuint") return EasingUtil::Type::EaseInOutQuint;
	if (typeStr == "EaseInBack") return EasingUtil::Type::EaseInBack;
	if (typeStr == "EaseOutBack") return EasingUtil::Type::EaseOutBack;
	if (typeStr == "EaseInOutBack") return EasingUtil::Type::EaseInOutBack;
	return EasingUtil::Type::EaseInOutQuad; // デフォルト
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
		StartShake(0.3f, 0.3f, 25.0f, 0.90f);
		break;

	case ShakeIntensity::Medium:
		// 中程度の揺れ: 中時間（0.5秒）、中程度の振幅、中周波数、中速減衰
		StartShake(0.5f, 0.5f, 20.0f, 0.8f);
		break;

	case ShakeIntensity::Large:
		// 激しい揺れ: 長時間（0.8秒）、大きい振幅、低周波数、遅い減衰
		StartShake(0.8f, 0.7f, 15.0f, 0.75f);
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

void CameraController::SetStageBounds(float minX, float maxX, float minY, float maxY)
{
	stageBoundsMinX_ = minX;
	stageBoundsMaxX_ = maxX;
	stageBoundsMinY_ = minY;
	stageBoundsMaxY_ = maxY;
	useStageBounds_ = true;
}

Vector3 CameraController::ClampTargetToStageBounds(const Vector3& targetPos, float cameraDistance) const
{
	if (!useStageBounds_) {
		return targetPos;
	}

	Vector3 clampedPos = targetPos;

	// 視野角の計算
	float halfFovY = kFovY * 0.5f;
	float halfFovX = std::atan(std::tan(halfFovY) * kAspectRatio);

	// カメラの俯角を考慮
	float cosAngle = std::cos(pitchAngle_);
	float sinAngle = std::sin(pitchAngle_);

	// カメラからターゲットまでの水平距離
	float horizontalDistance = cameraDistance * cosAngle;

	// X軸方向の可視範囲（左右）
	float visibleHalfWidth = horizontalDistance * std::tan(halfFovX);

	// Y軸方向の可視範囲（上下）
	// カメラが斜めから見るため、上下の見える範囲は異なる
	float effectiveDistance = cameraDistance;
	float visibleHalfHeight = effectiveDistance * std::tan(halfFovY);

	// 可視範囲の実際の高さ（俯角を考慮した補正）
	float actualVisibleTop = visibleHalfHeight * (1.0f + sinAngle * 0.5f);
	float actualVisibleBottom = visibleHalfHeight * (1.0f - sinAngle * 0.5f);

	// ターゲット位置をステージ境界内に制限
	// X軸の制限
	float minTargetX = stageBoundsMinX_ + visibleHalfWidth;
	float maxTargetX = stageBoundsMaxX_ - visibleHalfWidth;
	
	// minが maxを超えないように修正
	if (minTargetX > maxTargetX) {
		float center = (stageBoundsMinX_ + stageBoundsMaxX_) * 0.5f;
		clampedPos.x = center;
	} else {
		clampedPos.x = std::clamp(clampedPos.x, minTargetX, maxTargetX);
	}

	// Y軸の制限（俯角を考慮）
	float minTargetY = stageBoundsMinY_ + actualVisibleBottom;
	float maxTargetY = stageBoundsMaxY_ - actualVisibleTop;
	
	// minが maxを超えないように修正
	if (minTargetY > maxTargetY) {
		float center = (stageBoundsMinY_ + stageBoundsMaxY_) * 0.5f;
		clampedPos.y = center;
	} else {
		clampedPos.y = std::clamp(clampedPos.y, minTargetY, maxTargetY);
	}

	return clampedPos;
}

Vector3 CameraController::ClampCameraToStageBounds(const Vector3& cameraPos, float cameraDistance) const
{
	if (!useStageBounds_) {
		return cameraPos;
	}

	// カメラ位置からターゲット位置を逆算
	float cosAngle = std::cos(pitchAngle_);
	float sinAngle = std::sin(pitchAngle_);

	Vector3 targetPos;
	targetPos.x = cameraPos.x;
	targetPos.y = cameraPos.y - heightOffset_ - cameraDistance * sinAngle;
	targetPos.z = cameraPos.z + cameraDistance * cosAngle;

	// ターゲット位置をステージ境界内に制限
	Vector3 clampedTargetPos = ClampTargetToStageBounds(targetPos, cameraDistance);

	// 制限されたターゲット位置から正しいカメラ位置を再計算
	return CalculateCameraPosition(clampedTargetPos, cameraDistance);
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

Vector3 CameraController::CalculatePlayerPriorityTargetPosition(const Vector3& midpoint, float cameraDistance) const
{
	if (!object1_ || !object2_) {
		return midpoint;
	}

	// プレイヤー（object1_）とボス（object2_）の位置
	Vector3 playerPos = object1_->GetWorldPosition();
	Vector3 bossPos = object2_->GetWorldPosition();

	// 現在のカメラ距離でプレイヤーとボスが画面内に収まるかを確認
	// 視野角の計算
	float halfFovY = kFovY * 0.5f;
	float halfFovX = std::atan(std::tan(halfFovY) * kAspectRatio);

	// 画面パディングを考慮した有効視野
	float effectiveHalfFovX = halfFovX * (1.0f - screenPadding_);
	float effectiveHalfFovY = halfFovY * (1.0f - screenPadding_);

	// カメラの俯角を考慮
	float cosAngle = std::cos(pitchAngle_);
	// 現在のカメラ距離での可視範囲を計算
	float horizontalDistance = cameraDistance * cosAngle;
	float visibleHalfWidth = horizontalDistance * std::tan(effectiveHalfFovX);
	float visibleHalfHeight = cameraDistance * std::tan(effectiveHalfFovY);

	// 中点からプレイヤーとボスへの距離を計算
	Vector3 midpointToPlayer = {
		playerPos.x - midpoint.x,
		playerPos.y - midpoint.y,
		playerPos.z - midpoint.z
	};

	Vector3 midpointToBoss = {
		bossPos.x - midpoint.x,
		bossPos.y - midpoint.y,
		bossPos.z - midpoint.z
	};

	// チャタリング防止のためのマージン（可視範囲の10%）
	float marginRatio = 1.1f;

	// 両者が画面内に収まるかチェック（マージン付き）
	bool playerFitsX = std::abs(midpointToPlayer.x) <= visibleHalfWidth * marginRatio;
	bool bossFitsX = std::abs(midpointToBoss.x) <= visibleHalfWidth * marginRatio;
	bool playerFitsY = std::abs(midpointToPlayer.y) <= visibleHalfHeight * marginRatio;
	bool bossFitsY = std::abs(midpointToBoss.y) <= visibleHalfHeight * marginRatio;

	// 両者が画面内に収まる場合は中点をそのまま返す
	if (playerFitsX && bossFitsX && playerFitsY && bossFitsY) {
		return midpoint;
	}

	// 収まらない場合、プレイヤーを画面内に確実に収めるように調整
	Vector3 playerPriorityTarget = midpoint;

	// X軸方向の調整
	if (!playerFitsX || !bossFitsX) {
		// プレイヤーとボスの中間点から、プレイヤー寄りにシフト
		// プレイヤーを画面の端から余裕を持った位置に配置
		float playerSafeZoneRatio = 0.6f; // プレイヤーを可視範囲の60%の位置に配置
		
		if (midpointToPlayer.x > 0.0f) {
			// プレイヤーが右側にいる
			playerPriorityTarget.x = playerPos.x - visibleHalfWidth * playerSafeZoneRatio;
		} else {
			// プレイヤーが左側にいる
			playerPriorityTarget.x = playerPos.x + visibleHalfWidth * playerSafeZoneRatio;
		}
	}

	// Y軸方向の調整
	if (!playerFitsY || !bossFitsY) {
		// プレイヤーとボスの中間点から、プレイヤー寄りにシフト
		float playerSafeZoneRatio = 0.6f;
		
		if (midpointToPlayer.y > 0.0f) {
			// プレイヤーが上側にいる
			playerPriorityTarget.y = playerPos.y - visibleHalfHeight * playerSafeZoneRatio;
		} else {
			// プレイヤーが下側にいる
			playerPriorityTarget.y = playerPos.y + visibleHalfHeight * playerSafeZoneRatio;
		}
	}

	return playerPriorityTarget;
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

bool CameraController::StartCinematicByName(const std::string& presetName)
{
	auto& presetManager = CinematicPresetManager::GetInstance();
	const auto* config = presetManager.GetPreset(presetName);
	
	if (config) {
		StartCinematic(*config);
		return true;
	}
	
	return false;
}

void CameraController::StartSequence(std::shared_ptr<CinematicSequence> sequence)
{
	// 既存の演出を停止
	StopCinematic();
	
	activeSequence_ = sequence;
	if (activeSequence_) {
		activeSequence_->Start();
	}
}

bool CameraController::StartSequenceByName(const std::string& sequenceName)
{
	auto& presetManager = CinematicPresetManager::GetInstance();
	auto sequence = presetManager.GetSequence(sequenceName);
	
	if (sequence) {
		StartSequence(sequence);
		return true;
	}
	
	return false;
}

bool CameraController::IsSequenceActive() const
{
	return activeSequence_ && activeSequence_->IsActive();
}

void CameraController::StopSequence()
{
	if (activeSequence_) {
		activeSequence_->Stop();
		activeSequence_.reset();
	}
}

int CameraController::GetSequenceCurrentCutIndex() const
{
	if (activeSequence_ && activeSequence_->IsActive()) {
		return activeSequence_->GetCurrentCutIndex();
	}
	return -1;
}

void CameraController::UpdateSequence(float deltaTime)
{
	if (!activeSequence_ || !activeSequence_->IsActive()) {
		return;
	}
	
	// シーケンスの更新
	activeSequence_->Update(deltaTime);
	
	// 現在のカットを取得
	const CinematicCut* currentCut = activeSequence_->GetCurrentCut();
	if (!currentCut) {
		return;
	}
	
	// 現在のカットの演出設定を適用して演出を実行
	// カット切り替わり時に自動的に演出が開始される
	if (!cinematicActive_ || cinematicConfig_.type != currentCut->config.type ||
		cinematicConfig_.startPosition.x != currentCut->config.startPosition.x) {
		// 新しいカットの演出を開始
		StartCinematic(currentCut->config);
	}
	
	// 演出の更新（UpdateCinematicが呼ばれる）
	UpdateCinematic(deltaTime);
	
	// シーケンスが終了したらリセット
	if (!activeSequence_->IsActive()) {
		activeSequence_.reset();
	}
}

#ifdef _DEBUG
void CameraController::DrawImGui()
{
	if (editor_) {
		editor_->DrawImGui();
	}
}
#endif

