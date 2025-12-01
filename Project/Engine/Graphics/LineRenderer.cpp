#include "LineRenderer.h"
#include "Engine/Graphics/Render/Line/LineRendererPipeline.h"
#include "Engine/Camera/ICamera.h"
#include <numbers>

using namespace MathCore;

void LineRenderer::Initialize(LineRendererPipeline* rendererPipeline) {
	rendererPipeline_ = rendererPipeline;
	lines_.clear();
}

void LineRenderer::Update() {
	// ライン管理クラスなので、特に更新処理はない
}

void LineRenderer::Draw(const ICamera* camera) {
	if (!camera || !rendererPipeline_ || lines_.empty()) {
		return;
	}

	// ラインデータを頂点データに変換
	std::vector<LineRendererPipeline::LineVertex> vertices;
	vertices.reserve(lines_.size() * 2);

	for (const auto& line : lines_) {
		// 開始点
		vertices.push_back({ line.start, line.color, line.alpha });
		// 終了点
		vertices.push_back({ line.end, line.color, line.alpha });
	}

	// 頂点バッファを更新
	rendererPipeline_->UpdateVertexBuffer(vertices);

	// WVP行列を設定
	Matrix4x4 view = camera->GetViewMatrix();
	Matrix4x4 proj = camera->GetProjectionMatrix();
	rendererPipeline_->SetWVPMatrix(view, proj);

	// 描画
	rendererPipeline_->DrawLines(rendererPipeline_->GetDirectXCommon()->GetCommandList(),
		static_cast<uint32_t>(vertices.size()));
}

void LineRenderer::AddLine(const Line& line) {
	lines_.push_back(line);
}

void LineRenderer::AddLines(const std::vector<Line>& lines) {
	lines_.insert(lines_.end(), lines.begin(), lines.end());
}

void LineRenderer::DrawLine(const ICamera* camera, const Line& line) {
	if (!camera || !rendererPipeline_) {
		return;
	}

	// 一時的に1本だけ描画
	std::vector<LineRendererPipeline::LineVertex> vertices = {
		{ line.start, line.color, line.alpha },
   { line.end, line.color, line.alpha }
	};

	rendererPipeline_->UpdateVertexBuffer(vertices);

	Matrix4x4 view = camera->GetViewMatrix();
	Matrix4x4 proj = camera->GetProjectionMatrix();
	rendererPipeline_->SetWVPMatrix(view, proj);

	rendererPipeline_->DrawLines(rendererPipeline_->GetDirectXCommon()->GetCommandList(), 2);
}

void LineRenderer::DrawSphere(const ICamera* camera, const Vector3& center, float radius,
	const Vector3& color, float alpha, int segments) {
	if (!camera || !rendererPipeline_) {
		return;
	}

	std::vector<Line> sphereLines;

	// 緯度線を描画（複数の水平円）
	for (int lat = 0; lat <= segments; ++lat) {
		float theta = (static_cast<float>(lat) / segments) * std::numbers::pi_v<float>;
		float sinTheta = std::sin(theta);
		float cosTheta = std::cos(theta);

		for (int lon = 0; lon < segments; ++lon) {
			float phi1 = (static_cast<float>(lon) / segments) * 2.0f * std::numbers::pi_v<float>;
			float phi2 = (static_cast<float>(lon + 1) / segments) * 2.0f * std::numbers::pi_v<float>;

			float sinPhi1 = std::sin(phi1);
			float cosPhi1 = std::cos(phi1);
			float sinPhi2 = std::sin(phi2);
			float cosPhi2 = std::cos(phi2);

			Vector3 p1 = {
			 center.x + radius * sinTheta * cosPhi1,
			  center.y + radius * cosTheta,
		   center.z + radius * sinTheta * sinPhi1
			};

			Vector3 p2 = {
		  center.x + radius * sinTheta * cosPhi2,
		  center.y + radius * cosTheta,
				center.z + radius * sinTheta * sinPhi2
			};

			sphereLines.push_back({ p1, p2, color, alpha });
		}
	}

	// 経度線を描画（縦の線）
	for (int lon = 0; lon < segments; ++lon) {
		float phi = (static_cast<float>(lon) / segments) * 2.0f * std::numbers::pi_v<float>;
		float sinPhi = std::sin(phi);
		float cosPhi = std::cos(phi);

		for (int lat = 0; lat < segments; ++lat) {
			float theta1 = (static_cast<float>(lat) / segments) * std::numbers::pi_v<float>;
			float theta2 = (static_cast<float>(lat + 1) / segments) * std::numbers::pi_v<float>;

			float sinTheta1 = std::sin(theta1);
			float cosTheta1 = std::cos(theta1);
			float sinTheta2 = std::sin(theta2);
			float cosTheta2 = std::cos(theta2);

			Vector3 p1 = {
		  center.x + radius * sinTheta1 * cosPhi,
			center.y + radius * cosTheta1,
				 center.z + radius * sinTheta1 * sinPhi
			};

			Vector3 p2 = {
		 center.x + radius * sinTheta2 * cosPhi,
		center.y + radius * cosTheta2,
	  center.z + radius * sinTheta2 * sinPhi
			};

			sphereLines.push_back({ p1, p2, color, alpha });
		}
	}

	// 一時的に球体のラインを追加
	AddLines(sphereLines);
}

void LineRenderer::Clear() {
	lines_.clear();
}

void LineRenderer::DrawBox(const ICamera* camera, const Vector3& center, const Vector3& size,
	const Vector3& color, float alpha) {
	if (!camera || !rendererPipeline_) {
		return;
	}

	// ボックスの8つの頂点を計算
	Vector3 halfSize = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
	Vector3 vertices[8] = {
		{ center.x - halfSize.x, center.y - halfSize.y, center.z - halfSize.z }, // 0: 左下前
		{ center.x + halfSize.x, center.y - halfSize.y, center.z - halfSize.z }, // 1: 右下前
		{ center.x + halfSize.x, center.y + halfSize.y, center.z - halfSize.z }, // 2: 右上前
		{ center.x - halfSize.x, center.y + halfSize.y, center.z - halfSize.z }, // 3: 左上前
		{ center.x - halfSize.x, center.y - halfSize.y, center.z + halfSize.z }, // 4: 左下後
		{ center.x + halfSize.x, center.y - halfSize.y, center.z + halfSize.z }, // 5: 右下後
		{ center.x + halfSize.x, center.y + halfSize.y, center.z + halfSize.z }, // 6: 右上後
		{ center.x - halfSize.x, center.y + halfSize.y, center.z + halfSize.z }  // 7: 左上後
	};

	std::vector<Line> boxLines;

	// 前面の4辺
	boxLines.push_back({ vertices[0], vertices[1], color, alpha });
	boxLines.push_back({ vertices[1], vertices[2], color, alpha });
	boxLines.push_back({ vertices[2], vertices[3], color, alpha });
	boxLines.push_back({ vertices[3], vertices[0], color, alpha });

	// 後面の4辺
	boxLines.push_back({ vertices[4], vertices[5], color, alpha });
	boxLines.push_back({ vertices[5], vertices[6], color, alpha });
	boxLines.push_back({ vertices[6], vertices[7], color, alpha });
	boxLines.push_back({ vertices[7], vertices[4], color, alpha });

	// 前面と後面を結ぶ4辺
	boxLines.push_back({ vertices[0], vertices[4], color, alpha });
	boxLines.push_back({ vertices[1], vertices[5], color, alpha });
	boxLines.push_back({ vertices[2], vertices[6], color, alpha });
	boxLines.push_back({ vertices[3], vertices[7], color, alpha });

	AddLines(boxLines);
}

void LineRenderer::DrawCircle(const ICamera* camera, const Vector3& center, float radius, 
	const Vector3& normal, const Vector3& color, float alpha, int segments) {
	if (!camera || !rendererPipeline_) {
		return;
	}

	std::vector<Line> circleLines;

	// 法線ベクトルに基づいて円の平面を決定
	Vector3 up = { 0.0f, 1.0f, 0.0f };
	Vector3 right;

	// 法線がほぼ上向きの場合は別の軸を使用
	if (std::abs(normal.y) > 0.999f) {
		right = { 1.0f, 0.0f, 0.0f };
	} else {
		// 外積で右ベクトルを計算
		right = Vector::Normalize(Vector::Cross(up, normal));
	}

	// 上ベクトルを再計算
	up = Vector::Normalize(Vector::Cross(normal, right));

	// 円を描画
	for (int i = 0; i < segments; ++i) {
		float angle1 = (static_cast<float>(i) / segments) * 2.0f * std::numbers::pi_v<float>;
		float angle2 = (static_cast<float>(i + 1) / segments) * 2.0f * std::numbers::pi_v<float>;

		Vector3 p1 = {
			center.x + radius * (std::cos(angle1) * right.x + std::sin(angle1) * up.x),
			center.y + radius * (std::cos(angle1) * right.y + std::sin(angle1) * up.y),
			center.z + radius * (std::cos(angle1) * right.z + std::sin(angle1) * up.z)
		};

		Vector3 p2 = {
			center.x + radius * (std::cos(angle2) * right.x + std::sin(angle2) * up.x),
			center.y + radius * (std::cos(angle2) * right.y + std::sin(angle2) * up.y),
			center.z + radius * (std::cos(angle2) * right.z + std::sin(angle2) * up.z)
		};

		circleLines.push_back({ p1, p2, color, alpha });
	}

	AddLines(circleLines);
}

void LineRenderer::DrawCone(const ICamera* camera, const Vector3& apex, const Vector3& direction,
	float height, float angle, const Vector3& color, float alpha, int segments) {
	if (!camera || !rendererPipeline_) {
		return;
	}

	std::vector<Line> coneLines;

	// 角度をラジアンに変換
	float angleRad = angle * std::numbers::pi_v<float> / 180.0f;
	float baseRadius = height * std::tan(angleRad);

	// コーンの底面の中心
	Vector3 normalizedDir = Vector::Normalize(direction);
	Vector3 baseCenter = {
		apex.x + normalizedDir.x * height,
		apex.y + normalizedDir.y * height,
		apex.z + normalizedDir.z * height
	};

	// 底面の円を描画するための軸を計算
	Vector3 up = { 0.0f, 1.0f, 0.0f };
	Vector3 right;

	if (std::abs(normalizedDir.y) > 0.999f) {
		right = { 1.0f, 0.0f, 0.0f };
	} else {
		right = Vector::Normalize(Vector::Cross(up, normalizedDir));
	}
	up = Vector::Normalize(Vector::Cross(normalizedDir, right));

	// 底面の円を描画
	for (int i = 0; i < segments; ++i) {
		float angle1 = (static_cast<float>(i) / segments) * 2.0f * std::numbers::pi_v<float>;
		float angle2 = (static_cast<float>(i + 1) / segments) * 2.0f * std::numbers::pi_v<float>;

		Vector3 p1 = {
			baseCenter.x + baseRadius * (std::cos(angle1) * right.x + std::sin(angle1) * up.x),
			baseCenter.y + baseRadius * (std::cos(angle1) * right.y + std::sin(angle1) * up.y),
			baseCenter.z + baseRadius * (std::cos(angle1) * right.z + std::sin(angle1) * up.z)
		};

		Vector3 p2 = {
			baseCenter.x + baseRadius * (std::cos(angle2) * right.x + std::sin(angle2) * up.x),
			baseCenter.y + baseRadius * (std::cos(angle2) * right.y + std::sin(angle2) * up.y),
			baseCenter.z + baseRadius * (std::cos(angle2) * right.z + std::sin(angle2) * up.z)
		};

		// 底面の円
		coneLines.push_back({ p1, p2, color, alpha });

		// 頂点から底面への線（4本だけ描画）
		if (i % (segments / 4) == 0) {
			coneLines.push_back({ apex, p1, color, alpha });
		}
	}

	AddLines(coneLines);
}

void LineRenderer::DrawCylinder(const ICamera* camera, const Vector3& center, float radius,
	float height, const Vector3& direction, const Vector3& color, float alpha, int segments) {
	if (!camera || !rendererPipeline_) {
		return;
	}

	std::vector<Line> cylinderLines;

	Vector3 normalizedDir = Vector::Normalize(direction);
	float halfHeight = height * 0.5f;

	// 上面と下面の中心
	Vector3 topCenter = {
		center.x + normalizedDir.x * halfHeight,
		center.y + normalizedDir.y * halfHeight,
		center.z + normalizedDir.z * halfHeight
	};

	Vector3 bottomCenter = {
		center.x - normalizedDir.x * halfHeight,
		center.y - normalizedDir.y * halfHeight,
		center.z - normalizedDir.z * halfHeight
	};

	// 円を描画するための軸を計算
	Vector3 up = { 0.0f, 1.0f, 0.0f };
	Vector3 right;

	if (std::abs(normalizedDir.y) > 0.999f) {
		right = { 1.0f, 0.0f, 0.0f };
	} else {
		right = Vector::Normalize(Vector::Cross(up, normalizedDir));
	}
	up = Vector::Normalize(Vector::Cross(normalizedDir, right));

	// 上面と下面の円を描画
	for (int i = 0; i < segments; ++i) {
		float angle1 = (static_cast<float>(i) / segments) * 2.0f * std::numbers::pi_v<float>;
		float angle2 = (static_cast<float>(i + 1) / segments) * 2.0f * std::numbers::pi_v<float>;

		// 上面
		Vector3 topP1 = {
			topCenter.x + radius * (std::cos(angle1) * right.x + std::sin(angle1) * up.x),
			topCenter.y + radius * (std::cos(angle1) * right.y + std::sin(angle1) * up.y),
			topCenter.z + radius * (std::cos(angle1) * right.z + std::sin(angle1) * up.z)
		};

		Vector3 topP2 = {
			topCenter.x + radius * (std::cos(angle2) * right.x + std::sin(angle2) * up.x),
			topCenter.y + radius * (std::cos(angle2) * right.y + std::sin(angle2) * up.y),
			topCenter.z + radius * (std::cos(angle2) * right.z + std::sin(angle2) * up.z)
		};

		// 下面
		Vector3 bottomP1 = {
			bottomCenter.x + radius * (std::cos(angle1) * right.x + std::sin(angle1) * up.x),
			bottomCenter.y + radius * (std::cos(angle1) * right.y + std::sin(angle1) * up.y),
			bottomCenter.z + radius * (std::cos(angle1) * right.z + std::sin(angle1) * up.z)
		};

		Vector3 bottomP2 = {
			bottomCenter.x + radius * (std::cos(angle2) * right.x + std::sin(angle2) * up.x),
			bottomCenter.y + radius * (std::cos(angle2) * right.y + std::sin(angle2) * up.y),
			bottomCenter.z + radius * (std::cos(angle2) * right.z + std::sin(angle2) * up.z)
		};

		// 上面の円
		cylinderLines.push_back({ topP1, topP2, color, alpha });

		// 下面の円
		cylinderLines.push_back({ bottomP1, bottomP2, color, alpha });

		// 側面の線（4本だけ描画）
		if (i % (segments / 4) == 0) {
			cylinderLines.push_back({ topP1, bottomP1, color, alpha });
		}
	}

	AddLines(cylinderLines);
}