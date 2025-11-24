#include "GameScene.h"
#include "EngineSystem/EngineSystem.h"
#include "Scene/SceneManager.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "Engine/Camera/Debug/DebugCamera.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Render/RenderManager.h"
#include "Engine/Graphics/Light/LightManager.h"
#include "Engine/Graphics/Light/LightData.h"
#include "MathCore.h"

void GameScene::Initialize(EngineSystem* engine) {
   BaseScene::Initialize(engine);

  /* auto dxCommon = engine_->GetComponent<DirectXCommon>();
   auto modelManager = engine_->GetComponent<ModelManager>();
   auto& textureManager = TextureManager::GetInstance();

   auto playerModel = modelManager->CreateStaticModel("Resources/Models/Player/Player.obj");*/
   //player_ = std::make_unique<Player>();
}

void GameScene::Update() {
   BaseScene::Update();
}

void GameScene::Draw() {
   BaseScene::Draw();
}

void GameScene::Finalize() {}
