#pragma once

/// @brief 描画パスタイプ
enum class RenderPassType {
    Invalid = -1,        // 無効
    Model = 0,           // 通常モデル
    SkinnedModel,        // スキニングモデル
    SkyBox,              // SkyBox
    Sprite,              // スプライト
};
