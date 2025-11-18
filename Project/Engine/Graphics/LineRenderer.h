#pragma once

#include <d3d12.h>
#include <vector>
#include <wrl.h>

#include "Engine/Graphics/PipelineStateManager.h"
#include "Engine/Graphics/RootSignatureManager.h"
#include "Math/Vector/Vector3.h"
#include "MathCore.h"

class LineRenderer {
public:
    struct LineVertex {
        Vector3 position;
        Vector3 color;
        float alpha;
    };

    struct Line {
        Vector3 start;
        Vector3 end;
        Vector3 color;
        float alpha;
    };

    /// @brief ラインレンダラーの初期化
    /// @param device DirectX12デバイス
    void Initialize(ID3D12Device* device);

    /// @brief ラインの描画
    /// @param cmdList コマンドリスト
    /// @param view ビュー行列
    /// @param proj プロジェクション行列
    /// @param lines 描画するライン配列
    void Draw(ID3D12GraphicsCommandList* cmdList, const Matrix4x4& view, const Matrix4x4& proj, const std::vector<Line>& lines);

    /// @brief 単一ラインの描画
    /// @param cmdList コマンドリスト
    /// @param view ビュー行列
    /// @param proj プロジェクション行列
    /// @param line 描画するライン
    void DrawLine(ID3D12GraphicsCommandList* cmdList, const Matrix4x4& view, const Matrix4x4& proj, const Line& line);

    /// @brief デバッグ用：球体をラインで描画
    /// @param cmdList コマンドリスト
    /// @param view ビュー行列
    /// @param proj プロジェクション行列
    /// @param center 球体の中心座標
    /// @param radius 球体の半径
    /// @param color ラインの色
    /// @param alpha ラインの透明度
    /// @param segments 分割数（デフォルト16）
    void DrawSphere(ID3D12GraphicsCommandList* cmdList, const Matrix4x4& view, const Matrix4x4& proj, 
         const Vector3& center, float radius, const Vector3& color = {1.0f, 1.0f, 1.0f}, 
         float alpha = 1.0f, int segments = 16);

private:
    /// @brief 描画の前処理
    /// @param cmdList コマンドリスト
    /// @param view ビュー行列
    /// @param proj プロジェクション行列
    void PreDraw(ID3D12GraphicsCommandList* cmdList, const Matrix4x4& view, const Matrix4x4& proj);

    /// @brief 頂点データの更新
    /// @param lines ライン配列
    void UpdateVertexBuffer(const std::vector<Line>& lines);

private:
    PipelineStateManager psoManager_;
    RootSignatureManager rsManager_;

    // ワールド・ビュー・プロジェクション行列のバッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> wvpBuffer_;
    Matrix4x4* wvpData_ = nullptr;

    // 頂点バッファ
    std::vector<LineVertex> vertices_;
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vbView_{};
    uint32_t maxVertexCount_ = 65536; // デフォルトの最大頂点数

    // PSOとルートシグネチャ
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
};