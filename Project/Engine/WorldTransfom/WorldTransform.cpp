#include "WorldTransform.h"
#include "Engine/Graphics/Resource/ResourceFactory.h"
#include <cassert>

#ifdef _DEBUG
#include <imgui.h>
#endif

using namespace MathCore;

void WorldTransform::Initialize(ID3D12Device* device)
{
    // 定数バッファを作成
    constantBuffer_ = ResourceFactory::CreateBufferResource(device, sizeof(ConstantBufferDataWorldTransform));

    // マッピング
    constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mapped_));
    assert(mapped_ != nullptr);

    // 初期行列を転送
    TransferMatrix();
}

void WorldTransform::TransferMatrix()
{
    // ローカル行列を計算
    Matrix4x4 localMatrix = Matrix::MakeAffine(scale, rotate, translate);

    // 親がいる場合は親の行列と合成
    if (parent_) {
        matWorld_ = Matrix::Multiply(localMatrix, parent_->GetWorldMatrix());
    } else {
        matWorld_ = localMatrix;
    }

    // GPUに転送
    if (mapped_) {
        mapped_->matWorld = matWorld_;
    }
}

D3D12_GPU_VIRTUAL_ADDRESS WorldTransform::GetGPUVirtualAddress() const
{
    return constantBuffer_ ? constantBuffer_->GetGPUVirtualAddress() : 0;
}

Vector3 WorldTransform::GetWorldPosition() const
{
    return { matWorld_.m[3][0], matWorld_.m[3][1], matWorld_.m[3][2] };
}

void WorldTransform::SetWorldMatrix(const Matrix4x4& matrix)
{
    matWorld_ = matrix;
    
    // GPUに転送
    if (mapped_) {
        mapped_->matWorld = matWorld_;
 }
}

#ifdef _DEBUG
bool WorldTransform::DrawImGui(const std::string& label)
{
    bool changed = false;

    if (ImGui::CollapsingHeader((label + " Transform").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        // Scale
        if (ImGui::DragFloat3((label + " Scale").c_str(), &scale.x, 0.01f, 0.001f, 10.0f)) {
            changed = true;
        }

        // Rotate（ラジアン表示）
        if (ImGui::DragFloat3((label + " Rotate").c_str(), &rotate.x, 0.01f, -6.28f, 6.28f)) {
            changed = true;
        }

        // Translate
        if (ImGui::DragFloat3((label + " Translate").c_str(), &translate.x, 0.05f, -100.0f, 100.0f)) {
            changed = true;
        }

        // 親の情報
        if (parent_) {
            ImGui::Text("Parent: Yes");
            Vector3 worldPos = GetWorldPosition();
            ImGui::Text("World Position: (%.2f, %.2f, %.2f)", worldPos.x, worldPos.y, worldPos.z);
        } else {
            ImGui::Text("Parent: None");
        }

        // リセットボタン
        if (ImGui::Button((label + " Reset").c_str())) {
            scale = { 1.0f, 1.0f, 1.0f };
            rotate = { 0.0f, 0.0f, 0.0f };
            translate = { 0.0f, 0.0f, 0.0f };
            changed = true;
        }
    }

    return changed;
}
#else
bool WorldTransform::DrawImGui(const std::string& label)
{
    (void)label;
    return false;
}
#endif
