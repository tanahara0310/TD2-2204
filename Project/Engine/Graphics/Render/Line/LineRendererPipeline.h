#pragma once
#include "Engine/Graphics/Render/IRenderer.h"
#include "Engine/Graphics/PipelineStateManager.h"
#include "Engine/Graphics/RootSignatureManager.h"
#include "Engine/Graphics/Shader/ShaderCompiler.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Resource/ResourceFactory.h"
#include "MathCore.h"
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include <vector>

// 前方宣言
class ICamera;

// Line用 Root Parameter インデックス定数
namespace LineRendererRootParam {
    static constexpr UINT kWVP = 0;  // WVP行列用CBV (b0, VS)
}

/// @brief ライン描画用レンダラーパイプライン
class LineRendererPipeline : public IRenderer {
public:
    /// @brief ライン頂点データ
    struct LineVertex {
        Vector3 position;
        Vector3 color;
        float alpha;
    };

    /// @brief 最大頂点数
    static constexpr uint32_t kMaxVertexCount = 65536;

    // IRendererインターフェースの実装
    void Initialize(ID3D12Device* device) override;
    void BeginPass(ID3D12GraphicsCommandList* cmdList, BlendMode blendMode) override;
    void EndPass() override;
    RenderPassType GetRenderPassType() const override { return RenderPassType::Line; }
    void SetCamera(const ICamera* camera) override;

    /// @brief 初期化（DirectXCommonとResourceFactory付き）
    /// @param dxCommon DirectXCommon
    /// @param resourceFactory ResourceFactory
    void Initialize(DirectXCommon* dxCommon, ResourceFactory* resourceFactory);

    /// @brief ルートシグネチャを取得
    ID3D12RootSignature* GetRootSignature() const { return rootSignatureMg_->GetRootSignature(); }

    /// @brief 頂点バッファを更新
    /// @param vertices 頂点データ
    void UpdateVertexBuffer(const std::vector<LineVertex>& vertices);

    /// @brief ラインを描画
    /// @param cmdList コマンドリスト
    /// @param vertexCount 頂点数
    void DrawLines(ID3D12GraphicsCommandList* cmdList, uint32_t vertexCount);

    /// @brief WVP行列を設定
    /// @param view ビュー行列
    /// @param proj プロジェクション行列
    void SetWVPMatrix(const Matrix4x4& view, const Matrix4x4& proj);

    /// @brief DirectXCommonを取得
    DirectXCommon* GetDirectXCommon() { return dxCommon_; }

    /// @brief ResourceFactoryを取得
    ResourceFactory* GetResourceFactory() { return resourceFactory_; }

private:
    std::unique_ptr<RootSignatureManager> rootSignatureMg_ = std::make_unique<RootSignatureManager>();
    std::unique_ptr<PipelineStateManager> psoMg_ = std::make_unique<PipelineStateManager>();
    std::unique_ptr<ShaderCompiler> shaderCompiler_ = std::make_unique<ShaderCompiler>();

    ID3D12PipelineState* pipelineState_ = nullptr;
    BlendMode currentBlendMode_ = BlendMode::kBlendModeNormal;

    // DirectXCommonとResourceFactory
    DirectXCommon* dxCommon_ = nullptr;
    ResourceFactory* resourceFactory_ = nullptr;

    // カメラ
    const ICamera* camera_ = nullptr;

    // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vbView_{};
    std::vector<LineVertex> vertices_;

    // WVP行列バッファ
    Microsoft::WRL::ComPtr<ID3D12Resource> wvpBuffer_;
    Matrix4x4* wvpData_ = nullptr;

    // コマンドリスト（BeginPass/EndPassで使用）
    ID3D12GraphicsCommandList* currentCmdList_ = nullptr;
};
