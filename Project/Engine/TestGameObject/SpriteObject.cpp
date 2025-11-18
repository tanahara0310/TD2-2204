#include "SpriteObject.h"
#include "Engine/EngineSystem/EngineSystem.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Graphics/Render/RenderManager.h"
#include "Engine/Graphics/Render/Sprite/SpriteRenderer.h"
#include <imgui.h>

void SpriteObject::Initialize(EngineSystem* engine) {
    // デフォルトテクスチャで初期化
    Initialize(engine, "Resources/SampleResources/uvChecker.png");
}

void SpriteObject::Initialize(EngineSystem* engine, const std::string& textureFilePath) {
    engine_ = engine;
    
    // Sprite初期化（テクスチャサイズ自動設定）
    sprite_ = std::make_unique<Sprite>();
    auto* renderManager = engine_->GetComponent<RenderManager>();
    auto* spriteRenderer = dynamic_cast<SpriteRenderer*>(renderManager->GetRenderer(RenderPassType::Sprite));
    sprite_->Initialize(spriteRenderer, textureFilePath);
    
    // テクスチャの読み込み
    textureHandle_ = TextureManager::GetInstance().Load(textureFilePath);
    
    // トランスフォーム初期化
    transform_.scale = { 1.0f, 1.0f, 1.0f };
    transform_.rotate = { 0.0f, 0.0f, 0.0f };
    transform_.translate = { 0.0f, 0.0f, 0.0f };
    
    // アクティブ状態
    isActive_ = true;
}

void SpriteObject::Update() {
    if (!sprite_ || !isActive_) return;
    
    // Spriteのトランスフォームを更新
    sprite_->SetPosition(transform_.translate);
    sprite_->SetScale(transform_.scale);
    sprite_->SetRotate(transform_.rotate);
}

void SpriteObject::Draw() {
    if (!sprite_ || !isActive_) return;
    
    // Sprite描画
    sprite_->Draw(textureHandle_.gpuHandle);
}

bool SpriteObject::DrawImGui() {
    if (!sprite_) return false;
    
    bool changed = false;
    
    // スプライトごとにユニークなラベルを生成
    std::string header = std::string(GetObjectName()) + " (" + std::to_string(reinterpret_cast<uintptr_t>(this)) + ")";
    
    if (ImGui::CollapsingHeader(header.c_str())) {
        ImGui::PushID(this);
        
        // アクティブ状態の切り替え
        bool active = isActive_;
        if (ImGui::Checkbox("Active", &active)) {
            isActive_ = active;
            changed = true;
        }
        
        ImGui::Separator();
        
        // Spriteの内部ImGuiを呼び出し（UV変換はSprite内部で管理）
        if (sprite_->DrawImGui("Properties")) {
            changed = true;
        }
        
        ImGui::PopID();
    }
    
    return changed;
}

void SpriteObject::SetTexture(const std::string& textureFilePath) {
    textureHandle_ = TextureManager::GetInstance().Load(textureFilePath);
}
