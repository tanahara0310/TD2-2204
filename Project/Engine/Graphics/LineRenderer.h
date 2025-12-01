#pragma once

#include <d3d12.h>
#include <vector>
#include <wrl.h>
#include <memory>

#include "Math/Vector/Vector3.h"
#include "MathCore.h"
#include "Engine/ObjectCommon/IDrawable.h"

// 前方宣言
class LineRendererPipeline;
class ICamera;

/// @brief ライン管理クラス（IDrawableを継承）
class LineRenderer : public IDrawable {
public:
    /// @brief ライン構造体
    struct Line {
      Vector3 start;
   Vector3 end;
        Vector3 color;
        float alpha;
    };

    LineRenderer() = default;
    ~LineRenderer() override = default;

    /// @brief 初期化
    /// @param rendererPipeline LineRendererPipelineへのポインタ
    void Initialize(LineRendererPipeline* rendererPipeline);

    // IDrawableインターフェースの実装
    void Update() override;
    void Draw(const ICamera* camera) override;
    bool DrawImGui() override { return false; }
    RenderPassType GetRenderPassType() const override { return RenderPassType::Line; }
    const char* GetObjectName() const override { return "LineRenderer"; }

    /// @brief ラインを追加
    /// @param line 追加するライン
    void AddLine(const Line& line);

    /// @brief 複数のラインを追加
    /// @param lines 追加するライン配列
    void AddLines(const std::vector<Line>& lines);

    /// @brief 単一ラインを描画（即座に描画）
    /// @param camera カメラ
    /// @param line 描画するライン
    void DrawLine(const ICamera* camera, const Line& line);

    /// @brief デバッグ用：球体をラインで描画
    /// @param camera カメラ
    /// @param center 球体の中心座標
    /// @param radius 球体の半径
    /// @param color ラインの色
    /// @param alpha ラインの透明度
    /// @param segments 分割数（デフォルト16）
    void DrawSphere(const ICamera* camera, const Vector3& center, float radius, 
        const Vector3& color = {1.0f, 1.0f, 1.0f}, float alpha = 1.0f, int segments = 16);

    /// @brief デバッグ用：ボックスをラインで描画
    /// @param camera カメラ
    /// @param center ボックスの中心座標
    /// @param size ボックスのサイズ
/// @param color ラインの色
    /// @param alpha ラインの透明度
    void DrawBox(const ICamera* camera, const Vector3& center, const Vector3& size,
        const Vector3& color = {1.0f, 1.0f, 1.0f}, float alpha = 1.0f);

    /// @brief デバッグ用：円をラインで描画
    /// @param camera カメラ
    /// @param center 円の中心座標
    /// @param radius 円の半径
    /// @param normal 円の法線方向
    /// @param color ラインの色
    /// @param alpha ラインの透明度
    /// @param segments 分割数（デフォルト32）
    void DrawCircle(const ICamera* camera, const Vector3& center, float radius, const Vector3& normal,
      const Vector3& color = {1.0f, 1.0f, 1.0f}, float alpha = 1.0f, int segments = 32);

    /// @brief デバッグ用：コーンをラインで描画
    /// @param camera カメラ
    /// @param apex コーンの頂点
    /// @param direction コーンの方向
    /// @param height コーンの高さ
    /// @param angle コーンの角度（度）
    /// @param color ラインの色
    /// @param alpha ラインの透明度
    /// @param segments 分割数（デフォルト16）
    void DrawCone(const ICamera* camera, const Vector3& apex, const Vector3& direction, 
        float height, float angle, const Vector3& color = {1.0f, 1.0f, 1.0f}, 
     float alpha = 1.0f, int segments = 16);

    /// @brief デバッグ用：円柱をラインで描画
    /// @param camera カメラ
    /// @param center 円柱の中心座標
    /// @param radius 円柱の半径
    /// @param height 円柱の高さ
    /// @param direction 円柱の方向
    /// @param color ラインの色
    /// @param alpha ラインの透明度
    /// @param segments 分割数（デフォルト16）
    void DrawCylinder(const ICamera* camera, const Vector3& center, float radius, 
        float height, const Vector3& direction, const Vector3& color = {1.0f, 1.0f, 1.0f}, 
    float alpha = 1.0f, int segments = 16);

    /// @brief 全てのラインをクリア
    void Clear();

/// @brief ライン数を取得
    /// @return 現在のライン数
    size_t GetLineCount() const { return lines_.size(); }

private:
  LineRendererPipeline* rendererPipeline_ = nullptr;
    std::vector<Line> lines_;
};