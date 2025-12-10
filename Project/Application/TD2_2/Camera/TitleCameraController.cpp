#include "TitleCameraController.h"
#include "Engine/Camera/Release/Camera.h"
#include "MathCore.h"
#include <imgui.h>
#include <cmath>

void TitleCameraController::ApplyToCamera(Camera* camera) {
    if (!camera) {
        return;
    }

    // LookAtでビュー行列を計算
    Matrix4x4 viewMatrix = MathCore::Matrix::LookAt(cameraPos_, target_, up_);

    // カメラの位置を設定
    camera->SetTranslate(cameraPos_);

    // ビュー行列を直接設定
    camera->SetViewMatrix(viewMatrix);
}

void TitleCameraController::Update(float deltaTime) {
    // ターゲット移動アニメーション
    if (isTargetAnimating_) {
        targetAnimTimer_ += deltaTime * kTargetAnimSpeed;
        
        // sin波で滑らかに上下移動
        float offset = std::sin(targetAnimTimer_) * kTargetAnimAmplitude;
        
        // Y座標のみを変更
        target_.y = baseTarget_.y + offset;
    }
}

bool TitleCameraController::DrawImGui() {
    bool changed = false;

    if (ImGui::Begin("Title Camera Controller")) {
        ImGui::Text("Camera Parameters");
        ImGui::Separator();

        // カメラ位置の調整
        if (ImGui::DragFloat3("Camera Position", &cameraPos_.x, 0.1f)) {
            changed = true;
        }

        // 注視点の調整
        if (ImGui::DragFloat3("Target Position", &target_.x, 0.1f)) {
            changed = true;
        }

        // 上方向ベクトルの調整（通常は変更不要）
        if (ImGui::TreeNode("Advanced")) {
            if (ImGui::DragFloat3("Up Vector", &up_.x, 0.01f)) {
                changed = true;
            }
            ImGui::TreePop();
        }

        ImGui::Separator();

        // カメラからターゲットまでの距離を表示
        Vector3 direction = {
            target_.x - cameraPos_.x,
            target_.y - cameraPos_.y,
            target_.z - cameraPos_.z
        };
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
        ImGui::Text("Distance to Target: %.2f", distance);

        // 角度情報を表示
        float yaw = std::atan2(direction.x, direction.z) * (180.0f / 3.14159265f);
        float horizontalDistance = std::sqrt(direction.x * direction.x + direction.z * direction.z);
        float pitch = std::atan2(direction.y, horizontalDistance) * (180.0f / 3.14159265f);
        ImGui::Text("Yaw: %.2f degrees", yaw);
        ImGui::Text("Pitch: %.2f degrees", pitch);

        ImGui::Separator();

        // プリセットボタン
        if (ImGui::Button("Reset to Default")) {
            cameraPos_ = { -12.0f, -6.0f, -68.0f };
            target_ = { 44.37f, 9.94f, -12.81f };
            up_ = { 0.0f, 1.0f, 0.0f };
            changed = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Front View")) {
            cameraPos_ = { 0.0f, 0.0f, -50.0f };
            target_ = { 0.0f, 0.0f, 0.0f };
            changed = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Low Angle")) {
            cameraPos_ = { -15.0f, -10.0f, -70.0f };
            target_ = { 40.0f, 10.0f, -15.0f };
            changed = true;
        }
    }
    ImGui::End();

    return changed;
}
