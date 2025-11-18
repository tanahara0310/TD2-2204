#include "EmissionModule.h"
#include <cmath>

using namespace MathCore;

uint32_t EmissionModule::CalculateEmissionCount(float deltaTime) {
    if (!enabled_ || !isPlaying_) {
        return 0;
    }

    uint32_t emissionCount = 0;

    // 持続時間チェック
    if (!emissionData_.loop && currentTime_ >= emissionData_.duration) {
        return 0;
    }

    // バースト放出
    if (!hasEmittedBurst_ && currentTime_ >= emissionData_.burstTime) {
        emissionCount += emissionData_.burstCount;
        hasEmittedBurst_ = true;
    }

    // 時間当たりの放出
    if (emissionData_.rateOverTime > 0) {
        emissionAccumulator_ += static_cast<float>(emissionData_.rateOverTime) * deltaTime;
        uint32_t particlesToEmit = static_cast<uint32_t>(emissionAccumulator_);
        emissionAccumulator_ -= static_cast<float>(particlesToEmit);
        emissionCount += particlesToEmit;
    }

    return emissionCount;
}

void EmissionModule::UpdateTime(float deltaTime) {
    if (isPlaying_) {
        currentTime_ += deltaTime;
        
        // ループ処理
        if (emissionData_.loop && currentTime_ >= emissionData_.duration) {
            currentTime_ = 0.0f;
            hasEmittedBurst_ = false;
        }
    }
}

void EmissionModule::Play() {
    isPlaying_ = true;
    currentTime_ = 0.0f;
    emissionAccumulator_ = 0.0f;
    hasEmittedBurst_ = false;
}

void EmissionModule::Stop() {
    isPlaying_ = false;
}

Vector3 EmissionModule::GenerateEmissionPosition(const Vector3& emitterPosition) {
    if (!enabled_) {
        return emitterPosition;
    }

    switch (emissionData_.shapeType) {
        case ShapeType::Box:
            return GenerateBoxPosition(emitterPosition);
        case ShapeType::Sphere:
            return GenerateSpherePosition(emitterPosition);
        case ShapeType::Circle:
            return GenerateCirclePosition(emitterPosition);
        case ShapeType::CircleHarf:
            return GenerateCircleHarfPosition(emitterPosition);
        case ShapeType::Cone:
            return GenerateConePosition(emitterPosition);
        case ShapeType::Hemisphere:
            return GenerateHemispherePosition(emitterPosition);
        case ShapeType::Ring:
            return GenerateRingPosition(emitterPosition);
        case ShapeType::Line:
            return GenerateLinePosition(emitterPosition);
        case ShapeType::Cylinder:
            return GenerateCylinderPosition(emitterPosition);
        case ShapeType::Edge:
            return GenerateEdgePosition(emitterPosition);
        case ShapeType::Point:
        default:
            return GeneratePointPosition(emitterPosition);
    }
}

#ifdef _DEBUG
bool EmissionModule::ShowImGui() {
    bool changed = false;
    
    // 有効/無効の切り替え
    if (ImGui::Checkbox("有効##放出", &enabled_)) {
        changed = true;
    }

    if (!enabled_) {
        ImGui::BeginDisabled();
    }

    // 基本的な放出設定
    ImGui::Separator();
    ImGui::Text("基本放出設定");
    changed |= ImGui::DragInt("時間あたりの放出数", reinterpret_cast<int*>(&emissionData_.rateOverTime), 1, 0, 100);
    changed |= ImGui::DragInt("バースト放出数", reinterpret_cast<int*>(&emissionData_.burstCount), 1, 0, 50);
    changed |= ImGui::DragFloat("バーストタイミング", &emissionData_.burstTime, 0.1f, 0.0f, 10.0f);
    changed |= ImGui::DragFloat("持続時間", &emissionData_.duration, 0.1f, 0.1f, 60.0f);
    changed |= ImGui::Checkbox("ループ", &emissionData_.loop);

    // エミッター形状設定
    ImGui::Separator();
    ImGui::Text("エミッター形状");
    
    // 形状タイプの選択
    static const char* shapeTypeNames[] = {
        "点", "ボックス", "球体", "円", "コーン", "半球", "リング", "ライン", "円柱", "エッジ", "半円"
    };
    int currentShapeType = static_cast<int>(emissionData_.shapeType);
    if (ImGui::Combo("形状タイプ", &currentShapeType, shapeTypeNames, IM_ARRAYSIZE(shapeTypeNames))) {
        emissionData_.shapeType = static_cast<ShapeType>(currentShapeType);
        changed = true;
    }

    // 選択された形状に応じてパラメータを動的に表示
    switch (emissionData_.shapeType) {
        case ShapeType::Point:
            // 点形状：ランダム範囲のみ
            ImGui::Text("点形状パラメータ:");
            changed |= ImGui::DragFloat("ランダム位置範囲", &emissionData_.randomPositionRange, 0.01f, 0.0f, 5.0f);
            ImGui::TextDisabled("パーティクルは1点から放出され、オプションでランダム散布します");
            break;

        case ShapeType::Box:
            // ボックス形状：3Dスケール + 表面放出オプション
            ImGui::Text("ボックス形状パラメータ:");
            changed |= ImGui::DragFloat3("ボックスサイズ", &emissionData_.scale.x, 0.1f, 0.1f, 20.0f);
            changed |= ImGui::Checkbox("表面のみから放出", &emissionData_.emitFromSurface);
            ImGui::TextDisabled("パーティクルは矩形の内部または表面から放出されます");
            break;

        case ShapeType::Sphere:
            // 球体形状：半径 + 表面放出オプション
            ImGui::Text("球体形状パラメータ:");
            changed |= ImGui::DragFloat("半径", &emissionData_.radius, 0.1f, 0.1f, 20.0f);
            changed |= ImGui::Checkbox("表面のみから放出", &emissionData_.emitFromSurface);
            ImGui::TextDisabled("パーティクルは球体の内部または表面から放出されます");
            break;

        case ShapeType::Circle:
            // 円形状：半径
            ImGui::Text("円形状パラメータ:");
            changed |= ImGui::DragFloat("半径", &emissionData_.radius, 0.1f, 0.1f, 20.0f);
            ImGui::TextDisabled("パーティクルはXZ平面上の円形エリア内から放出されます");
            break;

        case ShapeType::Cone:
            // コーン形状：角度と高さ
            ImGui::Text("コーン形状パラメータ:");
            changed |= ImGui::DragFloat("コーン角度", &emissionData_.angle, 1.0f, 0.0f, 90.0f, "%.1f度");
            changed |= ImGui::DragFloat("高さ", &emissionData_.height, 0.1f, 0.1f, 20.0f);
            ImGui::TextDisabled("パーティクルはY軸に沿ったコーン形状で放出されます");
            break;

        case ShapeType::Hemisphere:
            // 半球形状：半径
            ImGui::Text("半球形状パラメータ:");
            changed |= ImGui::DragFloat("半径", &emissionData_.radius, 0.1f, 0.1f, 20.0f);
            changed |= ImGui::Checkbox("表面のみから放出", &emissionData_.emitFromSurface);
            ImGui::TextDisabled("パーティクルは球体の上半分から放出されます");
            break;

        case ShapeType::Ring:
            // リング形状：内径と外径
            ImGui::Text("リング形状パラメータ:");
            changed |= ImGui::DragFloat("外径", &emissionData_.radius, 0.1f, 0.1f, 20.0f);
            changed |= ImGui::DragFloat("内径", &emissionData_.innerRadius, 0.1f, 0.0f, emissionData_.radius);
            ImGui::TextDisabled("パーティクルはXZ平面上のリング状エリアから放出されます");
            break;

        case ShapeType::Line:
            // 線形状：長さと方向
            ImGui::Text("線形状パラメータ:");
            changed |= ImGui::DragFloat("長さ", &emissionData_.scale.x, 0.1f, 0.1f, 20.0f);
            changed |= ImGui::DragFloat3("方向", &emissionData_.emissionDirection.x, 0.1f);
            ImGui::TextDisabled("パーティクルは指定方向のライン上から放出されます");
            break;

        case ShapeType::Cylinder:
            // 円柱形状：半径と高さ
            ImGui::Text("円柱形状パラメータ:");
            changed |= ImGui::DragFloat("半径", &emissionData_.radius, 0.1f, 0.1f, 20.0f);
            changed |= ImGui::DragFloat("高さ", &emissionData_.height, 0.1f, 0.1f, 20.0f);
            changed |= ImGui::Checkbox("表面のみから放出", &emissionData_.emitFromSurface);
            ImGui::TextDisabled("パーティクルは円柱の内部または表面から放出されます");
            break;

        case ShapeType::Edge:
            // エッジ形状：半径のみ（表面のみ）
            ImGui::Text("エッジ形状パラメータ:");
            changed |= ImGui::DragFloat("半径", &emissionData_.radius, 0.1f, 0.1f, 20.0f);
            ImGui::TextDisabled("パーティクルは球体の表面のみから放出されます");
            break;
            
        case ShapeType::CircleHarf:
            // 半円形状：半径
            ImGui::Text("半円形状パラメータ:");
            changed |= ImGui::DragFloat("半径", &emissionData_.radius, 0.1f, 0.1f, 20.0f);
            ImGui::TextDisabled("パーティクルは上下の扇形エリアから放出されます");
            break;
    }

    // 追加の共通パラメータ
    ImGui::Separator();
    ImGui::Text("追加パラメータ:");
    
    // 一部の形状では共通のランダム位置範囲も使用可能
    if (emissionData_.shapeType != ShapeType::Point) {
        changed |= ImGui::DragFloat("追加ランダム範囲", &emissionData_.randomPositionRange, 0.01f, 0.0f, 2.0f);
        ImGui::TextDisabled("すべての形状に適用される追加のランダム散布");
    }

    if (!enabled_) {
        ImGui::EndDisabled();
    }

    return changed;
}
#endif

Vector3 EmissionModule::GeneratePointPosition(const Vector3& emitterPosition) {
    Vector3 position = emitterPosition;
    
    // ランダム位置範囲を適用
    if (emissionData_.randomPositionRange > 0.0f) {
        position.x += random_.GetFloat(-emissionData_.randomPositionRange, emissionData_.randomPositionRange);
        position.y += random_.GetFloat(-emissionData_.randomPositionRange, emissionData_.randomPositionRange);
        position.z += random_.GetFloat(-emissionData_.randomPositionRange, emissionData_.randomPositionRange);
    }
    
    return position;
}

Vector3 EmissionModule::GenerateBoxPosition(const Vector3& emitterPosition) {
    Vector3 randomOffset = {
        random_.GetFloatSigned() * emissionData_.scale.x,
        random_.GetFloatSigned() * emissionData_.scale.y,
        random_.GetFloatSigned() * emissionData_.scale.z
    };

    return {
        emitterPosition.x + randomOffset.x,
        emitterPosition.y + randomOffset.y,
        emitterPosition.z + randomOffset.z
    };
}

Vector3 EmissionModule::GenerateSpherePosition(const Vector3& emitterPosition) {
    // 球面上の均等分布のためのランダム点生成
    Vector3 direction = {
        random_.GetFloatSigned(),
        random_.GetFloatSigned(),
        random_.GetFloatSigned()
    };
    
    // 正規化
    float length = Vector::Length(direction);
    if (length > 0.0f) {
        direction.x /= length;
        direction.y /= length;
        direction.z /= length;
    }
    
    // 半径内の均等分布
    float r = std::cbrt(random_.GetFloat()) * emissionData_.radius;
    
    return {
        emitterPosition.x + direction.x * r,
        emitterPosition.y + direction.y * r,
        emitterPosition.z + direction.z * r
    };
}

Vector3 EmissionModule::GenerateCirclePosition(const Vector3& emitterPosition) {
    float angle = random_.GetFloat(0.0f, 2.0f * 3.14159f);

    // --- 半径に揺らぎを加える ---
    float baseRadius = emissionData_.radius;

    // 波打ちパラメータ
    float waveFreq = 12.0f;       // ギザギザの数
    float waveAmp = 0.2f;        // 揺らぎの大きさ
    float noise = std::sin(angle * waveFreq) * waveAmp;

    // 最終的な半径
    float r = baseRadius + noise;

    // XY 平面上のリング座標
    return {
        emitterPosition.x + std::cos(angle) * r, // X
        emitterPosition.y + std::sin(angle) * r, // Y
        emitterPosition.z                        // Zは固定
    };
}

Vector3 EmissionModule::GenerateConePosition(const Vector3& emitterPosition) {
    float circleAngle = random_.GetFloat(0.0f, 2.0f * 3.14159f);
    float height = random_.GetFloat() * emissionData_.height;
    float coneRadius = height * std::tan(emissionData_.angle * 3.14159f / 180.0f);
    float r = std::sqrt(random_.GetFloat()) * coneRadius;
    
    return {
        emitterPosition.x + std::cos(circleAngle) * r,
        emitterPosition.y + height,
        emitterPosition.z + std::sin(circleAngle) * r
    };
}

Vector3 EmissionModule::GenerateHemispherePosition(const Vector3& emitterPosition) {
    // 半球面上の均等分布のためのランダム点生成
    Vector3 direction;
    do {
        direction = {
            random_.GetFloatSigned(),
            std::abs(random_.GetFloatSigned()), // Y成分は常に正（上半球）
            random_.GetFloatSigned()
        };
    } while (Vector::Length(direction) > 1.0f);
    
    // 正規化
    float length = Vector::Length(direction);
    if (length > 0.0f) {
        direction.x /= length;
        direction.y /= length;
        direction.z /= length;
    }
    
    // 半径内の均等分布
    float r = std::cbrt(random_.GetFloat()) * emissionData_.radius;
    
    return {
        emitterPosition.x + direction.x * r,
        emitterPosition.y + direction.y * r,
        emitterPosition.z + direction.z * r
    };
}

Vector3 EmissionModule::GenerateRingPosition(const Vector3& emitterPosition) {
    float angle = random_.GetFloat(0.0f, 2.0f * 3.14159f);
    // リング形状：内径と外径の間で均等分布
    float minR = emissionData_.innerRadius;
    float maxR = emissionData_.radius;
    float r = minR + std::sqrt(random_.GetFloat()) * (maxR - minR);
    
    return {
        emitterPosition.x + std::cos(angle) * r,
        emitterPosition.y,
        emitterPosition.z + std::sin(angle) * r
    };
}

Vector3 EmissionModule::GenerateLinePosition(const Vector3& emitterPosition) {
    // 線分は指定された方向に沿って生成
    Vector3 direction = emissionData_.emissionDirection;
    float length = Vector::Length(direction);
    if (length > 0.0f) {
        direction.x /= length;
        direction.y /= length;
        direction.z /= length;
    } else {
        direction = {0.0f, 1.0f, 0.0f}; // デフォルトは上方向
    }
    
    float linePosition = random_.GetFloatSigned() * emissionData_.scale.x; // scaleのX成分を線の長さとして使用
    
    return {
        emitterPosition.x + direction.x * linePosition,
        emitterPosition.y + direction.y * linePosition,
        emitterPosition.z + direction.z * linePosition
    };
}

Vector3 EmissionModule::GenerateCylinderPosition(const Vector3& emitterPosition) {
    float angle = random_.GetFloat(0.0f, 2.0f * 3.14159f);
    float height = random_.GetFloatSigned() * (emissionData_.height * 0.5f);
    
    float r;
    if (emissionData_.emitFromSurface) {
        // 表面からのみ放出（円柱の側面と上下面）
        if (random_.GetBool(0.8f)) {
            // 側面
            r = emissionData_.radius;
        } else {
            // 上下面
            r = std::sqrt(random_.GetFloat()) * emissionData_.radius;
            height = (height > 0) ? (emissionData_.height * 0.5f) : (-emissionData_.height * 0.5f);
        }
    } else {
        // 円柱内部全体
        r = std::sqrt(random_.GetFloat()) * emissionData_.radius;
    }
    
    return {
        emitterPosition.x + std::cos(angle) * r,
        emitterPosition.y + height,
        emitterPosition.z + std::sin(angle) * r
    };
}

Vector3 EmissionModule::GenerateEdgePosition(const Vector3& emitterPosition) {
    // 球体の表面上のランダム点を生成
    Vector3 direction = {
        random_.GetFloatSigned(),
        random_.GetFloatSigned(),
        random_.GetFloatSigned()
    };
    
    // 正規化して表面上の点にする
    float length = Vector::Length(direction);
    if (length > 0.0f) {
        direction.x /= length;
        direction.y /= length;
        direction.z /= length;
    }
    
    // 球体の表面上に配置
    return {
        emitterPosition.x + direction.x * emissionData_.radius,
        emitterPosition.y + direction.y * emissionData_.radius,
        emitterPosition.z + direction.z * emissionData_.radius
    };
}

Vector3 EmissionModule::GenerateCircleHarfPosition(const Vector3& emitterPosition) {
    constexpr float PI = 3.14159265358979323846f;

    // --- 上下どちらかをランダムに選ぶ ---
    bool top = random_.GetBool();

    float minAngle, maxAngle;
    if (top) {
        // 上側の扇形（150°～210°）
        minAngle = 5.0f * PI / 6.0f;   // 150°
        maxAngle = 7.0f * PI / 6.0f;   // 210°
    }
    else {
        // 下側の扇形（330°～30°）
        // → [330°, 360°) と [0°, 30°] の2区間に分ける必要あり
        if (random_.GetBool()) {
            minAngle = 11.0f * PI / 6.0f; // 330°
            maxAngle = 2.0f * PI;         // 360°
        }
        else {
            minAngle = 0.0f;              // 0°
            maxAngle = PI / 6.0f;         // 30°
        }
    }

    float angle = random_.GetFloat(minAngle, maxAngle);

    // --- 半径に揺らぎを加える ---
    float baseRadius = emissionData_.radius;
    float waveFreq = 12.0f;       // ギザギザの数
    float waveAmp = 0.2f;        // 揺らぎの大きさ
    float noise = std::sin(angle * waveFreq) * waveAmp;

    float r = baseRadius + noise;

    // XY 平面上のリング座標
    return {
        emitterPosition.x + std::cos(angle) * r, // X
        emitterPosition.y + std::sin(angle) * r, // Y
        emitterPosition.z                        // Zは固定
    };
}
