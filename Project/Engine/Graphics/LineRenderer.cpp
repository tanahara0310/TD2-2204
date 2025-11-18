#include "LineRenderer.h"

#include <cassert>

#include "Engine/Graphics/Resource/ResourceFactory.h"
#include "Engine/Graphics/Shader/ShaderCompiler.h"
#include "MathCore.h"

using namespace MathCore;

void LineRenderer::Initialize(ID3D12Device* device)
{
    uint32_t bufferSize = static_cast<uint32_t>(sizeof(LineVertex) * maxVertexCount_);

    // 頂点バッファ作成
    D3D12_HEAP_PROPERTIES heapProp = { D3D12_HEAP_TYPE_UPLOAD };
    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = bufferSize;
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer_));

    // 定数バッファ作成
    wvpBuffer_ = ResourceFactory::CreateBufferResource(device, sizeof(Matrix4x4));
    wvpBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&wvpData_));

    // 頂点バッファビューの設定
    vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vbView_.SizeInBytes = bufferSize;
    vbView_.StrideInBytes = sizeof(LineVertex);

    // シェーダーのコンパイル（Lineシェーダーを使用）
    ShaderCompiler compiler;
    compiler.Initialize();
    auto vertexShaderBlob = compiler.CompileShader(L"Resources/Shader/Line/Line.VS.hlsl", L"vs_6_0");
    assert(vertexShaderBlob != nullptr);

    auto pixelShaderBlob = compiler.CompileShader(L"Resources/Shader/Line/Line.PS.hlsl", L"ps_6_0");
    assert(pixelShaderBlob != nullptr);

    // ルートシグネチャの作成
    RootSignatureManager::RootDescriptorConfig cbvConfig;
    cbvConfig.shaderRegister = 0;  // b0
    cbvConfig.visibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rsManager_.AddRootCBV(cbvConfig);
    
    rsManager_.SetFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    rsManager_.Create(device);
    rootSignature_ = rsManager_.GetRootSignature();

    // ビルダーパターンでPSOを構築
    bool result = psoManager_.CreateBuilder()
        .AddInputElement("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0)
        .AddInputElement("COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)
        .AddInputElement("ALPHA", 0, DXGI_FORMAT_R32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT)
        .SetRasterizer(D3D12_CULL_MODE_NONE, D3D12_FILL_MODE_SOLID)
        .SetDepthStencil(false, false) // ラインは深度テスト無効
        .SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE)
        .BuildAllBlendModes(device, vertexShaderBlob, pixelShaderBlob, rootSignature_.Get());
    
    if (!result) {
		throw std::runtime_error("Failed to create pipeline state for LineRenderer.");
    }
    
    // アルファブレンドを使用
    pipelineState_ = psoManager_.GetPipelineState(BlendMode::kBlendModeNormal);
}

void LineRenderer::Draw(ID3D12GraphicsCommandList* cmdList, const Matrix4x4& view, const Matrix4x4& proj, const std::vector<Line>& lines)
{
    if (lines.empty()) {
        return;
    }

    // 頂点データの更新
    UpdateVertexBuffer(lines);

    // 描画の前処理
    PreDraw(cmdList, view, proj);

    // 描画実行
    cmdList->DrawInstanced(static_cast<UINT>(vertices_.size()), 1, 0, 0);
}

void LineRenderer::DrawLine(ID3D12GraphicsCommandList* cmdList, const Matrix4x4& view, const Matrix4x4& proj, const Line& line)
{
    std::vector<Line> singleLine = { line };
    Draw(cmdList, view, proj, singleLine);
}

void LineRenderer::DrawSphere(ID3D12GraphicsCommandList* cmdList, const Matrix4x4& view, const Matrix4x4& proj, 
    const Vector3& center, float radius, const Vector3& color, float alpha, int segments)
{
std::vector<Line> lines;

// 緯度線を描画（複数の水平円）
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = (static_cast<float>(lat) / segments) * std::numbers::pi_v<float>;
   float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);
    
        for (int lon = 0; lon < segments; ++lon) {
            float phi1 = (static_cast<float>(lon) / segments) * 2.0f * std::numbers::pi_v<float>;
     float phi2 = (static_cast<float>(lon + 1) / segments) * 2.0f * std::numbers::pi_v<float>;
            
    float sinPhi1 = std::sin(phi1);
            float cosPhi1 = std::cos(phi1);
       float sinPhi2 = std::sin(phi2);
        float cosPhi2 = std::cos(phi2);
            
            Vector3 p1 = {
          center.x + radius * sinTheta * cosPhi1,
                center.y + radius * cosTheta,
         center.z + radius * sinTheta * sinPhi1
  };
            
            Vector3 p2 = {
     center.x + radius * sinTheta * cosPhi2,
                center.y + radius * cosTheta,
 center.z + radius * sinTheta * sinPhi2
            };
            
lines.push_back({ p1, p2, color, alpha });
}
    }
    
    // 経度線を描画（縦の線）
    for (int lon = 0; lon < segments; ++lon) {
        float phi = (static_cast<float>(lon) / segments) * 2.0f * std::numbers::pi_v<float>;
        float sinPhi = std::sin(phi);
  float cosPhi = std::cos(phi);
        
        for (int lat = 0; lat < segments; ++lat) {
            float theta1 = (static_cast<float>(lat) / segments) * std::numbers::pi_v<float>;
  float theta2 = (static_cast<float>(lat + 1) / segments) * std::numbers::pi_v<float>;
       
float sinTheta1 = std::sin(theta1);
        float cosTheta1 = std::cos(theta1);
   float sinTheta2 = std::sin(theta2);
     float cosTheta2 = std::cos(theta2);
     
            Vector3 p1 = {
                center.x + radius * sinTheta1 * cosPhi,
    center.y + radius * cosTheta1,
            center.z + radius * sinTheta1 * sinPhi
            };
            
  Vector3 p2 = {
     center.x + radius * sinTheta2 * cosPhi,
 center.y + radius * cosTheta2,
              center.z + radius * sinTheta2 * sinPhi
};
            
     lines.push_back({ p1, p2, color, alpha });
     }
    }
    
    Draw(cmdList, view, proj, lines);
}

void LineRenderer::PreDraw(ID3D12GraphicsCommandList* cmdList, const Matrix4x4& view, const Matrix4x4& proj)
{
    // ビュープロジェクション行列の更新
    *wvpData_ = Matrix::Multiply(view, proj);

    // パイプラインの設定
    cmdList->SetGraphicsRootSignature(rootSignature_.Get());
    cmdList->SetPipelineState(pipelineState_.Get());
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    cmdList->IASetVertexBuffers(0, 1, &vbView_);
    cmdList->SetGraphicsRootConstantBufferView(0, wvpBuffer_->GetGPUVirtualAddress());
}

void LineRenderer::UpdateVertexBuffer(const std::vector<Line>& lines)
{
    vertices_.clear();

    // ラインを頂点データに変換
    for (const auto& line : lines) {
        vertices_.push_back({ line.start, line.color, line.alpha });
        vertices_.push_back({ line.end, line.color, line.alpha });
    }

    // 頂点バッファの更新
    uint32_t sizeInBytes = static_cast<uint32_t>(sizeof(LineVertex) * vertices_.size());
    void* map = nullptr;
    vertexBuffer_->Map(0, nullptr, &map);
    memcpy(map, vertices_.data(), sizeInBytes);
    vertexBuffer_->Unmap(0, nullptr);
    vbView_.SizeInBytes = sizeInBytes;
}