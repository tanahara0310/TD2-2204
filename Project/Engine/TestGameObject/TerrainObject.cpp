#include "TerrainObject.h"
#include "Engine/EngineSystem/EngineSystem.h"
#include "Engine/Graphics/Model/ModelManager.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/TextureManager.h"
#include <numbers>

void TerrainObject::Initialize() {
   auto engine = GetEngineSystem();
   // 必須コンポーネントの取得
   auto dxCommon = engine->GetComponent<DirectXCommon>();
   auto modelManager = engine->GetComponent<ModelManager>();

   if (!dxCommon || !modelManager) {
	  return;
   }

   // 静的モデルとして作成
   model_ = modelManager->CreateStaticModel("Resources/SampleResources/terrain/terrain.obj");

   // トランスフォームの初期化
   transform_.Initialize(dxCommon->GetDevice());

   // テレインの初期回転
   transform_.rotate = { 0.0f, std::numbers::pi_v<float> *0.5f, 0.0f };

   // テクスチャの読み込み
   auto& textureManager = TextureManager::GetInstance();
   texture_ = textureManager.Load("Resources/SampleResources/terrain/grass.png");
}
