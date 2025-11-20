#pragma once

#include "Object3D.h"
#include <memory>

/// @brief SneakWalkモデルオブジェクト
class SneakWalkModelObject : public Object3d {
public:
    /// @brief 初期化処理
    /// @param engine エンジンシステムへのポインタ
    void Initialize();

    /// @brief 更新処理
    void Update() override;

    /// @brief 描画処理
    /// @param camera カメラ
    void Draw(ICamera* camera) override;

    /// @brief デバッグ描画処理
    /// @param outLines ライン配列の出力先
    void DrawDebug(std::vector<LineRenderer::Line>& outLines) override;

    /// @brief ImGuiデバッグUI描画
    /// @return ImGuiで変更があった場合true
    bool DrawImGui() override;

    /// @brief オブジェクト名を取得
    /// @return オブジェクト名
    const char* GetObjectName() const override { return "SneakWalkModel"; }

    /// @brief スケルトン描画フラグを設定
    /// @param draw 描画するか
    void SetDrawSkeleton(bool draw) { drawSkeleton_ = draw; }

    /// @brief スケルトン描画フラグを取得
    /// @return 描画するならtrue
    bool GetDrawSkeleton() const { return drawSkeleton_; }

    /// @brief Joint半径を設定
    /// @param radius Joint半径
    void SetJointRadius(float radius) { jointRadius_ = radius; }

    /// @brief Joint半径を取得
    /// @return Joint半径
    float GetJointRadius() const { return jointRadius_; }

private:
    EngineSystem* engine_ = nullptr;
    bool drawSkeleton_ = true;
    float jointRadius_ = 0.05f;
    TextureManager::LoadedTexture uvCheckerTexture_;  // テクスチャハンドル
};
