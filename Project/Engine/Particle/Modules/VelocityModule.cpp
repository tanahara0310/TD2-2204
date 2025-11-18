#include "VelocityModule.h"
#include "../ParticleSystem.h" // Particle構造体のために必要

using namespace MathCore;

void VelocityModule::ApplyInitialVelocity(Particle& particle) {
	if (!enabled_) {
		particle.velocity = velocityData_.startSpeed;
		return;
	}

	Vector3 velocity = velocityData_.startSpeed;

	if (velocityData_.useRandomDirection) {
		velocity = GenerateRandomDirection();
	}

	// ランダム範囲を適用
	Vector3 randomFactor = {
		1.0f + random_.GetFloatSigned() * velocityData_.randomSpeedRange.x,
		1.0f + random_.GetFloatSigned() * velocityData_.randomSpeedRange.y,
		1.0f + random_.GetFloatSigned() * velocityData_.randomSpeedRange.z
	};

	velocity.x *= randomFactor.x * velocityData_.startSpeedMultiplier;
	velocity.y *= randomFactor.y * velocityData_.startSpeedMultiplier;
	velocity.z *= randomFactor.z * velocityData_.startSpeedMultiplier;

	particle.velocity = velocity;
}

void VelocityModule::UpdateVelocity(Particle& particle, float deltaTime) {
	if (!enabled_) {
		return;
	}

	// 基本的な速度更新（重力や抵抗などは別のモジュールで処理）
	// ここでは基本的な移動のみ
	(void)particle;   // 未使用パラメータの警告を抑制
	(void)deltaTime;  // 未使用パラメータの警告を抑制
}

#ifdef _DEBUG
bool VelocityModule::ShowImGui() {
	bool changed = false;

	// 有効/無効の切り替え
	if (ImGui::Checkbox("有効##速度", &enabled_)) {
		changed = true;
	}

	if (!enabled_) {
		ImGui::BeginDisabled();
	}

	changed |= ImGui::DragFloat3("初期速度", &velocityData_.startSpeed.x, 0.1f);
	changed |= ImGui::DragFloat("速度倍率", &velocityData_.startSpeedMultiplier, 0.1f, 0.0f, 10.0f);
	changed |= ImGui::DragFloat3("ランダム速度範囲", &velocityData_.randomSpeedRange.x, 0.1f, 0.0f, 5.0f);
	changed |= ImGui::Checkbox("ランダム方向使用", &velocityData_.useRandomDirection);

	if (!enabled_) {
		ImGui::EndDisabled();
	}

	return changed;
}
#endif

Vector3 VelocityModule::GenerateRandomDirection() {
	Vector3 direction = {
		random_.GetFloatSigned(),
		random_.GetFloatSigned(),
		random_.GetFloatSigned()
	};

	// 正規化
	float length = Vector::Length(direction);
	if (length > 0.0f) {
		direction.x /= length;
		direction.y /= length;
		direction.z /= length;
	} else {
		direction = { 0.0f, 1.0f, 0.0f };
	}

	return direction;
}