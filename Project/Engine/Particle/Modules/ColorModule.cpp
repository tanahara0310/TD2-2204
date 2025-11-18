#include "ColorModule.h"
#include "../ParticleSystem.h" // Particle構造体のために必要
#include <algorithm>

void ColorModule::ApplyInitialColor(Particle& particle) {
    if (!enabled_) {
        particle.color = { 1.0f, 1.0f, 1.0f, 1.0f };
        return;
    }

    Vector4 color = colorData_.startColor;

    // グラデーション使用時はランダム色範囲を適用しない
    if (!colorData_.useGradient) {
        // ランダム色の範囲を適用
        color.x += random_.GetFloatSigned() * colorData_.randomColorRange.x;
        color.y += random_.GetFloatSigned() * colorData_.randomColorRange.y;
        color.z += random_.GetFloatSigned() * colorData_.randomColorRange.z;
        color.w += random_.GetFloatSigned() * colorData_.randomColorRange.w;

        // 色の値を0-1の範囲に制限
        color.x = (std::max)(0.0f, (std::min)(1.0f, color.x));
        color.y = (std::max)(0.0f, (std::min)(1.0f, color.y));
        color.z = (std::max)(0.0f, (std::min)(1.0f, color.z));
        color.w = (std::max)(0.0f, (std::min)(1.0f, color.w));
    }

    particle.color = color;
}

void ColorModule::UpdateColor(Particle& particle) {
    if (!enabled_ || !colorData_.useGradient) {
        return;
    }

    // ライフタイムに基づいて色を補間
    float t = particle.currentTime / particle.lifeTime;
    t = (std::max)(0.0f, (std::min)(1.0f, t)); // 0-1の範囲に制限

    particle.color = LerpColor(colorData_.startColor, colorData_.endColor, t);
}

#ifdef _DEBUG
bool ColorModule::ShowImGui() {
    bool changed = false;

    // 有効/無効の切り替え
    if (ImGui::Checkbox("有効##色", &enabled_)) {
        changed = true;
    }

    if (!enabled_) {
        ImGui::BeginDisabled();
    }

    changed |= ImGui::ColorEdit4("開始色", &colorData_.startColor.x);
    changed |= ImGui::ColorEdit4("終了色", &colorData_.endColor.x);
    changed |= ImGui::Checkbox("グラデーション使用", &colorData_.useGradient);
    changed |= ImGui::ColorEdit4("ランダム色範囲", &colorData_.randomColorRange.x);

    if (!enabled_) {
        ImGui::EndDisabled();
    }

    return changed;
}
#endif

Vector4 ColorModule::LerpColor(const Vector4& color1, const Vector4& color2, float t) {
    return {
        color1.x + (color2.x - color1.x) * t,
        color1.y + (color2.y - color1.y) * t,
        color1.z + (color2.z - color1.z) * t,
        color1.w + (color2.w - color1.w) * t
    };
}