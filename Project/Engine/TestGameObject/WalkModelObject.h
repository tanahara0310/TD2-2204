#pragma once

#include "Object3D.h"
#include <memory>

/// @brief Walkモデルオブジェクト
class WalkModelObject : public Object3d {
public:
    /// @brief 初期化処理
    /// @param engine エンジンシステムへのポインタ
    void Initialize();

    /// @brief 更新処理
    void Update() override;

    /// @brief 描画処理
    /// @param camera カメラ
    void Draw(const ICamera* camera) override;

    /// @brief デバッグ描画処理
    /// @param outLines ライン配列の出力先
    void DrawDebug(std::vector<LineRenderer::Line>& outLines) override;

    /// @brief ImGuiデバッグUI描画
    /// @return ImGuiで変更があった場合true
    bool DrawImGui() override;

    /// @brief オブジェクト名を取得
    /// @return オブジェクト名
    const char* GetObjectName() const override { return "WalkModel"; }

    /// @brief Skeleton描画ON/OFF
    /// @param enable 有効にする場合true
    void SetDrawSkeleton(bool enable) { drawSkeleton_ = enable; }

    /// @brief Skeleton描画ON/OFFを取得
    /// @return Skeleton描画が有効ならtrue
    bool GetDrawSkeleton() const { return drawSkeleton_; }

    /// @brief Joint半径を設定
    /// @param radius Joint半径
    void SetJointRadius(float radius) { jointRadius_ = radius; }

    /// @brief Joint半径を取得
    /// @return Joint半径
    float GetJointRadius() const { return jointRadius_; }

private:
    EngineSystem* engine_ = nullptr;  // エンジンシステムへのポインタ
    bool drawSkeleton_ = true;    // Skeleton描画フラグ
    float jointRadius_ = 0.05f;    // Joint半径
    float animationTime_ = 0.0f;   // アニメーション時刻
    bool animationInitialized_ = false;  // アニメーション初期化フラグ
    TextureManager::LoadedTexture uvCheckerTexture_;  // テクスチャハンドル
};
