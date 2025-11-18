#include "CameraManager.h"

#ifdef _DEBUG
#include <imgui.h>
#include "Camera/Debug/DebugCamera.h"
#include "Camera/Release/Camera.h"
#endif

void CameraManager::RegisterCamera(const std::string& name, std::unique_ptr<ICamera> camera)
{
	if (!camera) {
		return;
	}

	// 既存の同名カメラがあれば削除
	if (cameras_.find(name) != cameras_.end()) {
		// アクティブカメラだった場合はクリア
		if (activeCameraName_ == name) {
			activeCameraName_.clear();
			activeCamera_ = nullptr;
		}
	}

	cameras_[name] = std::move(camera);

	// 最初に登録されたカメラを自動的にアクティブに設定
	if (cameras_.size() == 1) {
		SetActiveCamera(name);
	}
}

void CameraManager::UnregisterCamera(const std::string& name)
{
	auto it = cameras_.find(name);
	if (it == cameras_.end()) {
		return;
	}

	// アクティブカメラだった場合はクリア
	if (activeCameraName_ == name) {
		activeCameraName_.clear();
		activeCamera_ = nullptr;
	}

	cameras_.erase(it);
}

bool CameraManager::SetActiveCamera(const std::string& name)
{
	auto it = cameras_.find(name);
	if (it == cameras_.end()) {
		return false;
	}

	activeCameraName_ = name;
	activeCamera_ = it->second.get();
	return true;
}

ICamera* CameraManager::GetActiveCamera() const
{
	return activeCamera_;
}

ICamera* CameraManager::GetCamera(const std::string& name) const
{
	auto it = cameras_.find(name);
	if (it == cameras_.end()) {
		return nullptr;
	}
	return it->second.get();
}

const Matrix4x4& CameraManager::GetViewMatrix() const
{
	static Matrix4x4 identity = MathCore::Matrix::Identity();
	if (!activeCamera_) {
		return identity;
	}
	return activeCamera_->GetViewMatrix();
}

const Matrix4x4& CameraManager::GetProjectionMatrix() const
{
	static Matrix4x4 identity = MathCore::Matrix::Identity();
	if (!activeCamera_) {
		return identity;
	}
	return activeCamera_->GetProjectionMatrix();
}

Vector3 CameraManager::GetCameraPosition() const
{
	if (!activeCamera_) {
		return { 0.0f, 0.0f, 0.0f };
	}
	return activeCamera_->GetPosition();
}

void CameraManager::Update()
{
	if (activeCamera_ && activeCamera_->GetActive()) {
		activeCamera_->Update();
	}
}

#ifdef _DEBUG
void CameraManager::DrawImGui()
{
	// 専用のカメラウィンドウを作成（ビューポートの下に配置）
	if (ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_None)) {
		ImGui::Text("登録カメラ数: %zu", cameras_.size());
		ImGui::Separator();

		// カメラ切り替え
		if (cameras_.size() > 1) {
			ImGui::Text("カメラ選択:");
			size_t count = 0;
			for (const auto& [name, camera] : cameras_) {
				bool isActive = (name == activeCameraName_);
				if (ImGui::RadioButton(name.c_str(), isActive)) {
					SetActiveCamera(name);
				}
				// 最後以外は横に並べる（単純に2つまで）
				if (++count < cameras_.size() && count < 3) {
					ImGui::SameLine();
				}
			}
			ImGui::Separator();
		}

		// アクティブカメラ情報
		if (activeCamera_) {
			ImGui::Text("アクティブカメラ: %s", activeCameraName_.c_str());
			
			// カメラの有効/無効切り替え
			bool isActive = activeCamera_->GetActive();
			if (ImGui::Checkbox("カメラ有効", &isActive)) {
				activeCamera_->SetActive(isActive);
			}

			// 位置情報
			Vector3 pos = activeCamera_->GetPosition();
			ImGui::Text("位置: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);

			ImGui::Separator();

			// ===== DebugCamera固有のコントロール =====
			DebugCamera* debugCam = dynamic_cast<DebugCamera*>(activeCamera_);
			if (debugCam) {
				ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "デバッグカメラ制御");
				
				// 現在の状態表示
				Vector3 target = debugCam->GetTarget();
				float distance = debugCam->GetDistance();
				float pitch = debugCam->GetPitch();
				float yaw = debugCam->GetYaw();

				ImGui::Text("注視点: (%.2f, %.2f, %.2f)", target.x, target.y, target.z);
				ImGui::Text("距離: %.2f", distance);
				ImGui::Text("ピッチ: %.2f° (%.3f rad)", pitch * 180.0f / 3.14159f, pitch);
				ImGui::Text("ヨー: %.2f° (%.3f rad)", yaw * 180.0f / 3.14159f, yaw);

				if (debugCam->IsControlling()) {
					ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "操作中");
				}

				ImGui::Separator();

				// カメラ設定
				if (ImGui::TreeNode("カメラ設定")) {
					auto settings = debugCam->GetSettings();
					bool settingsChanged = false;

					settingsChanged |= ImGui::SliderFloat("回転感度", &settings.rotationSensitivity, 0.001f, 0.01f, "%.4f");
					settingsChanged |= ImGui::SliderFloat("パン感度", &settings.panSensitivity, 0.0001f, 0.002f, "%.4f");
					settingsChanged |= ImGui::SliderFloat("ズーム感度", &settings.zoomSensitivity, 0.1f, 5.0f, "%.1f");
					
					ImGui::Separator();
					
					settingsChanged |= ImGui::DragFloat("最小距離", &settings.minDistance, 0.01f, 0.01f, 1.0f, "%.2f");
					settingsChanged |= ImGui::DragFloat("最大距離", &settings.maxDistance, 10.0f, 100.0f, 50000.0f, "%.1f");
					
					ImGui::Separator();
					
					settingsChanged |= ImGui::Checkbox("Y軸反転", &settings.invertY);
					settingsChanged |= ImGui::Checkbox("スムーズ移動", &settings.smoothMovement);
					
					if (settings.smoothMovement) {
						settingsChanged |= ImGui::SliderFloat("スムージング係数", &settings.smoothingFactor, 0.01f, 0.5f, "%.3f");
					}

					if (settingsChanged) {
						debugCam->SetSettings(settings);
					}

					ImGui::TreePop();
				}

				// パラメータ直接制御
				if (ImGui::TreeNode("パラメータ直接制御")) {
					float tempDistance = distance;
					if (ImGui::SliderFloat("距離", &tempDistance, debugCam->GetSettings().minDistance, debugCam->GetSettings().maxDistance, "%.2f")) {
						debugCam->SetDistance(tempDistance);
					}

					float pitchDegrees = pitch * 180.0f / 3.14159f;
					if (ImGui::SliderFloat("ピッチ (度)", &pitchDegrees, -89.0f, 89.0f, "%.1f°")) {
						debugCam->SetPitch(pitchDegrees * 3.14159f / 180.0f);
					}

					float yawDegrees = yaw * 180.0f / 3.14159f;
					if (ImGui::SliderFloat("ヨー (度)", &yawDegrees, -180.0f, 180.0f, "%.1f°")) {
						debugCam->SetYaw(yawDegrees * 3.14159f / 180.0f);
					}

					Vector3 tempTarget = target;
					if (ImGui::DragFloat3("注視点", &tempTarget.x, 0.1f, -1000.0f, 1000.0f, "%.2f")) {
						debugCam->SetTarget(tempTarget);
					}

					ImGui::TreePop();
				}

				// プリセット
				if (ImGui::TreeNode("プリセット")) {
					const float buttonWidth = 80.0f;

					if (ImGui::Button("リセット", ImVec2(buttonWidth, 0))) {
						debugCam->Reset();
					}
					ImGui::SameLine();

					if (ImGui::Button("正面", ImVec2(buttonWidth, 0))) {
						debugCam->ApplyPreset(DebugCamera::CameraPreset::Front);
					}
					ImGui::SameLine();

					if (ImGui::Button("背面", ImVec2(buttonWidth, 0))) {
						debugCam->ApplyPreset(DebugCamera::CameraPreset::Back);
					}

					if (ImGui::Button("左側", ImVec2(buttonWidth, 0))) {
						debugCam->ApplyPreset(DebugCamera::CameraPreset::Left);
					}
					ImGui::SameLine();

					if (ImGui::Button("右側", ImVec2(buttonWidth, 0))) {
						debugCam->ApplyPreset(DebugCamera::CameraPreset::Right);
					}
					ImGui::SameLine();

					if (ImGui::Button("上から", ImVec2(buttonWidth, 0))) {
						debugCam->ApplyPreset(DebugCamera::CameraPreset::Top);
					}

					if (ImGui::Button("下から", ImVec2(buttonWidth, 0))) {
						debugCam->ApplyPreset(DebugCamera::CameraPreset::Bottom);
					}
					ImGui::SameLine();

					if (ImGui::Button("斜め", ImVec2(buttonWidth, 0))) {
						debugCam->ApplyPreset(DebugCamera::CameraPreset::Diagonal);
					}
					ImGui::SameLine();

					if (ImGui::Button("接近", ImVec2(buttonWidth, 0))) {
						debugCam->ApplyPreset(DebugCamera::CameraPreset::CloseUp);
					}

					if (ImGui::Button("広角", ImVec2(buttonWidth, 0))) {
						debugCam->ApplyPreset(DebugCamera::CameraPreset::Wide);
					}

					ImGui::TreePop();
				}

				// 操作方法
				if (ImGui::TreeNode("操作方法")) {
					ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.0f), "シーンウィンドウ上でのみ有効（Blender式）");
					ImGui::Separator();

					ImGui::BulletText("中ドラッグ: カメラ回転");
					ImGui::BulletText("Shift + 中ドラッグ: カメラ移動");
					ImGui::BulletText("ホイール: ズーム");

					ImGui::TreePop();
				}
			}
			// ===== Release Camera固有のコントロール =====
			else {
				Camera* releaseCam = dynamic_cast<Camera*>(activeCamera_);
				if (releaseCam) {
					ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "リリースカメラ制御");

					Vector3 scale = releaseCam->GetScale();
					Vector3 rotate = releaseCam->GetRotate();
					Vector3 translate = releaseCam->GetTranslate();

					if (ImGui::DragFloat3("スケール", &scale.x, 0.01f, 0.01f, 10.0f, "%.2f")) {
						releaseCam->SetScale(scale);
					}

					if (ImGui::DragFloat3("回転", &rotate.x, 0.01f, -3.14159f, 3.14159f, "%.2f")) {
						releaseCam->SetRotate(rotate);
					}

					if (ImGui::DragFloat3("位置", &translate.x, 0.1f, -100.0f, 100.0f, "%.2f")) {
						releaseCam->SetTranslate(translate);
					}
				}
			}
		} else {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "アクティブなカメラがありません");
		}
	}
	ImGui::End();
}
#endif
