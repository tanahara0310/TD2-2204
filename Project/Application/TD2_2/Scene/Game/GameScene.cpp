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
#include "Application/TD2_2/Utility/GameUtils.h"

void GameScene::Initialize(EngineSystem* engine) {
   BaseScene::Initialize(engine);

   GameUtils::Initialize(engine_);

   auto modelManager = engine_->GetComponent<ModelManager>();
   auto& textureManager = TextureManager::GetInstance();

   auto playerModel = modelManager->CreateStaticModel("Resources/Models/Player/Player.obj");
   auto playerTexture = textureManager.Load("Resources/SampleResources/white1x1.png");
   auto player = std::make_unique<Player>();
   player->Initialize(std::move(playerModel), playerTexture);
   player_ = player.get();
   gameObjects_.push_back(std::move(player));
}

void GameScene::Update() {
   BaseScene::Update();
}

void GameScene::Draw() {
   BaseScene::Draw();
}

void GameScene::Finalize() {}
