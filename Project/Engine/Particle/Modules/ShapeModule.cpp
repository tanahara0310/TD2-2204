#include "ShapeModule.h"
#include "../ParticleSystem.h" // Particle構造体のために必要
#include <cmath>
#include <algorithm>

#ifdef _DEBUG
#include "Engine/Graphics/LineRenderer.h"
#include "Engine/Camera/ICamera.h"
#endif

ShapeModule::ShapeModule() {
	shapeData_.shapeType = ShapeType::Point;
	shapeData_.scale = { 1.0f, 1.0f, 1.0f };
	shapeData_.radius = 1.0f;
	shapeData_.innerRadius = 0.5f;
	shapeData_.height = 2.0f;
	shapeData_.angle = 25.0f;
	shapeData_.randomPositionRange = 0.0f;
	shapeData_.emitFromSurface = false;
	shapeData_.emissionDirection = { 0.0f, 1.0f, 0.0f };
	shapeData_.circlePlane = CirclePlane::XZ;
}

Vector3 ShapeModule::GeneratePosition(const Vector3& emitterPosition) {
	if (!enabled_) {
		return emitterPosition;
	}

	Vector3 position;

	switch (shapeData_.shapeType) {
	case ShapeType::Box:
		position = GenerateBoxPosition(emitterPosition);
		break;
	case ShapeType::Sphere:
		position = GenerateSpherePosition(emitterPosition);
		break;
	case ShapeType::Circle:
		position = GenerateCirclePosition(emitterPosition);
		break;
	case ShapeType::CircleHalf:
		position = GenerateCircleHalfPosition(emitterPosition);
		break;
	case ShapeType::Cone:
		position = GenerateConePosition(emitterPosition);
		break;
	case ShapeType::Hemisphere:
		position = GenerateHemispherePosition(emitterPosition);
		break;
	case ShapeType::Ring:
		position = GenerateRingPosition(emitterPosition);
		break;
	case ShapeType::Line:
		position = GenerateLinePosition(emitterPosition);
		break;
	case ShapeType::Cylinder:
		position = GenerateCylinderPosition(emitterPosition);
		break;
	case ShapeType::Edge:
		position = GenerateEdgePosition(emitterPosition);
		break;
	case ShapeType::Point:
	default:
		position = GeneratePointPosition(emitterPosition);
		break;
	}

	// 追加のランダム位置範囲を適用
	if (shapeData_.randomPositionRange > 0.0f) {
		position.x += random_.GetFloat(-shapeData_.randomPositionRange, shapeData_.randomPositionRange);
		position.y += random_.GetFloat(-shapeData_.randomPositionRange, shapeData_.randomPositionRange);
		position.z += random_.GetFloat(-shapeData_.randomPositionRange, shapeData_.randomPositionRange);
	}

	return position;
}

Vector3 ShapeModule::GeneratePointPosition(const Vector3& emitterPosition) {
	return emitterPosition;
}

Vector3 ShapeModule::GenerateBoxPosition(const Vector3& emitterPosition) {
	Vector3 randomOffset = {
		  random_.GetFloatSigned() * shapeData_.scale.x,
	  random_.GetFloatSigned() * shapeData_.scale.y,
		  random_.GetFloatSigned() * shapeData_.scale.z
	};

	return {
		emitterPosition.x + randomOffset.x,
  emitterPosition.y + randomOffset.y,
		emitterPosition.z + randomOffset.z
	};
}

Vector3 ShapeModule::GenerateSpherePosition(const Vector3& emitterPosition) {
	// 球面上の均等分布のためのランダム点生成
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
	}

	// 半径内の均等分布（表面のみか内部も含むか）
	float r;
	if (shapeData_.emitFromSurface) {
		r = shapeData_.radius;
	} else {
		r = std::cbrt(random_.GetFloat()) * shapeData_.radius;
	}

	return {
		emitterPosition.x + direction.x * r,
		emitterPosition.y + direction.y * r,
 emitterPosition.z + direction.z * r
	};
}

Vector3 ShapeModule::GenerateCirclePosition(const Vector3& emitterPosition) {
	float angle = random_.GetFloat(0.0f, 2.0f * 3.14159f);
	float r = std::sqrt(random_.GetFloat()) * shapeData_.radius;

	// 平面に応じた座標計算
	switch (shapeData_.circlePlane) {
	case CirclePlane::XZ:
		return {
		 emitterPosition.x + std::cos(angle) * r,
		emitterPosition.y,
		  emitterPosition.z + std::sin(angle) * r
		};
	case CirclePlane::XY:
		return {
	 emitterPosition.x + std::cos(angle) * r,
		  emitterPosition.y + std::sin(angle) * r,
			emitterPosition.z
		};
	case CirclePlane::YZ:
		return {
		   emitterPosition.x,
	 emitterPosition.y + std::cos(angle) * r,
		  emitterPosition.z + std::sin(angle) * r
		};
	default:
		return {
	emitterPosition.x + std::cos(angle) * r,
	emitterPosition.y,
			emitterPosition.z + std::sin(angle) * r
		};
	}
}

Vector3 ShapeModule::GenerateConePosition(const Vector3& emitterPosition) {
	float circleAngle = random_.GetFloat(0.0f, 2.0f * 3.14159f);
	float height = random_.GetFloat() * shapeData_.height;
	float coneRadius = height * std::tan(shapeData_.angle * 3.14159f / 180.0f);
	float r = std::sqrt(random_.GetFloat()) * coneRadius;

	return {
		emitterPosition.x + std::cos(circleAngle) * r,
		emitterPosition.y + height,
		emitterPosition.z + std::sin(circleAngle) * r
	};
}

Vector3 ShapeModule::GenerateHemispherePosition(const Vector3& emitterPosition) {
	// 半球面上の均等分布のためのランダム点生成
	Vector3 direction;
	do {
		direction = {
	  random_.GetFloatSigned(),
	   std::abs(random_.GetFloatSigned()), // Y成分は常に正（上半球）
		 random_.GetFloatSigned()
		};
	} while (Vector::Length(direction) > 1.0f);

	// 正規化
	float length = Vector::Length(direction);
	if (length > 0.0f) {
		direction.x /= length;
		direction.y /= length;
		direction.z /= length;
	}

	// 半径内の均等分布
	float r;
	if (shapeData_.emitFromSurface) {
		r = shapeData_.radius;
	} else {
		r = std::cbrt(random_.GetFloat()) * shapeData_.radius;
	}

	return {
		emitterPosition.x + direction.x * r,
		emitterPosition.y + direction.y * r,
		emitterPosition.z + direction.z * r
	};
}

Vector3 ShapeModule::GenerateRingPosition(const Vector3& emitterPosition) {
	float angle = random_.GetFloat(0.0f, 2.0f * 3.14159f);
	float minR = shapeData_.innerRadius;
	float maxR = shapeData_.radius;
	float r = minR + std::sqrt(random_.GetFloat()) * (maxR - minR);

	switch (shapeData_.circlePlane) {
	case CirclePlane::XZ:
		return {
		emitterPosition.x + std::cos(angle) * r,
	  emitterPosition.y,
  emitterPosition.z + std::sin(angle) * r
		};
	case CirclePlane::XY:
		return {
		emitterPosition.x + std::cos(angle) * r,
			emitterPosition.y + std::sin(angle) * r,
emitterPosition.z
		};
	case CirclePlane::YZ:
		return {
	emitterPosition.x,
			   emitterPosition.y + std::cos(angle) * r,
	emitterPosition.z + std::sin(angle) * r
		};
	default:
		return {
	   emitterPosition.x + std::cos(angle) * r,
			emitterPosition.y,
			emitterPosition.z + std::sin(angle) * r
		};
	}
}

Vector3 ShapeModule::GenerateLinePosition(const Vector3& emitterPosition) {
	Vector3 direction = shapeData_.emissionDirection;
	float length = Vector::Length(direction);
	if (length > 0.0f) {
		direction.x /= length;
		direction.y /= length;
		direction.z /= length;
	} else {
		direction = { 0.0f, 1.0f, 0.0f };
	}

	float linePosition = random_.GetFloatSigned() * shapeData_.scale.x;

	return {
	 emitterPosition.x + direction.x * linePosition,
		emitterPosition.y + direction.y * linePosition,
		emitterPosition.z + direction.z * linePosition
	};
}

Vector3 ShapeModule::GenerateCylinderPosition(const Vector3& emitterPosition) {
	float angle = random_.GetFloat(0.0f, 2.0f * 3.14159f);
	float height = random_.GetFloatSigned() * (shapeData_.height * 0.5f);

	float r;
	if (shapeData_.emitFromSurface) {
		if (random_.GetBool(0.8f)) {
			r = shapeData_.radius;
		} else {
			r = std::sqrt(random_.GetFloat()) * shapeData_.radius;
			height = (height > 0) ? (shapeData_.height * 0.5f) : (-shapeData_.height * 0.5f);
		}
	} else {
		r = std::sqrt(random_.GetFloat()) * shapeData_.radius;
	}

	return {
		  emitterPosition.x + std::cos(angle) * r,
	 emitterPosition.y + height,
   emitterPosition.z + std::sin(angle) * r
	};
}

Vector3 ShapeModule::GenerateEdgePosition(const Vector3& emitterPosition) {
	Vector3 direction = {
		random_.GetFloatSigned(),
		random_.GetFloatSigned(),
	  random_.GetFloatSigned()
	};

	float length = Vector::Length(direction);
	if (length > 0.0f) {
		direction.x /= length;
		direction.y /= length;
		direction.z /= length;
	}

	return {
		emitterPosition.x + direction.x * shapeData_.radius,
		emitterPosition.y + direction.y * shapeData_.radius,
		emitterPosition.z + direction.z * shapeData_.radius
	};
}

Vector3 ShapeModule::GenerateCircleHalfPosition(const Vector3& emitterPosition) {
	constexpr float PI = 3.14159265358979323846f;
	bool top = random_.GetBool();

	float minAngle, maxAngle;
	if (top) {
		minAngle = 5.0f * PI / 6.0f;
		maxAngle = 7.0f * PI / 6.0f;
	} else {
		if (random_.GetBool()) {
			minAngle = 11.0f * PI / 6.0f;
			maxAngle = 2.0f * PI;
		} else {
			minAngle = 0.0f;
			maxAngle = PI / 6.0f;
		}
	}

	float angle = random_.GetFloat(minAngle, maxAngle);
	float r = shapeData_.radius;

	switch (shapeData_.circlePlane) {
	case CirclePlane::XZ:
		return {
			emitterPosition.x + std::cos(angle) * r,
	   emitterPosition.y,
  emitterPosition.z + std::sin(angle) * r
		};
	case CirclePlane::XY:
		return {
	   emitterPosition.x + std::cos(angle) * r,
			   emitterPosition.y + std::sin(angle) * r,
	 emitterPosition.z
		};
	case CirclePlane::YZ:
		return {
	   emitterPosition.x,
			emitterPosition.y + std::cos(angle) * r,
		emitterPosition.z + std::sin(angle) * r
		};
	default:
		return {
			emitterPosition.x + std::cos(angle) * r,
		 emitterPosition.y,
	emitterPosition.z + std::sin(angle) * r
		};
	}
}

#ifdef _DEBUG
void ShapeModule::DrawEmitterShape(LineRenderer* lineRenderer, const ICamera* camera,
	const Vector3& emitterPosition) {

	if (!debugDraw_ || !lineRenderer || !camera) {
		return;
	}

	const Vector3 color = { 0.0f, 1.0f, 0.0f };  // 緑色
	const float alpha = 0.8f;

	// 形状ごとにデバッグ描画
	switch (shapeData_.shapeType) {
	case ShapeType::Point:
		// 点：小さな十字を描画
		{
			const float crossSize = 0.1f;
			LineRenderer::Line line1;
			line1.start = { emitterPosition.x - crossSize, emitterPosition.y, emitterPosition.z };
			line1.end = { emitterPosition.x + crossSize, emitterPosition.y, emitterPosition.z };
			line1.color = color;
			line1.alpha = alpha;
			lineRenderer->DrawLine(camera, line1);

			LineRenderer::Line line2;
			line2.start = { emitterPosition.x, emitterPosition.y - crossSize, emitterPosition.z };
			line2.end = { emitterPosition.x, emitterPosition.y + crossSize, emitterPosition.z };
			line2.color = color;
			line2.alpha = alpha;
			lineRenderer->DrawLine(camera, line2);

			LineRenderer::Line line3;
			line3.start = { emitterPosition.x, emitterPosition.y, emitterPosition.z - crossSize };
			line3.end = { emitterPosition.x, emitterPosition.y, emitterPosition.z + crossSize };
			line3.color = color;
			line3.alpha = alpha;
			lineRenderer->DrawLine(camera, line3);
		}
		break;

	case ShapeType::Box:
		// ボックス
		lineRenderer->DrawBox(camera, emitterPosition, shapeData_.scale, color, alpha);
		break;

	case ShapeType::Sphere:
		// 球体
		lineRenderer->DrawSphere(camera, emitterPosition, shapeData_.radius, color, alpha);
		break;

	case ShapeType::Circle:
		// 円
		{
			Vector3 normal;
			switch (shapeData_.circlePlane) {
			case CirclePlane::XZ:
				normal = { 0.0f, 1.0f, 0.0f };  // Y軸（地面に平行）
				break;
			case CirclePlane::XY:
				normal = { 0.0f, 0.0f, 1.0f };  // Z軸（垂直）
				break;
			case CirclePlane::YZ:
				normal = { 1.0f, 0.0f, 0.0f };  // X軸（垂直）
				break;
			default:
				normal = { 0.0f, 1.0f, 0.0f };
				break;
			}
			lineRenderer->DrawCircle(camera, emitterPosition, shapeData_.radius, normal, color, alpha);
		}
		break;

	case ShapeType::Cone:
		// コーン
		lineRenderer->DrawCone(camera, emitterPosition, { 0.0f, 1.0f, 0.0f }, 
			shapeData_.height, shapeData_.angle, color, alpha);
		break;

	case ShapeType::Hemisphere:
		// 半球（球体 + 円）
		lineRenderer->DrawSphere(camera, emitterPosition, shapeData_.radius, color, alpha, 16);
		lineRenderer->DrawCircle(camera, emitterPosition, shapeData_.radius, 
			{ 0.0f, 1.0f, 0.0f }, color, alpha);
		break;

	case ShapeType::Ring:
		// リング（外円 + 内円）
		{
			Vector3 normal;
			switch (shapeData_.circlePlane) {
			case CirclePlane::XZ:
				normal = { 0.0f, 1.0f, 0.0f };
				break;
			case CirclePlane::XY:
				normal = { 0.0f, 0.0f, 1.0f };
				break;
			case CirclePlane::YZ:
				normal = { 1.0f, 0.0f, 0.0f };
				break;
			default:
				normal = { 0.0f, 1.0f, 0.0f };
				break;
			}
			// 外円
			lineRenderer->DrawCircle(camera, emitterPosition, shapeData_.radius, normal, color, alpha);
			// 内円
			lineRenderer->DrawCircle(camera, emitterPosition, shapeData_.innerRadius, normal, color, alpha);
		}
		break;

	case ShapeType::Line:
		// 線分
		{
			Vector3 direction = shapeData_.emissionDirection;
			float length = Vector::Length(direction);
			if (length > 0.0f) {
				direction.x /= length;
				direction.y /= length;
				direction.z /= length;
			} else {
				direction = { 0.0f, 1.0f, 0.0f };
			}

			float halfLength = shapeData_.scale.x * 0.5f;
			Vector3 lineStart = {
				emitterPosition.x - direction.x * halfLength,
				emitterPosition.y - direction.y * halfLength,
				emitterPosition.z - direction.z * halfLength
			};
			Vector3 lineEnd = {
				emitterPosition.x + direction.x * halfLength,
				emitterPosition.y + direction.y * halfLength,
				emitterPosition.z + direction.z * halfLength
			};
			
			LineRenderer::Line lineData;
			lineData.start = lineStart;
			lineData.end = lineEnd;
			lineData.color = color;
			lineData.alpha = alpha;
			lineRenderer->DrawLine(camera, lineData);
		}
		break;

	case ShapeType::Cylinder:
		// 円柱
		lineRenderer->DrawCylinder(camera, emitterPosition, shapeData_.radius, 
			shapeData_.height, { 0.0f, 1.0f, 0.0f }, color, alpha);
		break;

	case ShapeType::Edge:
		// エッジ（球体の表面）
		lineRenderer->DrawSphere(camera, emitterPosition, shapeData_.radius, color, alpha);
		break;

	case ShapeType::CircleHalf:
		// 半円
		{
			Vector3 normal;
			switch (shapeData_.circlePlane) {
			case CirclePlane::XZ:
				normal = { 0.0f, 1.0f, 0.0f };
				break;
			case CirclePlane::XY:
				normal = { 0.0f, 0.0f, 1.0f };
				break;
			case CirclePlane::YZ:
				normal = { 1.0f, 0.0f, 0.0f };
				break;
			default:
				normal = { 0.0f, 1.0f, 0.0f };
				break;
			}
			// 半円（円の半分）
			lineRenderer->DrawCircle(camera, emitterPosition, shapeData_.radius, normal, color, alpha, 16);
			
			// 半円の境界線を描画
			Vector3 circleStart = emitterPosition;
			Vector3 circleEnd = emitterPosition;
			
			switch (shapeData_.circlePlane) {
			case CirclePlane::XZ:
				circleStart = { emitterPosition.x - shapeData_.radius, emitterPosition.y, emitterPosition.z };
				circleEnd = { emitterPosition.x + shapeData_.radius, emitterPosition.y, emitterPosition.z };
				break;
			case CirclePlane::XY:
				circleStart = { emitterPosition.x - shapeData_.radius, emitterPosition.y, emitterPosition.z };
				circleEnd = { emitterPosition.x + shapeData_.radius, emitterPosition.y, emitterPosition.z };
				break;
			case CirclePlane::YZ:
				circleStart = { emitterPosition.x, emitterPosition.y - shapeData_.radius, emitterPosition.z };
				circleEnd = { emitterPosition.x, emitterPosition.y + shapeData_.radius, emitterPosition.z };
				break;
			}
			
			LineRenderer::Line lineData;
			lineData.start = circleStart;
			lineData.end = circleEnd;
			lineData.color = color;
			lineData.alpha = alpha;
			lineRenderer->DrawLine(camera, lineData);
		}
		break;
	}
}

bool ShapeModule::ShowImGui() {
	bool changed = false;

	if (ImGui::Checkbox("有効##形状", &enabled_)) {
		changed = true;
	}

	if (!enabled_) {
		ImGui::BeginDisabled();
	}

	ImGui::Separator();
	ImGui::Text("形状設定");

	static const char* shapeTypeNames[] = {
	"点", "ボックス", "球体", "円", "コーン", "半球", "リング", "ライン", "円柱", "エッジ", "半円"
	};
	int currentShapeType = static_cast<int>(shapeData_.shapeType);
	if (ImGui::Combo("形状タイプ", &currentShapeType, shapeTypeNames, IM_ARRAYSIZE(shapeTypeNames))) {
		shapeData_.shapeType = static_cast<ShapeType>(currentShapeType);
		changed = true;
	}

	// 形状ごとのパラメータ表示
	switch (shapeData_.shapeType) {
	case ShapeType::Point:
		ImGui::TextDisabled("点形状：単一の点から放出");
		break;

	case ShapeType::Box:
		changed |= ImGui::DragFloat3("ボックスサイズ", &shapeData_.scale.x, 0.1f, 0.1f, 20.0f);
		changed |= ImGui::Checkbox("表面のみから放出", &shapeData_.emitFromSurface);
		break;

	case ShapeType::Sphere:
	case ShapeType::Edge:
		changed |= ImGui::DragFloat("半径", &shapeData_.radius, 0.1f, 0.1f, 20.0f);
		if (shapeData_.shapeType == ShapeType::Sphere) {
			changed |= ImGui::Checkbox("表面のみから放出", &shapeData_.emitFromSurface);
		}
		break;

	case ShapeType::Circle:
	case ShapeType::CircleHalf:
		changed |= ImGui::DragFloat("半径", &shapeData_.radius, 0.1f, 0.1f, 20.0f);
		{
			const char* planeNames[] = { "XZ (地面)", "XY (垂直 Z固定)", "YZ (垂直 X固定)" };
			int curPlane = static_cast<int>(shapeData_.circlePlane);
			if (ImGui::Combo("円の平面", &curPlane, planeNames, IM_ARRAYSIZE(planeNames))) {
				shapeData_.circlePlane = static_cast<CirclePlane>(curPlane);
				changed = true;
			}
		}
		break;

	case ShapeType::Cone:
		changed |= ImGui::DragFloat("コーン角度", &shapeData_.angle, 1.0f, 0.0f, 90.0f, "%.1f度");
		changed |= ImGui::DragFloat("高さ", &shapeData_.height, 0.1f, 0.1f, 20.0f);
		break;

	case ShapeType::Hemisphere:
		changed |= ImGui::DragFloat("半径", &shapeData_.radius, 0.1f, 0.1f, 20.0f);
		changed |= ImGui::Checkbox("表面のみから放出", &shapeData_.emitFromSurface);
		break;

	case ShapeType::Ring:
		changed |= ImGui::DragFloat("外径", &shapeData_.radius, 0.1f, 0.1f, 20.0f);
		changed |= ImGui::DragFloat("内径", &shapeData_.innerRadius, 0.1f, 0.0f, shapeData_.radius);
		{
			const char* planeNames[] = { "XZ (地面)", "XY (垂直 Z固定)", "YZ (垂直 X固定)" };
			int curPlane = static_cast<int>(shapeData_.circlePlane);
			if (ImGui::Combo("リングの平面", &curPlane, planeNames, IM_ARRAYSIZE(planeNames))) {
				shapeData_.circlePlane = static_cast<CirclePlane>(curPlane);
				changed = true;
			}
		}
		break;

	case ShapeType::Line:
		changed |= ImGui::DragFloat("長さ", &shapeData_.scale.x, 0.1f, 0.1f, 20.0f);
		changed |= ImGui::DragFloat3("方向", &shapeData_.emissionDirection.x, 0.1f);
		break;

	case ShapeType::Cylinder:
		changed |= ImGui::DragFloat("半径", &shapeData_.radius, 0.1f, 0.1f, 20.0f);
		changed |= ImGui::DragFloat("高さ", &shapeData_.height, 0.1f, 0.1f, 20.0f);
		changed |= ImGui::Checkbox("表面のみから放出", &shapeData_.emitFromSurface);
		break;
	}

	// 追加パラメータ
	ImGui::Separator();
	ImGui::Text("追加パラメータ");
	changed |= ImGui::DragFloat("ランダム位置範囲", &shapeData_.randomPositionRange, 0.01f, 0.0f, 2.0f);

	// デバッグ描画
	ImGui::Separator();
	ImGui::Text("デバッグ表示");
	if (ImGui::Checkbox("形状を表示", &debugDraw_)) {
		changed = true;
	}

	if (!enabled_) {
		ImGui::EndDisabled();
	}

	return changed;
}
#endif
