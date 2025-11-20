#pragma once

#include "Object2d.h"
#include "Engine/Graphics/Sprite/Sprite.h"
#include "Engine/Graphics/TextureManager.h"
#include <memory>
#include <string>

/// @brief スプライトオブジェクト - Object2d基底クラスを継承してRenderManager対応
class SpriteObject : public Object2d {
public:
    /// @brief コンストラクタ
    SpriteObject() = default;
    
    /// @brief デストラクタ
    ~SpriteObject() override = default;
    
    /// @brief 初期化（テクスチャ指定版 - 推奨）
    /// @param engine エンジンシステム
    /// @param textureFilePath テクスチャファイルパス
    void Initialize(const std::string& textureFilePath);
    
    /// @brief 更新
    void Update() override;
    
    /// @brief 描画（2D専用）
    void Draw() override;
    
    /// @brief デバッグUI描画
    /// @return 変更があった場合true
    bool DrawImGui() override;
    
    /// @brief オブジェクト名を取得
    /// @return オブジェクト名
    const char* GetObjectName() const override { return "Sprite"; }
    
    /// @brief このオブジェクトの描画パスタイプを取得
    /// @return 描画パスタイプ（Sprite）
    RenderPassType GetRenderPassType() const override { return RenderPassType::Sprite; }
    
    /// @brief Spriteインスタンスを取得
    /// @return Spriteへのポインタ
    Sprite* GetSprite() { return sprite_.get(); }
    
    /// @brief テクスチャハンドルを設定
    /// @param textureFilePath テクスチャファイルパス
    void SetTexture(const std::string& textureFilePath);
    
private:
    
    /// @brief Spriteインスタンス
    std::unique_ptr<Sprite> sprite_;
    
    /// @brief テクスチャハンドル
    TextureManager::LoadedTexture textureHandle_;
};
