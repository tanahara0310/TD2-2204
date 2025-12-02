#include "TitleUI.h"
#include "EngineSystem/EngineSystem.h"
#include "ObjectCommon/IDrawable.h"

std::vector<std::unique_ptr<IDrawable>> TitleUI::Initialize(EngineSystem* engine) {
    (void)engine;

    std::vector<std::unique_ptr<IDrawable>> sprites;

    // タイトルロゴを作成
    auto titleLogo = CreateTitleLogo();
    titleLogo_ = titleLogo.get();
    sprites.push_back(std::move(titleLogo));

    return sprites;
}

void TitleUI::Update() {
    if (titleLogo_) {
        titleLogo_->Update();
    }
}

std::unique_ptr<SpriteObject> TitleUI::CreateTitleLogo() {
    auto sprite = std::make_unique<SpriteObject>();
    sprite->Initialize("Resources/GameResources/Title/Title.png");
    // 2Dカメラは画面中央が原点(0,0)なので、中央に配置するには(0,0)を指定
    sprite->GetTransform().translate = {0.0f, 0.0f, 0.0f};
    sprite->SetAnchor({0.5f, 0.5f});
    return sprite;
}
