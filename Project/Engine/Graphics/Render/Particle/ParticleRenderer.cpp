#include "ParticleRenderer.h"
#include "Engine/Particle/ParticleSystem.h"
#include "Engine/Graphics/Resource/ResourceFactory.h"
#include "Engine/Camera/ICamera.h"
#include <cassert>

void ParticleRenderer::Initialize(ID3D12Device* device) {
    device_ = device;

    // リソースファクトリが設定されているか確認
    assert(resourceFactory_ != nullptr && "ResourceFactory must be set before initialization");

    // パイプラインとシェーダーマネージャーの初期化
    pipelineMg_ = std::make_unique<PipelineStateManager>();
    rootSignatureMg_ = std::make_unique<RootSignatureManager>();
    shaderCompiler_ = std::make_unique<ShaderCompiler>();

    // シェーダーコンパイラの初期化
    shaderCompiler_->Initialize();

    // ルートシグネチャとPSOの作成
    CreateRootSignature();
    CreatePSO();

    // 共有頂点バッファの作成
    CreateSharedVertexBuffer();
}

void ParticleRenderer::BeginPass(ID3D12GraphicsCommandList* cmdList, BlendMode blendMode) {
    cmdList_ = cmdList;

    // ルートシグネチャとパイプラインステートを設定
    cmdList_->SetGraphicsRootSignature(rootSignatureMg_->GetRootSignature());
    cmdList_->SetPipelineState(pipelineMg_->GetPipelineState(blendMode));

    // プリミティブトポロジを設定
    cmdList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 共有頂点バッファを設定
    cmdList_->IASetVertexBuffers(0, 1, &vertexBufferView_);
}

void ParticleRenderer::EndPass() {
    cmdList_ = nullptr;
}

void ParticleRenderer::SetCamera(const ICamera* camera) {
    camera_ = camera;
}

void ParticleRenderer::Draw(ParticleSystem* particle) {
    if (!cmdList_ || !particle || !particle->IsActive()) {
        return;
    }

    uint32_t instanceCount = particle->GetInstanceCount();
    if (instanceCount == 0) {
        return;
    }

    // インスタンシングリソースを設定
    cmdList_->SetGraphicsRootDescriptorTable(0, particle->GetInstancingSrvHandleGPU());
    cmdList_->SetGraphicsRootConstantBufferView(1, particle->GetMaterialGPUAddress());
    cmdList_->SetGraphicsRootDescriptorTable(2, particle->GetTextureHandle());

    // 描画コマンドを発行
    cmdList_->DrawInstanced(6, instanceCount, 0, 0);
}

void ParticleRenderer::CreateRootSignature() {
    // Root Parameter 0: インスタンシング用SRV (t0, Vertex Shader)
    RootSignatureManager::DescriptorRangeConfig instanceRange;
    instanceRange.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    instanceRange.numDescriptors = 1;
    instanceRange.baseShaderRegister = 0;  // t0
    rootSignatureMg_->AddDescriptorTable({ instanceRange }, D3D12_SHADER_VISIBILITY_VERTEX);

    // Root Parameter 1: マテリアル用CBV (b0, Pixel Shader)
    RootSignatureManager::RootDescriptorConfig materialCBV;
    materialCBV.shaderRegister = 0;
    materialCBV.visibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootSignatureMg_->AddRootCBV(materialCBV);

    // Root Parameter 2: テクスチャ用ディスクリプタテーブル (t0, Pixel Shader)
    RootSignatureManager::DescriptorRangeConfig textureRange;
    textureRange.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    textureRange.numDescriptors = 1;
    textureRange.baseShaderRegister = 0;  // t0
    rootSignatureMg_->AddDescriptorTable({ textureRange }, D3D12_SHADER_VISIBILITY_PIXEL);

    // Static Sampler (s0, Pixel Shader)
    rootSignatureMg_->AddDefaultLinearSampler(0, D3D12_SHADER_VISIBILITY_PIXEL);

    // RootSignature の作成
    rootSignatureMg_->Create(device_);
}

void ParticleRenderer::CreatePSO() {
    // パーティクルのシェーダーコンパイル
    auto vertexShaderBlob = shaderCompiler_->CompileShader(L"Resources/Shader/Particle/Particle.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);

    auto pixelShaderBlob = shaderCompiler_->CompileShader(L"Resources/Shader/Particle/Particle.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

    // ビルダーパターンでPSOを構築
    bool result = pipelineMg_->CreateBuilder()
        .AddInputElement("POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)
        .AddInputElement("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)
        .AddInputElement("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)
        .SetRasterizer(D3D12_CULL_MODE_BACK, D3D12_FILL_MODE_SOLID)
        .SetDepthStencil(false, true)
        .SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
        .BuildAllBlendModes(device_, vertexShaderBlob, pixelShaderBlob, rootSignatureMg_->GetRootSignature());

    if (!result) {
        throw std::runtime_error("Failed to create PSO in ParticleRenderer");
    }
}

void ParticleRenderer::CreateSharedVertexBuffer() {
    // パーティクル用の頂点データ（四角形）
    std::vector<VertexData> particleVertices = {
        { .position = { -1.0f,  1.0f, 0.0f, 1.0f }, .texcoord = { 0.0f, 0.0f }, .normal = { 0.0f, 0.0f, -1.0f } }, // 左上
        { .position = {  1.0f,  1.0f, 0.0f, 1.0f }, .texcoord = { 1.0f, 0.0f }, .normal = { 0.0f, 0.0f, -1.0f } }, // 右上
        { .position = {  1.0f, -1.0f, 0.0f, 1.0f }, .texcoord = { 1.0f, 1.0f }, .normal = { 0.0f, 0.0f, -1.0f } }, // 右下

        { .position = { -1.0f,  1.0f, 0.0f, 1.0f }, .texcoord = { 0.0f, 0.0f }, .normal = { 0.0f, 0.0f, -1.0f } }, // 左上
        { .position = {  1.0f, -1.0f, 0.0f, 1.0f }, .texcoord = { 1.0f, 1.0f }, .normal = { 0.0f, 0.0f, -1.0f } }, // 右下
        { .position = { -1.0f, -1.0f, 0.0f, 1.0f }, .texcoord = { 0.0f, 1.0f }, .normal = { 0.0f, 0.0f, -1.0f } }  // 左下
    };

    size_t vertexBufferSize = sizeof(VertexData) * particleVertices.size();

    // 頂点バッファのリソースを作成
    vertexBuffer_ = resourceFactory_->CreateBufferResource(device_, vertexBufferSize);

    // 頂点バッファにデータをコピー
    VertexData* vertexData = nullptr;
    vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, particleVertices.data(), vertexBufferSize);

    // 頂点バッファビューの設定
    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(vertexBufferSize);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
}
