#include "DynamicCameraHelper.h"
#include "MathCore.h"
#include <cmath>

using namespace MathCore;

std::shared_ptr<CinematicSequence> DynamicCameraHelper::CreateApproachSequence(
    const Vector3& currentCameraPos,
    const Vector3& currentCameraRot,
    GameObject* target,
    float approachDistance,
    float duration,
    const std::string& easingType)
{
    if (!target) {
        return nullptr;
    }

    auto sequence = std::make_shared<CinematicSequence>();

    // 対象オブジェクトの位置を取得
    Vector3 targetPos = target->GetWorldPosition();

    // カットを作成
    CinematicCut cut;
    cut.name = "Approach to Target";
    cut.duration = duration;
    cut.config.type = CameraController::CinematicType::Dolly;
    cut.config.duration = duration;
    cut.config.startPosition = currentCameraPos;
    cut.config.startRotation = currentCameraRot;
    
    // 終了位置：対象の前方、指定距離の位置
    cut.config.endPosition = CalculateCameraPositionInFrontOf(target, approachDistance);
    
    // 終了回転：対象を注視する角度を計算
    Vector3 toTarget = {
        targetPos.x - cut.config.endPosition.x,
        targetPos.y - cut.config.endPosition.y,
        targetPos.z - cut.config.endPosition.z
    };
    toTarget = Vector::Normalize(toTarget);
    
    float yaw = std::atan2(toTarget.x, toTarget.z);
    float pitch = std::asin(-toTarget.y);
    cut.config.endRotation = { pitch, yaw, 0.0f };
    
    cut.config.useEasing = true;
    cut.config.easingType = easingType;

    sequence->AddCut(cut);
    return sequence;
}

std::shared_ptr<CinematicSequence> DynamicCameraHelper::CreateTwoObjectSequence(
    const Vector3& currentCameraPos,
    const Vector3& currentCameraRot,
    GameObject* object1,
    GameObject* object2,
    float approachDistance,
    float durationPerCut)
{
    if (!object1 || !object2) {
        return nullptr;
    }

    auto sequence = std::make_shared<CinematicSequence>();

    // カット1: 現在位置から object1 へ接近
    {
        Vector3 obj1Pos = object1->GetWorldPosition();
        
        CinematicCut cut1;
        cut1.name = "Approach to Object 1";
        cut1.duration = durationPerCut;
        cut1.config.type = CameraController::CinematicType::Dolly;
        cut1.config.duration = durationPerCut;
        cut1.config.startPosition = currentCameraPos;
        cut1.config.startRotation = currentCameraRot;
        cut1.config.endPosition = CalculateCameraPositionInFrontOf(object1, approachDistance);
        
        // object1を注視する回転
        Vector3 toObj1 = {
            obj1Pos.x - cut1.config.endPosition.x,
            obj1Pos.y - cut1.config.endPosition.y,
            obj1Pos.z - cut1.config.endPosition.z
        };
        toObj1 = Vector::Normalize(toObj1);
        float yaw1 = std::atan2(toObj1.x, toObj1.z);
        float pitch1 = std::asin(-toObj1.y);
        cut1.config.endRotation = { pitch1, yaw1, 0.0f };
        
        cut1.config.useEasing = true;
        cut1.config.easingType = "EaseInOutQuad";
        
        sequence->AddCut(cut1);
    }

    // カット2: object1 から object2 へ移動
    {
        Vector3 obj2Pos = object2->GetWorldPosition();
        
        CinematicCut cut2;
        cut2.name = "Move to Object 2";
        cut2.duration = durationPerCut;
        cut2.config.type = CameraController::CinematicType::Dolly;
        cut2.config.duration = durationPerCut;
        cut2.config.startPosition = CalculateCameraPositionInFrontOf(object1, approachDistance);
        cut2.config.endPosition = CalculateCameraPositionInFrontOf(object2, approachDistance);
        
        // 開始回転：object1を注視
        Vector3 obj1Pos = object1->GetWorldPosition();
        Vector3 toObj1 = {
            obj1Pos.x - cut2.config.startPosition.x,
            obj1Pos.y - cut2.config.startPosition.y,
            obj1Pos.z - cut2.config.startPosition.z
        };
        toObj1 = Vector::Normalize(toObj1);
        float yaw1 = std::atan2(toObj1.x, toObj1.z);
        float pitch1 = std::asin(-toObj1.y);
        cut2.config.startRotation = { pitch1, yaw1, 0.0f };
        
        // 終了回転：object2を注視
        Vector3 toObj2 = {
            obj2Pos.x - cut2.config.endPosition.x,
            obj2Pos.y - cut2.config.endPosition.y,
            obj2Pos.z - cut2.config.endPosition.z
        };
        toObj2 = Vector::Normalize(toObj2);
        float yaw2 = std::atan2(toObj2.x, toObj2.z);
        float pitch2 = std::asin(-toObj2.y);
        cut2.config.endRotation = { pitch2, yaw2, 0.0f };
        
        cut2.config.useEasing = true;
        cut2.config.easingType = "EaseInOutQuad";
        
        sequence->AddCut(cut2);
    }

    return sequence;
}

std::shared_ptr<CinematicSequence> DynamicCameraHelper::CreateOrbitApproachSequence(
    const Vector3& currentCameraPos,
    GameObject* target,
    float finalDistance,
    float orbitAngle,
    float duration)
{
    if (!target) {
        return nullptr;
    }

    auto sequence = std::make_shared<CinematicSequence>();
    Vector3 targetPos = target->GetWorldPosition();

    // カット1: 円弧を描きながら接近
    CinematicCut cut;
    cut.name = "Orbit Approach";
    cut.duration = duration;
    cut.config.type = CameraController::CinematicType::Arc;
    cut.config.duration = duration;
    cut.config.startPosition = currentCameraPos;
    
    // 終了位置を計算（対象の周りを回った位置）
    Vector3 currentToTarget = {
        targetPos.x - currentCameraPos.x,
        targetPos.y - currentCameraPos.y,
        targetPos.z - currentCameraPos.z
    };
    float currentDistance = Vector::Length(currentToTarget);
    currentToTarget = Vector::Normalize(currentToTarget);
    
    // 現在の角度を計算
    float currentYaw = std::atan2(-currentToTarget.x, -currentToTarget.z);
    
    // 目標角度を計算（orbitAngle分回転）
    float targetYaw = currentYaw + orbitAngle;
    
    // 終了位置
    cut.config.endPosition = {
        targetPos.x - std::sin(targetYaw) * finalDistance,
        targetPos.y + 2.0f,
        targetPos.z - std::cos(targetYaw) * finalDistance
    };
    
    // 円弧の高さを設定
    cut.config.orbitRadius = (currentDistance - finalDistance) * 0.3f;
    
    // 回転を計算
    Vector3 startDir = Vector::Normalize(currentToTarget);
    float startYaw = std::atan2(startDir.x, startDir.z);
    float startPitch = std::asin(-startDir.y);
    cut.config.startRotation = { startPitch, startYaw, 0.0f };
    
    Vector3 endToTarget = {
        targetPos.x - cut.config.endPosition.x,
        targetPos.y - cut.config.endPosition.y,
        targetPos.z - cut.config.endPosition.z
    };
    endToTarget = Vector::Normalize(endToTarget);
    float endYaw = std::atan2(endToTarget.x, endToTarget.z);
    float endPitch = std::asin(-endToTarget.y);
    cut.config.endRotation = { endPitch, endYaw, 0.0f };
    
    cut.config.useEasing = true;
    cut.config.easingType = "EaseInOutCubic";
    
    sequence->AddCut(cut);
    return sequence;
}

std::shared_ptr<CinematicSequence> DynamicCameraHelper::CreateRushApproachSequence(
    const Vector3& currentCameraPos,
    GameObject* target,
    float closeDistance,
    float duration)
{
    if (!target) {
        return nullptr;
    }

    auto sequence = std::make_shared<CinematicSequence>();
    Vector3 targetPos = target->GetWorldPosition();

    CinematicCut cut;
    cut.name = "Rush Approach";
    cut.duration = duration;
    cut.config.type = CameraController::CinematicType::Dolly;
    cut.config.duration = duration;
    cut.config.startPosition = currentCameraPos;
    cut.config.endPosition = CalculateCameraPositionInFrontOf(target, closeDistance, 1.0f);
    
    // 開始回転
    Vector3 startToTarget = {
        targetPos.x - currentCameraPos.x,
        targetPos.y - currentCameraPos.y,
        targetPos.z - currentCameraPos.z
    };
    startToTarget = Vector::Normalize(startToTarget);
    float startYaw = std::atan2(startToTarget.x, startToTarget.z);
    float startPitch = std::asin(-startToTarget.y);
    cut.config.startRotation = { startPitch, startYaw, 0.0f };
    
    // 終了回転
    Vector3 endToTarget = {
        targetPos.x - cut.config.endPosition.x,
        targetPos.y - cut.config.endPosition.y,
        targetPos.z - cut.config.endPosition.z
    };
    endToTarget = Vector::Normalize(endToTarget);
    float endYaw = std::atan2(endToTarget.x, endToTarget.z);
    float endPitch = std::asin(-endToTarget.y);
    cut.config.endRotation = { endPitch, endYaw, 0.0f };
    
    cut.config.useEasing = true;
    cut.config.easingType = "EaseInCubic";  // 加速しながら接近
    
    sequence->AddCut(cut);
    return sequence;
}

Vector3 DynamicCameraHelper::CalculateCameraPositionInFrontOf(
    GameObject* target,
    float distance,
    float heightOffset)
{
    if (!target) {
        return { 0.0f, 0.0f, 0.0f };
    }

    Vector3 targetPos = target->GetWorldPosition();
    
    // オブジェクトの前方にカメラを配置
    // 簡易的に、オブジェクトの後方（-Z方向）にカメラを配置
    return {
        targetPos.x,
        targetPos.y + heightOffset,
        targetPos.z - distance
    };
}

Vector3 DynamicCameraHelper::Lerp(const Vector3& start, const Vector3& end, float t)
{
    return {
        start.x + (end.x - start.x) * t,
        start.y + (end.y - start.y) * t,
        start.z + (end.z - start.z) * t
    };
}
