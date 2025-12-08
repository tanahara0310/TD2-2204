#pragma once

#include "CameraController.h"
#include "CinematicSequence.h"
#include "Application/TD2_2/GameObject/GameObject.h"
#include <memory>

/// @brief 動的カメラアニメーション生成ヘルパー
class DynamicCameraHelper {
public:
    /// @brief 現在位置からオブジェクトに近づくシーケンスを作成
    /// @param currentCameraPos 現在のカメラ位置
    /// @param currentCameraRot 現在のカメラ回転
    /// @param target 近づく対象オブジェクト
    /// @param approachDistance 対象からの距離
    /// @param duration 継続時間
    /// @param easingType イージングタイプ
    /// @return 作成されたシーケンス
    static std::shared_ptr<CinematicSequence> CreateApproachSequence(
        const Vector3& currentCameraPos,
        const Vector3& currentCameraRot,
        GameObject* target,
        float approachDistance = 10.0f,
        float duration = 2.0f,
        const std::string& easingType = "EaseInOutQuad");

    /// @brief 2つのオブジェクトを順番に映すシーケンスを作成
    /// @param currentCameraPos 現在のカメラ位置
    /// @param currentCameraRot 現在のカメラ回転
    /// @param object1 最初に映すオブジェクト
    /// @param object2 次に映すオブジェクト
    /// @param approachDistance 各オブジェクトからの距離
    /// @param durationPerCut 各カットの継続時間
    /// @return 作成されたシーケンス
    static std::shared_ptr<CinematicSequence> CreateTwoObjectSequence(
        const Vector3& currentCameraPos,
        const Vector3& currentCameraRot,
        GameObject* object1,
        GameObject* object2,
        float approachDistance = 10.0f,
        float durationPerCut = 2.0f);

    /// @brief オブジェクトの周りを回りながら近づくシーケンスを作成
    /// @param currentCameraPos 現在のカメラ位置
    /// @param target 対象オブジェクト
    /// @param finalDistance 最終的な距離
    /// @param orbitAngle 回転角度（ラジアン、0でそのまま接近）
    /// @param duration 継続時間
    /// @return 作成されたシーケンス
    static std::shared_ptr<CinematicSequence> CreateOrbitApproachSequence(
        const Vector3& currentCameraPos,
        GameObject* target,
        float finalDistance = 8.0f,
        float orbitAngle = 1.57f,  // 90度
        float duration = 3.0f);

    /// @brief オブジェクトに向かって急接近するシーケンスを作成
    /// @param currentCameraPos 現在のカメラ位置
    /// @param target 対象オブジェクト
    /// @param closeDistance 最終的な距離
    /// @param duration 継続時間
    /// @return 作成されたシーケンス
    static std::shared_ptr<CinematicSequence> CreateRushApproachSequence(
        const Vector3& currentCameraPos,
        GameObject* target,
        float closeDistance = 5.0f,
        float duration = 1.0f);

private:
    /// @brief オブジェクトの前方位置を計算
    /// @param target 対象オブジェクト
    /// @param distance オブジェクトからの距離
    /// @param heightOffset 高さオフセット
    /// @return カメラ位置
    static Vector3 CalculateCameraPositionInFrontOf(
        GameObject* target,
        float distance,
        float heightOffset = 2.0f);

    /// @brief 2点間の中間位置を計算
    /// @param start 開始位置
    /// @param end 終了位置
    /// @param t 補間値（0.0～1.0）
    /// @return 中間位置
    static Vector3 Lerp(const Vector3& start, const Vector3& end, float t);
};
