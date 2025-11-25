#include "SpriteObject.h"
#include "Engine/EngineSystem/EngineSystem.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Graphics/Render/RenderManager.h"
#include "Engine/Graphics/Render/Sprite/SpriteRenderer.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Resource/ResourceFactory.h"
#include "Engine/Graphics/Structs/VertexData.h"
#include "Engine/Graphics/Structs/SpriteMaterial.h"
#include <cmath>
#include <imgui.h>

using namespace MathCore;

void SpriteObject::Initialize(const std::string& textureFilePath) {
    auto engine = GetEngineSystem();
    
    // SpriteRendererを取得
    auto* renderManager = engine->GetComponent<RenderManager>();
    spriteRenderer_ = dynamic_cast<SpriteRenderer*>(renderManager->GetRenderer(RenderPassType::Sprite));
    
    // テクスチャの読み込み
    textureHandle_ = TextureManager::GetInstance().Load(textureFilePath);
    
    // テクスチャサイズを自動設定
    SetSizeFromTexture(textureFilePath);
    
    // 頂点バッファを作成
    CreateVertexBuffer();
    
    // デフォルト値を設定
    Reset();
    
    // アクティブ状態
    isActive_ = true;
}

void SpriteObject::SetSizeFromTexture(const std::string& textureFilePath) {
    auto& textureManager = TextureManager::GetInstance();
    DirectX::TexMetadata metadata = textureManager.GetMetadata(textureFilePath);
    
    textureSize_.x = static_cast<float>(metadata.width);
    textureSize_.y = static_cast<float>(metadata.height);
}

void SpriteObject::CreateVertexBuffer() {
    if (!spriteRenderer_) return;
    
    DirectXCommon* dxCommon = spriteRenderer_->GetDirectXCommon();
    ResourceFactory* resourceFactory = spriteRenderer_->GetResourceFactory();
    
    if (!dxCommon || !resourceFactory) return;
    
    // 頂点バッファの生成（4頂点のクワッド）
    vertexResource_ = resourceFactory->CreateBufferResource(
        dxCommon->GetDevice(),
        sizeof(VertexData) * 4);
    
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
    
    // インデックスバッファの生成
    indexResource_ = resourceFactory->CreateBufferResource(
        dxCommon->GetDevice(),
        sizeof(uint32_t) * 6);
    
    uint32_t* indexData = nullptr;
    indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
    
    indexData[0] = 0; indexData[1] = 1; indexData[2] = 2;
    indexData[3] = 1; indexData[4] = 3; indexData[5] = 2;
    
    indexResource_->Unmap(0, nullptr);
    
    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
    
    UpdateVertexData();
}

void SpriteObject::UpdateVertexData() {
    if (!vertexResource_) return;
    
    VertexData* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    
    // アンカーポイントを考慮したローカル座標
     // anchorPoint_ : (0,0)=左上, (1,1)=右下
    float left = -anchorPoint_.x;
    float right = 1.0f - anchorPoint_.x;
    float top = anchorPoint_.y;        // 上は +Y（カメラ2Dに合わせる）
    float bottom = anchorPoint_.y - 1.0f; // 下は -Y

    // 左下
    vertexData[0].position = { left,  bottom, 0.0f, 1.0f };
    vertexData[0].texcoord = { uvMin_.x, uvMax_.y };  // (0,1) テクスチャ下側
    vertexData[0].normal = { 0.0f, 0.0f, -1.0f };

    // 左上
    vertexData[1].position = { left,  top,    0.0f, 1.0f };
    vertexData[1].texcoord = { uvMin_.x, uvMin_.y };  // (0,0) テクスチャ上側
    vertexData[1].normal = { 0.0f, 0.0f, -1.0f };

    // 右下
    vertexData[2].position = { right, bottom, 0.0f, 1.0f };
    vertexData[2].texcoord = { uvMax_.x, uvMax_.y };  // (1,1)
    vertexData[2].normal = { 0.0f, 0.0f, -1.0f };

    // 右上
    vertexData[3].position = { right, top,    0.0f, 1.0f };
    vertexData[3].texcoord = { uvMax_.x, uvMin_.y };  // (1,0)
    vertexData[3].normal = { 0.0f, 0.0f, -1.0f };

    vertexResource_->Unmap(0, nullptr);
}

void SpriteObject::Update() {
    if (!isActive_) return;
}

void SpriteObject::Draw2D(const ICamera* camera) {
    if (!spriteRenderer_) return;
    
    auto* commandList = spriteRenderer_->GetDirectXCommon()->GetCommandList();
    
    size_t bufferIndex = spriteRenderer_->GetAvailableConstantBuffer();
    
    // マテリアルデータ設定（必ず最新の値で上書き）
    auto& materialData = spriteRenderer_->GetMaterialDataPool()[bufferIndex];
    materialData->color = color_;
    materialData->uvTransform = uvTransform_;
    
    // 実際の描画サイズを計算（テクスチャサイズ × スケール）
    Vector3 actualScale = {
        textureSize_.x * transform_.scale.x,
        textureSize_.y * transform_.scale.y,
        transform_.scale.z
    };
    
    // 変換行列設定（必ず最新の値で上書き）
    auto& transformData = spriteRenderer_->GetTransformDataPool()[bufferIndex];
    Matrix4x4 worldMatrix = Matrix::MakeAffine(actualScale, transform_.rotate, transform_.translate);
    
    // カメラを使用してWVP行列を計算
    transformData->WVP = spriteRenderer_->CalculateWVPMatrix(transform_.translate, actualScale, transform_.rotate, camera);
    transformData->world = worldMatrix;
    
    // 定数バッファ設定
    commandList->SetGraphicsRootConstantBufferView(0, spriteRenderer_->GetMaterialResource(bufferIndex)->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(1, spriteRenderer_->GetTransformResource(bufferIndex)->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(2, textureHandle_.gpuHandle);
    
    // 頂点バッファ・インデックスバッファを設定
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->IASetIndexBuffer(&indexBufferView_);
    
    // 描画
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void SpriteObject::Reset() {
    transform_.scale = { 1.0f, 1.0f, 1.0f };
    transform_.rotate = { 0.0f, 0.0f, 0.0f };
    transform_.translate = { 0.0f, 0.0f, 0.0f };
    color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    uvTransform_ = Matrix::Identity();
    anchorPoint_ = { 0.5f, 0.5f };  // デフォルトを中央に変更
    uvMin_ = { 0.0f, 0.0f };
    uvMax_ = { 1.0f, 1.0f };
    
    if (vertexResource_) {
        UpdateVertexData();
    }
}

void SpriteObject::SetTexture(const std::string& textureFilePath) {
    textureHandle_ = TextureManager::GetInstance().Load(textureFilePath);
    SetSizeFromTexture(textureFilePath);
}

void SpriteObject::SetAnchor(const Vector2& anchor) {
    anchorPoint_ = anchor;
    if (vertexResource_) {
        UpdateVertexData();
    }
}

void SpriteObject::SetTextureRect(float texLeft, float texTop, float texWidth, float texHeight,
    const std::string& textureFilePath) {
    auto& textureManager = TextureManager::GetInstance();
    DirectX::TexMetadata metadata = textureManager.GetMetadata(textureFilePath);
    
    float textureWidth = static_cast<float>(metadata.width);
    float textureHeight = static_cast<float>(metadata.height);
    
    uvMin_.x = texLeft / textureWidth;
    uvMin_.y = texTop / textureHeight;
    uvMax_.x = (texLeft + texWidth) / textureWidth;
    uvMax_.y = (texTop + texHeight) / textureHeight;
    
    UpdateVertexData();
}

void SpriteObject::SetUVRect(float uvLeft, float uvTop, float uvRight, float uvBottom) {
    uvMin_.x = uvLeft;
    uvMin_.y = uvTop;
    uvMax_.x = uvRight;
    uvMax_.y = uvBottom;
    
    UpdateVertexData();
}

void SpriteObject::SetUVOffset(float offsetX, float offsetY) {
    uvTransform_ = Matrix::Translation({ offsetX, offsetY, 0.0f });
}

void SpriteObject::SetUVScale(float scaleX, float scaleY) {
    uvTransform_ = Matrix::Scale({ scaleX, scaleY, 1.0f });
}

void SpriteObject::SetUVRotation(float rotation) {
    uvTransform_ = Matrix::RotationZ(rotation);
}

void SpriteObject::ResetUVTransform() {
    uvTransform_ = Matrix::Identity();
}

void SpriteObject::UpdateUVTransformMatrix(const EulerTransform& uvTransform) {
    Matrix4x4 scaleMatrix = Matrix::Scale(uvTransform.scale);
    Matrix4x4 rotateMatrix = Matrix::RotationZ(uvTransform.rotate.z);
    Matrix4x4 translateMatrix = Matrix::Translation(uvTransform.translate);
    uvTransform_ = Matrix::Multiply(Matrix::Multiply(scaleMatrix, rotateMatrix), translateMatrix);
}

void SpriteObject::ChangeAnchorKeepingPosition(const Vector2& newAnchor) {
    // アンカーポイントを変更（座標は変更しない）
    anchorPoint_ = newAnchor;
    if (vertexResource_) {
        UpdateVertexData();
    }
}

bool SpriteObject::DrawImGui() {
    bool changed = false;
    
    std::string header = std::string(GetObjectName()) + " (" + std::to_string(reinterpret_cast<uintptr_t>(this)) + ")";
    
    if (ImGui::CollapsingHeader(header.c_str())) {
        ImGui::PushID(this);
        
        bool active = isActive_;
        if (ImGui::Checkbox("Active", &active)) {
            isActive_ = active;
            changed = true;
        }
        
        ImGui::Separator();
        
        ImGui::Text("座標 (Position):");
        if (ImGui::DragFloat("X##pos", &transform_.translate.x, 0.1f, -FLT_MAX, FLT_MAX, "%.2f")) {
            changed = true;
        }
        if (ImGui::DragFloat("Y##pos", &transform_.translate.y, 0.1f, -FLT_MAX, FLT_MAX, "%.2f")) {
            changed = true;
        }
        if (ImGui::DragFloat("Z##pos", &transform_.translate.z, 0.1f, -FLT_MAX, FLT_MAX, "%.2f")) {
            changed = true;
        }
        
        ImGui::Separator();
        
        ImGui::Text("スケール (Scale):");
        ImGui::Text("倍率（テクスチャサイズに対する）");
        if (ImGui::DragFloat3("##scale", &transform_.scale.x, 0.01f, 0.0f, 10.0f)) {
            changed = true;
        }
        
        ImGui::Text("テクスチャサイズ: %.0fx%.0f px", textureSize_.x, textureSize_.y);
        Vector2 actualSize = GetActualSize();
        ImGui::Text("実際の描画サイズ: %.0fx%.0f px", actualSize.x, actualSize.y);
        
        ImGui::Separator();
        
        ImGui::Text("回転 (Rotation):");
        if (ImGui::DragFloat3("##rotate", &transform_.rotate.x, 0.01f, -6.28f, 6.28f)) {
            changed = true;
        }
        
        ImGui::Separator();
        
        ImGui::Text("色 (Color):");
        if (ImGui::ColorEdit4("##color", &color_.x)) {
            changed = true;
        }
        
        ImGui::Separator();
        ImGui::Text("アンカーポイント (Anchor Point):");
        
        // アンカーポイントのドラッグ
        Vector2 anchorTemp = anchorPoint_;
        if (ImGui::DragFloat2("##anchor", &anchorTemp.x, 0.01f, 0.0f, 1.0f, "%.3f")) {
            ChangeAnchorKeepingPosition(anchorTemp);
            changed = true;
        }
        
        // プリセットボタン
        ImGui::Text("プリセット:");
        if (ImGui::Button("左上##topleft")) {
            ChangeAnchorKeepingPosition({ 0.0f, 0.0f });
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("中央##center")) {
            ChangeAnchorKeepingPosition({ 0.5f, 0.5f });
            changed = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("右下##bottomright")) {
            ChangeAnchorKeepingPosition({ 1.0f, 1.0f });
            changed = true;
        }
        
        ImGui::Separator();
        if (ImGui::Button("リセット##reset")) {
            Reset();
            changed = true;
        }
        
        ImGui::PopID();
    }
    
    return changed;
}
