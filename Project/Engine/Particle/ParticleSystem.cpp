#include "ParticleSystem.h"
#include "Engine/Utility/Random/RandomGenerator.h"
#include <iostream>
#ifdef _DEBUG
#include "Engine/Utility/Debug/ImGui/ImguiManager.h"
#endif

using namespace MathCore;

// 初期化関数
void ParticleSystem::Initialize(DirectXCommon* dxCommon, ResourceFactory* resourceFactory)
{
    dxCommon_ = dxCommon;
    resourceFactory_ = resourceFactory;

    // 統一乱数エンジンの初期化
    RandomGenerator::GetInstance().Initialize();

    // モジュールの初期化
    emissionModule_ = std::make_unique<EmissionModule>();
    velocityModule_ = std::make_unique<VelocityModule>();
    colorModule_ = std::make_unique<ColorModule>();
    lifetimeModule_ = std::make_unique<LifetimeModule>();
    forceModule_ = std::make_unique<ForceModule>();
    sizeModule_ = std::make_unique<SizeModule>();
    rotationModule_ = std::make_unique<RotationModule>();

    // エミッタートランスフォームの初期化
    emitterTransform_ = {
        { 1.0f, 1.0f, 1.0f }, // スケール
        { 0.0f, 0.0f, 0.0f }, // 回転
        { 0.0f, 0.0f, 0.0f }  // 平行移動
    };

    // マテリアルクラスの初期化
    materialManager_->Initialize(dxCommon->GetDevice(), resourceFactory);

    // シェーダークラスの初期化
    shaderCompiler_->Initialize();

    // ルートシグネチャの初期化
    CreateRootSignature();

    // パイプラインステートの初期化
    CreatePSO();

    // インスタンシング用のリソースを作成
    ResourceCreate();

    // SRVの作成
    CreateSRV();
}

// 更新処理関数
void ParticleSystem::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix)
{
    ShowImGui();

    const float kDeltaTime = 1.0f / 60.0f; // フレームレートを60FPSと仮定
    Matrix4x4 viewProjectionMatrix = Matrix::Multiply(viewMatrix, projectionMatrix);

    // 統計情報の更新
    statistics_.systemRuntime += kDeltaTime;
    deltaTimeAccumulator_ += kDeltaTime;

    // ビルボード行列を作成
    Matrix4x4 billboardMatrix = CreateBillboardMatrix(viewMatrix);

    // エミッションモジュールの更新
    emissionModule_->UpdateTime(kDeltaTime);
    uint32_t emissionCount = emissionModule_->CalculateEmissionCount(kDeltaTime);
    
    if (emissionCount > 0) {
        EmitParticles(emissionCount);
        // 統計情報の更新
        statistics_.totalParticlesCreated += emissionCount;
    }

    // パーティクルの更新前の数を記録
    uint32_t particleCountBefore = GetParticleCount();

    // パーティクルの更新
    UpdateParticles(kDeltaTime, viewProjectionMatrix, billboardMatrix);

    // パーティクルの更新後の統計情報を更新
    uint32_t currentParticleCount = GetParticleCount();
    if (currentParticleCount > statistics_.peakParticleCount) {
        statistics_.peakParticleCount = currentParticleCount;
    }

    // 破棄されたパーティクル数を計算（新規作成分を差し引く）
    if (particleCountBefore + emissionCount > currentParticleCount) {
        uint32_t destroyedCount = (particleCountBefore + emissionCount) - currentParticleCount;
        statistics_.totalParticlesDestroyed += destroyedCount;
    }

    // 平均ライフタイムの計算（1秒ごとに更新）
    if (deltaTimeAccumulator_ >= 1.0f) {
        if (statistics_.totalParticlesDestroyed > 0) {
            // 簡易的な平均ライフタイム計算
            auto lifetimeData = lifetimeModule_->GetLifetimeData();
            statistics_.averageLifetime = lifetimeData.startLifetime;
        }
        deltaTimeAccumulator_ = 0.0f;
    }
}

// 描画関数
void ParticleSystem::Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle)
{
    // ルートシグネチャとパイプラインステートを設定
    commandList->SetGraphicsRootSignature(rootSignatureMg_->GetRootSignature());
    commandList->SetPipelineState(pipelineMg_->GetPipelineState(blendMode_));
    
    // プリミティブトポロジを設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // パーティクルのインスタンシングリソースを設定
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->SetGraphicsRootDescriptorTable(0, instancingSrvHandleGPU_);
    commandList->SetGraphicsRootConstantBufferView(1, materialManager_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootDescriptorTable(2, textureHandle);
    
    if (instanceCount_ > 0) {
        // 描画コマンドを発行
        commandList->DrawInstanced(6, instanceCount_, 0, 0);
    }
}

void ParticleSystem::Play()
{
    emissionModule_->Play();
}

void ParticleSystem::Stop()
{
    emissionModule_->Stop();
}

bool ParticleSystem::IsPlaying() const
{
    return emissionModule_->IsPlaying();
}

void ParticleSystem::Clear()
{
    particles_.clear();
    instanceCount_ = 0;
}

void ParticleSystem::EmitParticles(uint32_t count)
{
    for (uint32_t i = 0; i < count && particles_.size() < kNumMaxInstance; ++i) {
        Particle newParticle = CreateNewParticle();
        particles_.push_back(newParticle);
    }
}

Particle ParticleSystem::CreateNewParticle()
{
    Particle particle;

    // 基本的なTransformを初期化
    particle.transform.scale = { 1.0f, 1.0f, 1.0f };
    particle.transform.rotate = { 0.0f, 0.0f, 0.0f };

    // エミッションモジュールから位置を生成
    particle.transform.translate = emissionModule_->GenerateEmissionPosition(emitterTransform_.translate);

    // その他のモジュールを適用
    velocityModule_->ApplyInitialVelocity(particle);
    colorModule_->ApplyInitialColor(particle);
    lifetimeModule_->ApplyInitialLifetime(particle);
    sizeModule_->ApplyInitialSize(particle);
    rotationModule_->ApplyInitialRotation(particle);

    return particle;
}

void ParticleSystem::UpdateParticles(float deltaTime, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& billboardMatrix)
{
    instanceCount_ = 0;

    for (auto particleIterator = particles_.begin(); particleIterator != particles_.end();) {
        // ライフタイムチェック
        if (!lifetimeModule_->UpdateLifetime(*particleIterator, deltaTime)) {
            particleIterator = particles_.erase(particleIterator);
            continue;
        }

        // 力の適用
        forceModule_->ApplyForces(*particleIterator, deltaTime);

        // 速度の更新
        velocityModule_->UpdateVelocity(*particleIterator, deltaTime);

        // 位置の更新
        particleIterator->transform.translate.x += particleIterator->velocity.x * deltaTime;
        particleIterator->transform.translate.y += particleIterator->velocity.y * deltaTime;
        particleIterator->transform.translate.z += particleIterator->velocity.z * deltaTime;

        // 色の更新
        colorModule_->UpdateColor(*particleIterator);

        // サイズの更新（ライフタイムに応じた線形補間）
        sizeModule_->UpdateSize(*particleIterator);

        // 回転の更新
        rotationModule_->UpdateRotation(*particleIterator, deltaTime);

        // GPU用データの設定
        if (instanceCount_ < kNumMaxInstance) {
            Matrix4x4 worldMatrix = Matrix::MakeAffine(
                particleIterator->transform.scale,
                particleIterator->transform.rotate,
                particleIterator->transform.translate);

            // ビルボードに変換を適用
            worldMatrix = Matrix::Multiply(worldMatrix, billboardMatrix);

            // ワールド行列とビュー投影行列を掛け合わせてWVPを計算
            Matrix4x4 worldViewProjection = Matrix::Multiply(worldMatrix, viewProjectionMatrix);

            instancingData_[instanceCount_].WVP = worldViewProjection;
            instancingData_[instanceCount_].World = worldMatrix;
            instancingData_[instanceCount_].color = particleIterator->color;

            ++instanceCount_;
        }

        ++particleIterator;
    }
}

Matrix4x4 ParticleSystem::CreateBillboardMatrix(const Matrix4x4& viewMatrix)
{
    switch (billboardType_) {
        case BillboardType::ViewFacing:
        {
            // 従来の方式：カメラに完全に向く
            Matrix4x4 billboardMatrix = Matrix::Inverse(viewMatrix);
            billboardMatrix.m[3][0] = 0.0f;
            billboardMatrix.m[3][1] = 0.0f;
            billboardMatrix.m[3][2] = 0.0f;
            return billboardMatrix;
        }
        
        case BillboardType::YAxisOnly:
        {
            // Y軸のみ固定：Y軸（上方向）を維持しながらカメラに向く
            Matrix4x4 billboardMatrix = Matrix::Identity();
            
            // ビュー行列の逆行列を取得してカメラの位置を取得
            Matrix4x4 invView = Matrix::Inverse(viewMatrix);
            Vector3 cameraPos = { invView.m[3][0], invView.m[3][1], invView.m[3][2] };
            
            // XZ平面でのカメラ方向ベクトルを計算（Y成分を0にして水平方向のみ）
            Vector3 horizontalDirection = { cameraPos.x, 0.0f, cameraPos.z };
            
            // 水平方向ベクトルの長さを計算
            float horizontalLength = sqrt(horizontalDirection.x * horizontalDirection.x + 
                                         horizontalDirection.z * horizontalDirection.z);
            
            Vector3 forward, right;
            
            if (horizontalLength < 0.0001f) {
                // カメラが真上または真下にある場合のフォールバック
                // デフォルトの方向を使用
                forward = { 0.0f, 0.0f, 1.0f };
                right = { 1.0f, 0.0f, 0.0f };
            } else {
                // 正規化して前方向ベクトルを計算
                forward = { 
                    horizontalDirection.x / horizontalLength, 
                    0.0f, 
                    horizontalDirection.z / horizontalLength 
                };
                
                // 右方向ベクトル = Y軸 × 前方向
                // Cross({0,1,0}, forward) = {-forward.z, 0, forward.x}
                right = { -forward.z, 0.0f, forward.x };
            }
            
            // Y軸は常にワールドのY軸（上方向）
            Vector3 up = { 0.0f, 1.0f, 0.0f };
            
            // ビルボード行列を構築
            billboardMatrix.m[0][0] = right.x;
            billboardMatrix.m[0][1] = right.y;
            billboardMatrix.m[0][2] = right.z;
            billboardMatrix.m[1][0] = up.x;
            billboardMatrix.m[1][1] = up.y;
            billboardMatrix.m[1][2] = up.z;
            billboardMatrix.m[2][0] = forward.x;
            billboardMatrix.m[2][1] = forward.y;
            billboardMatrix.m[2][2] = forward.z;
            
            return billboardMatrix;
        }
        
        case BillboardType::ScreenAligned:
        {
            // スクリーン平行：カメラの向きに関係なくスクリーンと平行
            Matrix4x4 billboardMatrix = Matrix::Identity();
            
            // カメラの右方向と上方向ベクトルを取得
            Matrix4x4 invView = Matrix::Inverse(viewMatrix);
            Vector3 right = { invView.m[0][0], invView.m[0][1], invView.m[0][2] };
            Vector3 up = { invView.m[1][0], invView.m[1][1], invView.m[1][2] };
            Vector3 forward = { invView.m[2][0], invView.m[2][1], invView.m[2][2] };
            
            billboardMatrix.m[0][0] = right.x;
            billboardMatrix.m[0][1] = right.y;
            billboardMatrix.m[0][2] = right.z;
            billboardMatrix.m[1][0] = up.x;
            billboardMatrix.m[1][1] = up.y;
            billboardMatrix.m[1][2] = up.z;
            billboardMatrix.m[2][0] = forward.x;
            billboardMatrix.m[2][1] = forward.y;
            billboardMatrix.m[2][2] = forward.z;
            
            return billboardMatrix;
        }
        
        case BillboardType::None:
        default:
            return Matrix::Identity();
    }
}

void ParticleSystem::ShowImGui()
{
#ifdef _DEBUG
    ImGui::Begin("Particle System Debug");

    // システム状態表示
    ImGui::Text("=== パーティクルシステム ===");
    
    // パーティクル数とシステム状態
    uint32_t currentCount = GetParticleCount();
    float usageRatio = static_cast<float>(currentCount) / static_cast<float>(kNumMaxInstance);
    
    ImGui::Text("状態: %s | パーティクル数: %u/%u (%.0f%%)", 
        IsPlaying() ? "動作中" : "停止中", 
        currentCount, 
        kNumMaxInstance, 
        usageRatio * 100.0f);
    
    // 使用率の警告表示
    if (usageRatio > 0.8f) {
        ImGui::SameLine();
        if (usageRatio > 0.95f) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "[危険]");
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[警告]");
        }
    }
    
    // 基本制御ボタン
    if (ImGui::Button("再生")) { Play(); }
    ImGui::SameLine();
    if (ImGui::Button("停止")) { Stop(); }
    ImGui::SameLine();
    if (ImGui::Button("クリア")) { Clear(); }
    
    ImGui::Separator();

    // プリセット管理
    presetManager_->ShowImGui(this);

    // エミッター設定
    if (ImGui::CollapsingHeader("エミッター設定")) {
        ImGui::DragFloat3("位置", &emitterTransform_.translate.x, 0.01f);
        
        // ビルボードタイプ
        static const char* billboardTypeNames[] = {
            "なし", "カメラ向き", "Y軸固定", "スクリーン平行"
        };
        int currentBillboardType = static_cast<int>(billboardType_);
        if (ImGui::Combo("ビルボードタイプ", &currentBillboardType, billboardTypeNames, IM_ARRAYSIZE(billboardTypeNames))) {
            billboardType_ = static_cast<BillboardType>(currentBillboardType);
        }
        
        // ブレンドモード
        static const char* blendModeNames[] = {
            "なし", "通常", "加算", "減算", "乗算", "スクリーン"
        };
        int currentBlendMode = static_cast<int>(blendMode_);
        if (ImGui::Combo("ブレンドモード", &currentBlendMode, blendModeNames, IM_ARRAYSIZE(blendModeNames))) {
            blendMode_ = static_cast<BlendMode>(currentBlendMode);
        }
    }


    // 各モジュールのImGui表示
    if (ImGui::CollapsingHeader("放出モジュール")) {
        emissionModule_->ShowImGui();
    }

    if (ImGui::CollapsingHeader("速度モジュール")) {
        velocityModule_->ShowImGui();
    }

    if (ImGui::CollapsingHeader("色モジュール")) {
        colorModule_->ShowImGui();
    }

    if (ImGui::CollapsingHeader("寿命モジュール")) {
        lifetimeModule_->ShowImGui();
    }

    if (ImGui::CollapsingHeader("力場モジュール")) {
        forceModule_->ShowImGui();
    }

    if (ImGui::CollapsingHeader("サイズモジュール")) {
        sizeModule_->ShowImGui();
    }

    if (ImGui::CollapsingHeader("回転モジュール")) {
        rotationModule_->ShowImGui();
    }


    // 統計情報表示
    if (ImGui::CollapsingHeader("統計情報")) {
        ImGui::Text("作成されたパーティクル数: %u", statistics_.totalParticlesCreated);
        ImGui::Text("破棄されたパーティクル数: %u", statistics_.totalParticlesDestroyed);
        ImGui::Text("最大同時パーティクル数: %u", statistics_.peakParticleCount);
        ImGui::Text("平均ライフタイム: %.2f秒", statistics_.averageLifetime);
        ImGui::Text("システム稼働時間: %.2f秒", statistics_.systemRuntime);
        
        if (ImGui::Button("統計リセット")) {
            ResetStatistics();
        }
    }

    ImGui::End();
#endif
}

// リソースの作成関数
void ParticleSystem::ResourceCreate()
{
    // インスタンシング用のリソースを作成
    instancingResource_ = resourceFactory_->CreateBufferResource(
        dxCommon_->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance);
    instancingResource_->Map(0, nullptr, reinterpret_cast<void**>(&instancingData_));

    // 頂点データを定義
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
    vertexBuffer_ = resourceFactory_->CreateBufferResource(dxCommon_->GetDevice(), vertexBufferSize);

    // 頂点バッファにデータをコピー
    VertexData* vertexData = nullptr;
    vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, particleVertices.data(), vertexBufferSize);

    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(vertexBufferSize);
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

// SRVの作成関数
void ParticleSystem::CreateSRV()
{
    // インスタンシング用のSRVを設定
    D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
    instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    instancingSrvDesc.Buffer.FirstElement = 0;
    instancingSrvDesc.Buffer.NumElements = kNumMaxInstance;
    instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

    // DescriptorManager経由でSRVを作成
    dxCommon_->GetDescriptorManager()->CreateSRV(
    instancingResource_.Get(),
      instancingSrvDesc,
        instancingSrvHandleCPU_,
        instancingSrvHandleGPU_,
 "ParticleInstancingSRV"
    );
}

void ParticleSystem::CreateRootSignature()
{
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
    rootSignatureMg_->Create(dxCommon_->GetDevice());
}

void ParticleSystem::CreatePSO()
{
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
        .BuildAllBlendModes(dxCommon_->GetDevice(), vertexShaderBlob, pixelShaderBlob, rootSignatureMg_->GetRootSignature());

    if (!result) {
		throw std::runtime_error("Failed to create PSO in ParticleSystem");

    }
}
