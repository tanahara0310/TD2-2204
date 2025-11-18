#include "SkeletonDebugRenderer.h"
#include "Engine/Math/MathCore.h"
#include <imgui.h>
#include <numbers>
#include <cmath>

void SkeletonDebugRenderer::GenerateSkeletonLines(
	const Skeleton& skeleton,
	const Matrix4x4& worldMatrix,
	float jointRadius,
	std::vector<LineRenderer::Line>& outLines
) {
	// すべてのJointを処理
	for (size_t i = 0; i < skeleton.joints.size(); ++i) {
		const Joint& joint = skeleton.joints[i];

		// JointのワールドSpace座標を計算
		Matrix4x4 jointWorldMatrix = MathCore::Matrix::Multiply(joint.skeletonSpaceMatrix, worldMatrix);

		// 平行移動成分を取得（Joint の位置）
		Vector3 jointPosition = {
			jointWorldMatrix.m[3][0],
			jointWorldMatrix.m[3][1],
			jointWorldMatrix.m[3][2]
		};

		// 球を構成する線を生成
		Vector3 jointColor = { 1.0f, 1.0f, 1.0f }; // 白色
		int segments = 8; // セグメント数を減らして最適化

		// 緯度線
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
					jointPosition.x + jointRadius * sinTheta * cosPhi1,
					jointPosition.y + jointRadius * cosTheta,
					jointPosition.z + jointRadius * sinTheta * sinPhi1
				};

				Vector3 p2 = {
					jointPosition.x + jointRadius * sinTheta * cosPhi2,
					jointPosition.y + jointRadius * cosTheta,
					jointPosition.z + jointRadius * sinTheta * sinPhi2
				};

				outLines.push_back({ p1, p2, jointColor, 1.0f });
			}
		}

		// 経度線
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
					jointPosition.x + jointRadius * sinTheta1 * cosPhi,
					jointPosition.y + jointRadius * cosTheta1,
					jointPosition.z + jointRadius * sinTheta1 * sinPhi
				};

				Vector3 p2 = {
					jointPosition.x + jointRadius * sinTheta2 * cosPhi,
					jointPosition.y + jointRadius * cosTheta2,
					jointPosition.z + jointRadius * sinTheta2 * sinPhi
				};

				outLines.push_back({ p1, p2, jointColor, 1.0f });
			}
		}

		// 親がいれば親との間に線を引く
		if (joint.parent) {
			const Joint& parentJoint = skeleton.joints[*joint.parent];
			Matrix4x4 parentWorldMatrix = MathCore::Matrix::Multiply(parentJoint.skeletonSpaceMatrix, worldMatrix);

			Vector3 parentPosition = {
				parentWorldMatrix.m[3][0],
				parentWorldMatrix.m[3][1],
				parentWorldMatrix.m[3][2]
			};

			// 親子を線で繋ぐ
			LineRenderer::Line line;
			line.start = parentPosition;
			line.end = jointPosition;
			line.color = { 0.0f, 1.0f, 0.0f }; // 緑色
			line.alpha = 1.0f;
			outLines.push_back(line);
		}
	}
}

bool SkeletonDebugRenderer::DrawSkeletonImGui(
	const Skeleton* skeleton,
	bool& drawSkeleton,
	float& jointRadius,
	const char* objectName
) {
	if (!skeleton) {
		return false;
	}

	bool changed = false;

	// Skeleton制御（TreeNode使用）
	if (ImGui::TreeNode("スケルトン制御")) {
		// 一意なIDを生成
		ImGui::PushID((std::string(objectName) + "_Skeleton").c_str());
		
		if (ImGui::Checkbox("スケルトンを描画", &drawSkeleton)) {
			changed = true;
		}
		
		if (ImGui::SliderFloat("ジョイント半径", &jointRadius, 0.01f, 0.5f)) {
			changed = true;
		}
		
		// Skeleton情報表示
		ImGui::Text("ジョイント数: %zu", skeleton->joints.size());
		ImGui::Text("ルートジョイントインデックス: %d", skeleton->root);
		
		// 各Jointの情報表示（TreeNodeで折りたたみ）
		if (ImGui::TreeNode("ジョイント詳細")) {
			for (const Joint& joint : skeleton->joints) {
				// Joint名でTreeNode（一意なIDを自動生成）
				ImGui::PushID(joint.index);
				if (ImGui::TreeNode("##joint", "%s", joint.name.c_str())) {
					ImGui::Text("インデックス: %d", joint.index);
					if (joint.parent) {
						ImGui::Text("親: %d (%s)", *joint.parent, skeleton->joints[*joint.parent].name.c_str());
					} else {
						ImGui::Text("親: なし (ルート)");
					}
					ImGui::Text("子: %zu", joint.children.size());
					ImGui::TreePop();
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		
		ImGui::PopID();
		ImGui::TreePop();
	}
	
	return changed;
}



