#pragma once

#include "ParticleModule.h"
#include "MathCore.h"

struct Particle;

/// @brief パーティクルの速度モジュール
class VelocityModule : public ParticleModule {
public:
    struct VelocityData {
        Vector3 startSpeed = { 0.0f, 1.0f, 0.0f };  // 初期速度
        float startSpeedMultiplier = 1.0f;           // 速度の倍率
        Vector3 randomSpeedRange = { 1.0f, 1.0f, 1.0f }; // ランダム速度の範囲
        bool useRandomDirection = true;              // ランダム方向を使用するか
    };

    VelocityModule() = default;
    ~VelocityModule() = default;

    /// @brief 速度データを設定
    /// @param data 速度データ
    void SetVelocityData(const VelocityData& data) { velocityData_ = data; }

    /// @brief 速度データを取得
    /// @return 速度データの参照
    const VelocityData& GetVelocityData() const { return velocityData_; }

    /// @brief パーティクルに初期速度を適用
    /// @param particle 対象のパーティクル
    void ApplyInitialVelocity(Particle& particle);

    /// @brief パーティクルの速度を更新
    /// @param particle 対象のパーティクル
    /// @param deltaTime フレーム時間
    void UpdateVelocity(Particle& particle, float deltaTime);

#ifdef _DEBUG
    /// @brief ImGuiデバッグ表示
    /// @return UIに変更があった場合true
    bool ShowImGui() override;
#endif

private:
    VelocityData velocityData_;

    /// @brief ランダムな方向ベクトルを生成
    /// @return ランダムな方向ベクトル
    Vector3 GenerateRandomDirection();
};