#include "FenceObject.h"
#include <EngineSystem.h>

void FenceObject::Initialize() {
   auto engine = GetEngineSystem();
   // 必須コンポーネントの取得
   auto dxCommon = engine->GetComponent<DirectXCommon>();
   auto modelManager = engine->GetComponent<ModelManager>();

   if (!dxCommon || !modelManager) {
	  return;
   }

   // 静的モデルとして作成
   model_ = modelManager->CreateStaticModel("Resources/SampleResources/fence/fence.obj");

   // トランスフォームの初期化
   transform_.Initialize(dxCommon->GetDevice());

   // テクスチャの読み込み
   auto& textureManager = TextureManager::GetInstance();
   texture_ = textureManager.Load("Resources/SampleResources/fence/fence.png");
}
