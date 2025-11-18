#pragma once

#include "../IRenderer.h"
#include "Engine/Graphics/PipelineStateManager.h"
#include "Engine/Graphics/RootSignatureManager.h"
#include "Engine/Graphics/Shader/ShaderCompiler.h"
#include "Engine/Graphics/Structs/VertexData.h"
#include <d3d12.h>
#include <wrl.h>
#include <memory>

// 前方宣言
class ParticleSystem;
class ICamera;
class ResourceFactory;

/// @brief パーティクル専用レンダラー
class ParticleRenderer : public IRenderer {
public:
    ParticleRenderer() = default;
    ~ParticleRenderer() override = default;

    /// @brief 初期化
    /// @param device D3D12デバイス
    void Initialize(ID3D12Device* device) override;

    /// @brief 描画パスの開始
    /// @param cmdList コマンドリスト
    /// @param blendMode ブレンドモード
    void BeginPass(ID3D12GraphicsCommandList* cmdList, BlendMode blendMode = BlendMode::kBlendModeNone) override;

    /// @brief 描画パスの終了
    void EndPass() override;

    /// @brief このレンダラーがサポートする描画タイプを取得
    /// @return 描画パスタイプ
    RenderPassType GetRenderPassType() const override { return RenderPassType::Particle; }

    /// @brief カメラを設定
    /// @param camera カメラオブジェクト
    void SetCamera(const ICamera* camera) override;

    /// @brief ResourceFactoryを設定（初期化前に呼び出す必要がある）
    /// @param resourceFactory リソースファクトリ
    void SetResourceFactory(ResourceFactory* resourceFactory) { resourceFactory_ = resourceFactory; }

    /// @brief パーティクルシステムを描画
    /// @param particle パーティクルシステム
    void Draw(ParticleSystem* particle);

private:
    // リソース管理
    ResourceFactory* resourceFactory_ = nullptr;
    ID3D12Device* device_ = nullptr;
    ID3D12GraphicsCommandList* cmdList_ = nullptr;
    const ICamera* camera_ = nullptr;

    // パイプラインとシェーダー
    std::unique_ptr<PipelineStateManager> pipelineMg_;
    std::unique_ptr<RootSignatureManager> rootSignatureMg_;
    std::unique_ptr<ShaderCompiler> shaderCompiler_;

    // 頂点バッファ（全パーティクルシステムで共有）
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};

    /// @brief ルートシグネチャの作成
    void CreateRootSignature();

    /// @brief パイプラインステートオブジェクトの作成
    void CreatePSO();

    /// @brief 共有頂点バッファの作成
    void CreateSharedVertexBuffer();
};
