#include "SizeModule.h"
#include "../ParticleSystem.h"
#include <algorithm>

void SizeModule::ApplyInitialSize(Particle& particle)
{
    if (!enabled_) {
        return;
    }

    if (sizeData_.use3DSize) {
        // 3Dサイズを使用
        Vector3 initialSize = sizeData_.startSize3D;
        
        // ランダム性を適用
        if (sizeData_.sizeRandomness > 0.0f) {
            initialSize.x = ApplyRandomness(initialSize.x, sizeData_.sizeRandomness);
            if (sizeData_.uniformScaling) {
                // 均等スケーリングの場合、X軸のランダム性を他の軸にも適用
                float randomFactor = initialSize.x / sizeData_.startSize3D.x;
                initialSize.y = sizeData_.startSize3D.y * randomFactor;
                initialSize.z = sizeData_.startSize3D.z * randomFactor;
            } else {
                // 非均等スケーリングの場合、各軸に個別にランダム性を適用
                initialSize.y = ApplyRandomness(initialSize.y, sizeData_.sizeRandomness);
                initialSize.z = ApplyRandomness(initialSize.z, sizeData_.sizeRandomness);
            }
        }
        
        // サイズ制限を適用
        initialSize.x = std::clamp(initialSize.x, sizeData_.minSize, sizeData_.maxSize);
        initialSize.y = std::clamp(initialSize.y, sizeData_.minSize, sizeData_.maxSize);
        initialSize.z = std::clamp(initialSize.z, sizeData_.minSize, sizeData_.maxSize);
        
        particle.transform.scale = initialSize;
    } else {
        // 1Dサイズを使用（均等スケーリング）
        float initialSize = ApplyRandomness(sizeData_.startSize, sizeData_.sizeRandomness);
        initialSize = std::clamp(initialSize, sizeData_.minSize, sizeData_.maxSize);
        particle.transform.scale = {initialSize, initialSize, initialSize};
    }
}

void SizeModule::UpdateSize(Particle& particle)
{
    if (!enabled_ || !sizeData_.sizeOverLifetime) {
        return;
    }

    // ライフタイム係数を取得
    float lifetimeRatio = GetLifetimeRatio(particle);
    
    // カーブを適用
    float curveValue = ApplyCurve(lifetimeRatio, sizeData_.sizeCurve);

    if (sizeData_.use3DSize) {
        // 3Dサイズでの補間
        Vector3 currentSize = LerpVector3(sizeData_.startSize3D, sizeData_.endSize3D, curveValue);
        
        // サイズ制限を適用
        currentSize.x = std::clamp(currentSize.x, sizeData_.minSize, sizeData_.maxSize);
        currentSize.y = std::clamp(currentSize.y, sizeData_.minSize, sizeData_.maxSize);
        currentSize.z = std::clamp(currentSize.z, sizeData_.minSize, sizeData_.maxSize);
        
        particle.transform.scale = currentSize;
    } else {
        // 1Dサイズでの補間（線形補間）
        float currentSize = sizeData_.startSize + (sizeData_.endSize - sizeData_.startSize) * curveValue;
        currentSize = std::clamp(currentSize, sizeData_.minSize, sizeData_.maxSize);
        particle.transform.scale = {currentSize, currentSize, currentSize};
    }
}

#ifdef _DEBUG
bool SizeModule::ShowImGui() {
    bool changed = false;
    
    // 有効/無効の切り替え
    if (ImGui::Checkbox("有効##サイズ", &enabled_)) {
        changed = true;
    }

    if (!enabled_) {
        ImGui::BeginDisabled();
    }

    // 3Dサイズ設定
    changed |= ImGui::Checkbox("3Dサイズ使用", &sizeData_.use3DSize);
    
    if (sizeData_.use3DSize) {
        changed |= ImGui::DragFloat3("開始サイズ3D", &sizeData_.startSize3D.x, 0.01f, 0.01f, 10.0f);
        changed |= ImGui::DragFloat3("終了サイズ3D", &sizeData_.endSize3D.x, 0.01f, 0.0f, 10.0f);
        changed |= ImGui::Checkbox("均等スケーリング", &sizeData_.uniformScaling);
    } else {
        changed |= ImGui::DragFloat("開始サイズ", &sizeData_.startSize, 0.01f, 0.01f, 10.0f);
        changed |= ImGui::DragFloat("終了サイズ", &sizeData_.endSize, 0.01f, 0.0f, 10.0f);
    }
    
    changed |= ImGui::Checkbox("寿命に応じたサイズ変化", &sizeData_.sizeOverLifetime);
    changed |= ImGui::DragFloat("サイズランダム性", &sizeData_.sizeRandomness, 0.01f, 0.0f, 1.0f);

    // サイズカーブ設定
    static const char* sizeCurveNames[] = {
        "線形", "イーズイン", "イーズアウト", "イーズインアウト", "一定"
    };
    int currentCurve = static_cast<int>(sizeData_.sizeCurve);
    if (ImGui::Combo("サイズカーブ", &currentCurve, sizeCurveNames, IM_ARRAYSIZE(sizeCurveNames))) {
        sizeData_.sizeCurve = static_cast<SizeData::SizeCurve>(currentCurve);
        changed = true;
    }

    // サイズ制限
    changed |= ImGui::DragFloat("最小サイズ", &sizeData_.minSize, 0.01f, 0.01f, 1.0f);
    changed |= ImGui::DragFloat("最大サイズ", &sizeData_.maxSize, 0.1f, 1.0f, 50.0f);

    if (!enabled_) {
        ImGui::EndDisabled();
    }

    return changed;
}
#endif

float SizeModule::GetLifetimeRatio(const Particle& particle)
{
    if (particle.lifeTime <= 0.0f) {
        return 1.0f; // ライフタイムが0以下の場合は終了扱い
    }
    
    float ratio = particle.currentTime / particle.lifeTime;
    return std::clamp(ratio, 0.0f, 1.0f);
}

float SizeModule::ApplyCurve(float t, SizeData::SizeCurve curve)
{
    switch (curve) {
        case SizeData::SizeCurve::Linear:
            return t;
            
        case SizeData::SizeCurve::EaseIn:
            return t * t; // 二次関数（加速）
            
        case SizeData::SizeCurve::EaseOut:
            return 1.0f - (1.0f - t) * (1.0f - t); // 減速
            
        case SizeData::SizeCurve::EaseInOut:
            if (t < 0.5f) {
                return 2.0f * t * t; // 前半は加速
            } else {
                return 1.0f - 2.0f * (1.0f - t) * (1.0f - t); // 後半は減速
            }
            
        case SizeData::SizeCurve::Constant:
            return 0.0f; // 変化なし（開始サイズを維持）
            
        default:
            return t;
    }
}

Vector3 SizeModule::LerpVector3(const Vector3& start, const Vector3& end, float t)
{
    return {
        start.x + (end.x - start.x) * t,
        start.y + (end.y - start.y) * t,
        start.z + (end.z - start.z) * t
    };
}

float SizeModule::ApplyRandomness(float baseSize, float randomness)
{
    if (randomness <= 0.0f) {
        return baseSize;
    }
    
    // ±randomness の範囲でランダム係数を生成
    float randomFactor = 1.0f + random_.GetFloat(-randomness, randomness);
    
    return baseSize * randomFactor;
}