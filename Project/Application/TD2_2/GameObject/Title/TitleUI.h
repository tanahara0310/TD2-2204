#pragma once
#include "Engine/ObjectCommon/SpriteObject.h"
#include <memory>
#include <vector>

class EngineSystem;
class IDrawable;

/// @brief タイトル画面のUI管理クラス
class TitleUI {
public:
    TitleUI() = default;
    ~TitleUI() = default;

    /// @brief 初期化（スプライトを作成してvectorで返す）
    /// @param engine エンジンシステム
    /// @return 作成したスプライトのunique_ptrのvector
    std::vector<std::unique_ptr<IDrawable>> Initialize(EngineSystem* engine);

    /// @brief 更新
    void Update();

private:
    /// @brief タイトルロゴを作成
    std::unique_ptr<SpriteObject> CreateTitleLogo();

    // UI要素のポインタ（所有権はgameObjects_が持つ）
    SpriteObject* titleLogo_ = nullptr;
};
