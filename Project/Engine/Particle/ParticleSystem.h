#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <vector>
#include <list>
#include <string>

#ifdef _DEBUG
#include "Engine/Utility/Debug/ImGui/ImguiManager.h"
#endif

// Math
#include "MathCore.h"

// Graphics関連
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Resource/ResourceFactory.h"
#include "Engine/Graphics/Material/MaterialManager.h"
#include "Engine/Graphics/PipelineStateManager.h"
#include "Engine/Graphics/RootSignatureManager.h"
#include "Engine/Graphics/Shader/ShaderCompiler.h"
#include "Engine/Graphics/Structs/VertexData.h"

// モジュール関連
#include "Modules/EmissionModule.h"
#include "Modules/VelocityModule.h"
#include "Modules/ColorModule.h"
#include "Modules/LifetimeModule.h"
#include "Modules/ForceModule.h"
#include "Modules/SizeModule.h"
#include "Modules/RotationModule.h"

// プリセット管理
#include "ParticlePresetManager.h"

using namespace MathCore;

// ビルボードタイプ
enum class BillboardType {
    None,           // ビルボード無効
    ViewFacing,     // カメラに向く
    YAxisOnly,      // Y軸のみ固定
    ScreenAligned   // スクリーン平行
};

// パーティクルのパラメータ
struct Particle {
    EulerTransform transform;
    Vector3 velocity;
    Vector4 color;
    float lifeTime;
    float currentTime;
    Vector3 rotationSpeed = {0.0f, 0.0f, 0.0f};
};

// GPU送信用パーティクルデータ
struct ParticleForGPU {
    Matrix4x4 WVP;
    Matrix4x4 World;
    Vector4 color;
};

/// @brief パーティクルシステムクラス
class ParticleSystem {
public:
    static constexpr uint32_t kNumMaxInstance = 4096; // パーティクルの最大数

    ParticleSystem() = default;
    ~ParticleSystem() = default;

    /// @brief 初期化
    /// @param dxCommon DirectXCommon
    /// @param resourceFactory リソースファクトリ
    void Initialize(DirectXCommon* dxCommon, ResourceFactory* resourceFactory);

    /// @brief 更新処理
    /// @param viewMatrix ビュー行列
    /// @param projectionMatrix プロジェクション行列
    void Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix);

    /// @brief 描画
    /// @param commandList コマンドリスト
    /// @param textureHandle テクスチャハンドル
    void Draw(ID3D12GraphicsCommandList* commandList, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);

    // ──────────────────────────────────────────────────────────
    // パーティクルシステム制御
    // ──────────────────────────────────────────────────────────

    /// @brief 再生開始
    void Play();

    /// @brief 再生停止
    void Stop();

    /// @brief 再生中かどうか
    /// @return 再生中の場合true
    bool IsPlaying() const;

    /// @brief 全パーティクルクリア
    void Clear();

    // ──────────────────────────────────────────────────────────
    // 設定アクセッサ
    // ──────────────────────────────────────────────────────────

    /// @brief エミッター位置を設定
    /// @param position 位置
    void SetEmitterPosition(const Vector3& position) { emitterTransform_.translate = position; }

    /// @brief エミッター位置を取得
    /// @return 位置
    Vector3 GetEmitterPosition() const { return emitterTransform_.translate; }

    /// @brief ビルボードタイプを設定
    /// @param type ビルボードタイプ
    void SetBillboardType(BillboardType type) { billboardType_ = type; }

    /// @brief ビルボードタイプを取得
    /// @return ビルボードタイプ
    BillboardType GetBillboardType() const { return billboardType_; }

    /// @brief ブレンドモードを設定
    /// @param mode ブレンドモード
    void SetBlendMode(BlendMode mode) { blendMode_ = mode; }

    /// @brief ブレンドモードを取得
    /// @return ブレンドモード
    BlendMode GetBlendMode() const { return blendMode_; }

    // ──────────────────────────────────────────────────────────
    // モジュールアクセッサ
    // ──────────────────────────────────────────────────────────

    /// @brief エミッションモジュールを取得
    /// @return エミッションモジュールの参照
    EmissionModule& GetEmissionModule() { return *emissionModule_; }

    /// @brief 速度モジュールを取得
    /// @return 速度モジュールの参照
    VelocityModule& GetVelocityModule() { return *velocityModule_; }

    /// @brief 色モジュールを取得
    /// @return 色モジュールの参照
    ColorModule& GetColorModule() { return *colorModule_; }

    /// @brief ライフタイムモジュールを取得
    /// @return ライフタイムモジュールの参照
    LifetimeModule& GetLifetimeModule() { return *lifetimeModule_; }

    /// @brief 力場モジュールを取得
    /// @return 力場モジュールの参照
    ForceModule& GetForceModule() { return *forceModule_; }

    /// @brief サイズモジュールを取得
    /// @return サイズモジュールの参照
    SizeModule& GetSizeModule() { return *sizeModule_; }

    /// @brief 回転モジュールを取得
    /// @return 回転モジュールの参照
    RotationModule& GetRotationModule() { return *rotationModule_; }

    // ──────────────────────────────────────────────────────────
    // 統計情報
    // ──────────────────────────────────────────────────────────

    /// @brief 現在のパーティクル数を取得
    /// @return パーティクル数
    uint32_t GetParticleCount() const { return static_cast<uint32_t>(particles_.size()); }

    /// @brief 最大パーティクル数を取得
    /// @return 最大パーティクル数
    uint32_t GetMaxParticleCount() const { return kNumMaxInstance; }

    struct Statistics {
        uint32_t totalParticlesCreated = 0;      // 作成された総パーティクル数
        uint32_t totalParticlesDestroyed = 0;    // 破棄された総パーティクル数
        uint32_t peakParticleCount = 0;          // 最大同時パーティクル数
        float averageLifetime = 0.0f;            // 平均ライフタイム
        float systemRuntime = 0.0f;              // システム稼働時間
    };
    
    /// @brief 統計情報を取得
    /// @return 統計情報の参照
    const Statistics& GetStatistics() const { return statistics_; }

    /// @brief 統計情報をリセット
    void ResetStatistics() { 
        statistics_ = Statistics(); 
        statistics_.systemRuntime = 0.0f;
    }

    /// @brief ImGuiデバッグ表示
    void ShowImGui();

private:
    // ──────────────────────────────────────────────────────────
    // パーティクルシステムのコア
    // ──────────────────────────────────────────────────────────

    // DirectXCommonへのポインタ
    DirectXCommon* dxCommon_ = nullptr;
    ResourceFactory* resourceFactory_ = nullptr;

    // パーティクルデータ
    std::list<Particle> particles_;
    uint32_t instanceCount_ = 0;

    // エミッター設定
    EulerTransform emitterTransform_;
    BillboardType billboardType_ = BillboardType::ViewFacing;
    BlendMode blendMode_ = BlendMode::kBlendModeAdd;

    // 統計情報
    Statistics statistics_;
    float deltaTimeAccumulator_ = 0.0f; // 時間の累積用

    // ──────────────────────────────────────────────────────────
    // モジュール
    // ──────────────────────────────────────────────────────────

    std::unique_ptr<EmissionModule> emissionModule_;
    std::unique_ptr<VelocityModule> velocityModule_;
    std::unique_ptr<ColorModule> colorModule_;
    std::unique_ptr<LifetimeModule> lifetimeModule_;
    std::unique_ptr<ForceModule> forceModule_;
    std::unique_ptr<SizeModule> sizeModule_;
    std::unique_ptr<RotationModule> rotationModule_;

    // プリセット管理
    std::unique_ptr<ParticlePresetManager> presetManager_ = std::make_unique<ParticlePresetManager>();

    // ──────────────────────────────────────────────────────────
    // GPU関連リソース
    // ──────────────────────────────────────────────────────────

    // インスタンシング
    Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
    D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_ = {};
    D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_ = {};
    ParticleForGPU* instancingData_ = nullptr;

    // 頂点データ
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};

    // マテリアル
    std::unique_ptr<MaterialManager> materialManager_ = std::make_unique<MaterialManager>();

    // パイプライン
    std::unique_ptr<PipelineStateManager> pipelineMg_ = std::make_unique<PipelineStateManager>();
    std::unique_ptr<RootSignatureManager> rootSignatureMg_ = std::make_unique<RootSignatureManager>();
    std::unique_ptr<ShaderCompiler> shaderCompiler_ = std::make_unique<ShaderCompiler>();

    // ──────────────────────────────────────────────────────────
    // 内部処理
    // ──────────────────────────────────────────────────────────

    /// @brief パーティクルの放出
    /// @param count 放出数
    void EmitParticles(uint32_t count);

    /// @brief 新しいパーティクルを作成
    /// @return 作成されたパーティクル
    Particle CreateNewParticle();

    /// @brief パーティクルの更新
    /// @param deltaTime フレーム時間
    /// @param viewProjectionMatrix ビュープロジェクション行列
    /// @param billboardMatrix ビルボード行列
    void UpdateParticles(float deltaTime, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& billboardMatrix);

    /// @brief ビルボード行列の作成
    /// @param viewMatrix ビュー行列
    /// @return ビルボード行列
    Matrix4x4 CreateBillboardMatrix(const Matrix4x4& viewMatrix);

    /// @brief リソースの作成
    void ResourceCreate();

    /// @brief SRVの作成
    void CreateSRV();

    /// @brief ルートシグネチャの作成
    void CreateRootSignature();

    /// @brief パイプラインステートオブジェクトの作成
    void CreatePSO();
};
