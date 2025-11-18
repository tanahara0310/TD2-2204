#pragma once

#include "ParticleModule.h"
#include "MathCore.h"
#include "Engine/Utility/Collision/CollisionUtils.h"

struct Particle;

/// @brief パーティクルの力場モジュール
class ForceModule : public ParticleModule {
public:
    struct ForceData {
        Vector3 gravity = { 0.0f, -9.8f, 0.0f };    // 重力
        Vector3 wind = { 0.0f, 0.0f, 0.0f };        // 風力
        float drag = 0.0f;                          // 抵抗力 (0.0f - 1.0f)
        
        // 加速度フィールド
        Vector3 acceleration = { 0.0f, 0.0f, 0.0f };
        BoundingBox area = { {-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f} };
        bool useAccelerationField = false;
    };

    ForceModule() = default;
    ~ForceModule() = default;

    /// @brief 力データを設定
    /// @param data 力データ
    void SetForceData(const ForceData& data) { forceData_ = data; }

    /// @brief 力データを取得
    /// @return 力データの参照
    const ForceData& GetForceData() const { return forceData_; }

    /// @brief パーティクルに力を適用
    /// @param particle 対象のパーティクル
    /// @param deltaTime フレーム時間
    void ApplyForces(Particle& particle, float deltaTime);

#ifdef _DEBUG
    /// @brief ImGuiデバッグ表示
    /// @return UIに変更があった場合true
    bool ShowImGui() override;
#endif

private:
    ForceData forceData_;
};