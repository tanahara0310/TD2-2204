#include "SphereObject.h"
#include <EngineSystem.h>

void SphereObject::Initialize(EngineSystem* engine) {
    // 必須コンポーネントの取得
    auto dxCommon = engine->GetComponent<DirectXCommon>();
    auto modelManager = engine->GetComponent<ModelManager>();
    
    if (!dxCommon || !modelManager) {
        return;
    }

    // 静的モデルとして作成
    model_ = modelManager->CreateStaticModel("Resources/sphere.obj");

    // トランスフォームの初期化
    transform_.Initialize(dxCommon->GetDevice());

    // テクスチャの読み込み
    auto& textureManager = TextureManager::GetInstance();
    texture_ = textureManager.Load("Resources/SampleResources/monsterBall.png");
}
