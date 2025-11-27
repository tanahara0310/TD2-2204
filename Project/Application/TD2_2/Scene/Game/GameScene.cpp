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
   // 基底クラスの初期化
   BaseScene::Initialize(engine);

   // ゲームユーティリティの初期化
   GameUtils::Initialize(engine_);

   // ゲームオブジェクトの初期化
   auto modelManager = engine_->GetComponent<ModelManager>();
   auto& textureManager = TextureManager::GetInstance();

   // プレイヤーの生成と初期化
   {
	  auto playerModel = modelManager->CreateStaticModel("Resources/Models/Player/Player.obj");
	  auto playerTexture = textureManager.Load("Resources/Textures/Player.png");
	  auto player = std::make_unique<Player>();
	  player->Initialize(std::move(playerModel), playerTexture);
	  player_ = player.get();
	  gameObjects_.push_back(std::move(player));
   }

   // ボスの生成と初期化
   {
	  auto bossModel = modelManager->CreateStaticModel("Resources/Models/Boss/Boss.obj");
	  auto bossTexture = textureManager.Load("Resources/Textures/Boss.png");
	  auto boss = std::make_unique<Boss>();
	  boss->Initialize(std::move(bossModel), bossTexture);
	  boss_ = boss.get();
	  gameObjects_.push_back(std::move(boss));
   }

   // 衝突設定の初期化
   {
	  collisionConfig_ = std::make_unique<CollisionConfig>();
	  collisionConfig_->SetCollisionEnabled(CollisionLayer::Player, CollisionLayer::Boss, true);
	  collisionConfig_->SetCollisionEnabled(CollisionLayer::Player, CollisionLayer::BossBullet, true);
	  collisionConfig_->SetCollisionEnabled(CollisionLayer::Boss, CollisionLayer::BossBullet, false);
	  collisionManager_ = std::make_unique<CollisionManager>(collisionConfig_.get());
   }

   // カメラコントローラーの初期化（プレイヤーとボスを追跡）
   {
	  cameraController_ = std::make_unique<CameraController>();
	  auto* releaseCamera = static_cast<Camera*>(cameraManager_->GetCamera("Release"));
	  cameraController_->Initialize(releaseCamera, player_, boss_);
	  
	  // カメラパラメータの調整（オプション）
	  cameraController_->SetMinDistance(20.0f);
	  cameraController_->SetMaxDistance(100.0f);
	  cameraController_->SetDistanceScale(1.8f);
	  cameraController_->SetHeightOffset(0.0f);
	  cameraController_->SetPitchAngle(0.0f);
	  cameraController_->SetSmoothSpeed(50.0f);
	  cameraController_->SetMarginDistance(8.0f);
   }
}

void GameScene::Update() {
   BaseScene::Update();

   // カメラコントローラーの更新
   if (cameraController_) {
	  cameraController_->Update();
   }

#ifdef _DEBUG
   // カメラコントローラーのデバッグUI
   if (cameraController_) {
	  cameraController_->DrawImGui();
   }
#endif

   // コライダー登録
   RegisterAllColliders();

   // 衝突判定
   CheckCollisions();
}

void GameScene::Draw() {
   BaseScene::Draw();
}

void GameScene::Finalize() {}

void GameScene::RegisterAllColliders(){
   collisionManager_->Clear();
   collisionManager_->RegisterCollider(player_->GetCollider());
   collisionManager_->RegisterCollider(boss_->GetCollider());
}

void GameScene::CheckCollisions(){
   collisionManager_->CheckAllCollisions();
}
