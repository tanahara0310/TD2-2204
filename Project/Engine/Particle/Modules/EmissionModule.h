#pragma once

#include "ParticleModule.h"
#include "MathCore.h"
/// @brief パーティクル放出モジュール
class EmissionModule : public ParticleModule {
public:
    enum class ShapeType {
        Point,          // 点
        Box,            // ボックス
        Sphere,         // 球体
        Circle,         // 円（XZ平面）
        Cone,           // コーン
        Hemisphere,     // 半球
        Ring,           // リング（ドーナツ型）
        Line,           // 線分
        Cylinder,       // 円柱
        Edge,            // エッジ（球体の表面のみ）
        CircleHarf
    };

    struct EmissionData {
        uint32_t rateOverTime = 10;     // 時間当たりの放出数
        uint32_t burstCount = 0;        // バースト放出数
        float burstTime = 0.0f;         // バースト発生時間
        float duration = 5.0f;          // 持続時間
        bool loop = true;               // ループするかどうか
        
        // エミッター形状関連
        ShapeType shapeType = ShapeType::Point;
        Vector3 scale = { 1.0f, 1.0f, 1.0f };      // 形状のスケール（Box, Line等）
        float radius = 1.0f;                       // 球体/円/リングの半径
        float innerRadius = 0.5f;                  // リングの内径
        float height = 2.0f;                       // 円柱/コーンの高さ
        float angle = 25.0f;                       // コーンの角度
        float randomPositionRange = 0.0f;          // 位置のランダム範囲
        
        // 放出方向制御
        bool emitFromSurface = false;              // 表面からのみ放出（球体/ボックス等）
        Vector3 emissionDirection = {0.0f, 1.0f, 0.0f}; // 優先放出方向（Line, Edge等）
    };

    EmissionModule() = default;
    ~EmissionModule() = default;

    /// @brief 放出データを設定
    /// @param data 放出データ
    void SetEmissionData(const EmissionData& data) { emissionData_ = data; }

    /// @brief 放出データを取得
    /// @return 放出データの参照
    const EmissionData& GetEmissionData() const { return emissionData_; }

    /// @brief この時間で放出すべきパーティクル数を計算
    /// @param deltaTime フレーム時間
    /// @return 放出すべきパーティクル数
    uint32_t CalculateEmissionCount(float deltaTime);

    /// @brief モジュールの時間を更新
    /// @param deltaTime フレーム時間
    void UpdateTime(float deltaTime);

    /// @brief 再生を開始
    void Play();

    /// @brief 停止
    void Stop();

    /// @brief 再生中かどうか
    /// @return 再生中の場合true
    bool IsPlaying() const { return isPlaying_; }

    /// @brief パーティクルの初期位置を生成
    /// @param emitterPosition エミッターの位置
    /// @return 生成された位置
    Vector3 GenerateEmissionPosition(const Vector3& emitterPosition);

#ifdef _DEBUG
    /// @brief ImGuiデバッグ表示
    /// @return UIに変更があった場合true
    bool ShowImGui() override;
#endif

private:
    EmissionData emissionData_;
    float currentTime_ = 0.0f;          // 現在の時間
    float emissionAccumulator_ = 0.0f;   // 放出用アキュムレータ
    bool isPlaying_ = false;
    bool hasEmittedBurst_ = false;       // バーストを放出したかどうか

    // 形状別の位置生成関数
    Vector3 GeneratePointPosition(const Vector3& emitterPosition);
    Vector3 GenerateBoxPosition(const Vector3& emitterPosition);
    Vector3 GenerateSpherePosition(const Vector3& emitterPosition);
    Vector3 GenerateCirclePosition(const Vector3& emitterPosition);
    Vector3 GenerateConePosition(const Vector3& emitterPosition);
    Vector3 GenerateHemispherePosition(const Vector3& emitterPosition);
    Vector3 GenerateRingPosition(const Vector3& emitterPosition);
    Vector3 GenerateLinePosition(const Vector3& emitterPosition);
    Vector3 GenerateCylinderPosition(const Vector3& emitterPosition);
    Vector3 GenerateEdgePosition(const Vector3& emitterPosition);
    Vector3 GenerateCircleHarfPosition(const Vector3& emitterPosition);
};