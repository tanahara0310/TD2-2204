#include "Sphere.h"
#include <EngineSystem.h>

void Sphere::Initialize()
{
   auto engine = GetEngineSystem();

   // 必須コンポーネントの取得
   auto dxCommon = engine->GetComponent<DirectXCommon>();
   auto modelManager = engine->GetComponent<ModelManager>();
   auto& textureManager = TextureManager::GetInstance();

   {
	  model_ = modelManager->CreateStaticModel("Resources/sphere.obj");
	  transform_.Initialize(dxCommon->GetDevice());
	  texture_ = textureManager.Load("Resources/SampleResources/monsterBall.png");

   }
}

void Sphere::Update()
{}

void Sphere::Draw(const ICamera* camera)
{
   if (!model_ || !camera) {
	  return;
   }

   // モデルの描画
   model_->Draw(transform_, camera, texture_.gpuHandle);
}
