#pragma once

#include "../PostEffectBase.h"

class GrayScale : public PostEffectBase {
public:
    /// @brief ImGuiでパラメータを調整
    void DrawImGui() override;

protected:
	const std::wstring& GetPixelShaderPath() const override
    {
        static const std::wstring pixelShaderPath = L"Resources/Shader/PostProcess/GrayScale.PS.hlsl";
        return pixelShaderPath;
    }
};
