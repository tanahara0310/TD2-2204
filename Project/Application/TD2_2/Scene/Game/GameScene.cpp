#include "GameScene.h"
#include <numbers>
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
#include "../../GameObject/Boss/ActionNode/ChargeToPlayerAction.h"
#include "../../GameObject/Boss/ActionNode/MoveAction.h"
#include "../../GameObject/Boss/ActionNode/MoveToCenterAction.h"
#include "../../GameObject/Boss/ActionNode/FleeFromPlayerAction.h"
#include "../../GameObject/Boss/ActionNode/ShootEightWayAction.h"
#include "../../GameObject/Bullet/Bullet.h"
#include "../Config/GameSceneConfig.h"

void GameScene::Initialize(EngineSystem* engine) {
   // 基底クラスの初期化
   BaseScene::Initialize(engine);

   // ゲームユーティリティの初期化
   GameUtils::Initialize(engine_);

   // ゲームオブジェクトの初期化
   auto modelManager = engine_->GetComponent<ModelManager>();
   auto& textureManager = TextureManager::GetInstance();

   // 雷エフェクトマネージャーの初期化
   {
	  lightningManager_ = std::make_unique<LightningEffectManager>();
	  lightningManager_->Initialize(modelManager, &textureManager);
   }

   // プレイヤーの生成と初期化
   {
	  modelManager->LoadModelResource("Resources/Models/Player/Damage", "PlayerDamage.obj");
	  modelManager->LoadModelResource("Resources/Models/PlayerPropeller", "PlayerPropeller.obj");
	  auto playerModel = modelManager->CreateStaticModel("Resources/Models/Player/Player.obj");
	  auto playerTexture = textureManager.Load("Resources/Textures/Player.png");
	  auto player = std::make_unique<Player>();
	  player->Initialize(std::move(playerModel), playerTexture);
	  player->SetStartDamageFunction([this]() {
		 if (cameraController_) {
			// プリセット版は継続時間も事前設定されている
			cameraController_->StartShake(CameraController::ShakeIntensity::Large);
		 }
		 });
	  player->SetHitEnemyFunction([this]() {
		 if (cameraController_) {
			cameraController_->StartShake(CameraController::ShakeIntensity::Medium);
		 }
		 });
	  player->RegisterModelResource("Damage", "Resources/Models/Player/Damage/PlayerDamage.obj");
	  player->RegisterModelResource("Player1", "Resources/Models/Player/Player.obj");
	  player->RegisterModelResource("Player2", "Resources/Models/PlayerPropeller/PlayerPropeller.obj");
	  player_ = player.get();
	  gameObjects_.push_back(std::move(player));
   }

   // ボスの生成と初期化
   {
	  modelManager->LoadModelResource("Resources/Models/Boss/Damage", "BossDamage.obj");
	  modelManager->LoadModelResource("Resources/Models/BossPropeller", "BossPropeller.obj");
	  auto bossModel = modelManager->CreateStaticModel("Resources/Models/Boss/Boss.obj");
	  auto bossTexture = textureManager.Load("Resources/Textures/Boss.png");
	  auto boss = std::make_unique<Boss>();
	  boss_ = boss.get();
	  bossBehaviorTree_ = CreateBossBehaviorTree();
	  boss->Initialize(std::move(bossModel), bossTexture);
	  boss->SetBehaviorTree(std::move(bossBehaviorTree_));
	  boss->RegisterModelResource("Damage", "Resources/Models/Boss/Damage/BossDamage.obj");
	  boss->RegisterModelResource("Boss1", "Resources/Models/Boss/Boss.obj");
	  boss->RegisterModelResource("Boss2", "Resources/Models/BossPropeller/BossPropeller.obj");
	  gameObjects_.push_back(std::move(boss));
   }

   // 背景の生成と初期化
   {
	  auto backgroundModel = modelManager->CreateStaticModel("Resources/Models/Background/Background.obj");
	  auto backgroundTexture = textureManager.Load("Resources/Textures/Background.png");
	  auto background = std::make_unique<Background>();
	  background_ = background.get();
	  background->Initialize(std::move(backgroundModel), backgroundTexture);
	  gameObjects_.push_back(std::move(background));
   }

   // HPUIの初期化
   {
	  playerHitPointUI_ = std::make_unique<HitPoint>();
	  auto sprites = playerHitPointUI_->Initialize({ -590.0f, 300.0f }, SettingObject::PLAYER, player_->GetMaxHP());

	  // スプライトをgameObjects_に追加
	  for (auto& sprite : sprites) {
		 gameObjects_.push_back(std::move(sprite));
	  }

	  bossHitPointUI_ = std::make_unique<HitPoint>();
	  auto bossSprites = bossHitPointUI_->Initialize({ 450.0f, 300.0f }, SettingObject::BOSS, boss_->GetMaxHP());
	  // スプライトをgameObjects_に追加
	  for (auto& sprite : bossSprites) {
		 gameObjects_.push_back(std::move(sprite));
	  }
   }

   // フレームの初期化
   InitializeFrames();


   // 衝突設定の初期化
   {
	  collisionConfig_ = std::make_unique<CollisionConfig>();
	  collisionConfig_->SetCollisionEnabled(CollisionLayer::Player, CollisionLayer::Boss, true);
	  collisionConfig_->SetCollisionEnabled(CollisionLayer::Player, CollisionLayer::LightningBullet, true);
	  collisionConfig_->SetCollisionEnabled(CollisionLayer::Player, CollisionLayer::ElasticSphere, true);
	  collisionConfig_->SetCollisionEnabled(CollisionLayer::Boss, CollisionLayer::LightningBullet, false);
	  collisionConfig_->SetCollisionEnabled(CollisionLayer::Boss, CollisionLayer::ElasticSphere, false);
	  collisionManager_ = std::make_unique<CollisionManager>(collisionConfig_.get());
   }

   // ステージ境界を設定（フレームの外側まで）
   // kStageSize はフレームを含めた全体サイズなので、フレーム1個分外側に広げる
   float stageHalfWidth = GameSceneConfig::kStageSize.x / 2.0f;
   float stageHalfHeight = GameSceneConfig::kStageSize.y / 2.0f;
   float frameWidth = GameSceneConfig::kFrameSize.x * 0.65f;
   float frameHeight = GameSceneConfig::kFrameSize.y * 0.6f;

   // カメラコントローラーの初期化（プレイヤーとボスを追跡）
   {
	  cameraController_ = std::make_unique<CameraController>();
	  auto* releaseCamera = static_cast<Camera*>(cameraManager_->GetCamera("Release"));
	  cameraController_->Initialize(releaseCamera, player_, boss_);

	  // カメラパラメータの調整（オプション）
	  cameraController_->SetMinDistance(35.0f);
	  cameraController_->SetMaxDistance(70.0f);
	  cameraController_->SetDistanceScale(1.8f);
	  cameraController_->SetHeightOffset(0.0f);
	  cameraController_->SetPitchAngle(0.0f);
	  cameraController_->SetSmoothSpeed(20.0f);
	  cameraController_->SetMarginDistance(8.0f);
	  cameraController_->SetScreenPadding(0.35f);


	  cameraController_->SetStageBounds(
		 GameSceneConfig::kStageCenter.x - stageHalfWidth - frameWidth,
		 GameSceneConfig::kStageCenter.x + stageHalfWidth + frameWidth,
		 GameSceneConfig::kStageCenter.y - stageHalfHeight - frameHeight,
		 GameSceneConfig::kStageCenter.y + stageHalfHeight + frameHeight
	  );
   }

   // テスト用：直線雷を1本配置
   {
	  frameWidth = GameSceneConfig::kFrameSize.x;
	  frameHeight = GameSceneConfig::kFrameSize.y;

	  LightningEffectManager::LinearEffectConfig config;
	  config.startOffset = { GameSceneConfig::kStageCenter.x - stageHalfWidth - frameWidth, 0.0f, 0.0f };
	  config.endOffset = { GameSceneConfig::kStageCenter.x + stageHalfWidth + frameWidth, 0.0f, 0.0f };
	  config.segmentCount = 12;
	  config.noiseScale = 3.0f;
	  config.noiseSpeed = 30.0f;
	  config.enableAnimation = true;
	  config.color = { 0.3f, 0.6f, 1.0f, 1.0f };   // 青白色
	  config.pathType = Lightning::PathType::Linear;

	  // ステージ中央に固定配置
	  lightningManager_->CreateLinearEffectAtPosition(
		 { 0.0f, -stageHalfHeight + frameHeight, 0.0f },
		 config,
		 gameObjects_
	  );

	  lightningManager_->CreateLinearEffectAtPosition(
		 { 0.0f,stageHalfHeight - frameHeight, 0.0f },
		 config,
		 gameObjects_
	  );

	  config.segmentCount = 8;
	  config.noiseSpeed = 20.0f;
	  config.color = { 0.8f, 1.0f, 1.0f, 1.0f };   // 青白色

	  lightningManager_->CreateLinearEffectAtPosition(
		 { 0.0f, -stageHalfHeight + frameHeight, 0.0f },
		 config,
		 gameObjects_
	  );

	  lightningManager_->CreateLinearEffectAtPosition(
		 { 0.0f,stageHalfHeight - frameHeight, 0.0f },
		 config,
		 gameObjects_
	  );

	  config.startOffset = { 0.0f, -stageHalfHeight - frameHeight, 0.0f };
	  config.endOffset = { 0.0f , stageHalfHeight + frameHeight, 0.0f };
	  config.segmentCount = 6;
	  config.color = { 0.3f, 0.6f, 1.0f, 1.0f };   // 青白色
	  config.noiseSpeed = 30.0f;

	  lightningManager_->CreateLinearEffectAtPosition(
		 { -stageHalfWidth + frameWidth, 0.0f, 0.0f },
		 config,
		 gameObjects_
	  );

	  lightningManager_->CreateLinearEffectAtPosition(
		 { stageHalfWidth - frameWidth,0.0f, 0.0f },
		 config,
		 gameObjects_
	  );

	  config.segmentCount = 5;
	  config.noiseSpeed = 20.0f;
	  config.color = { 0.8f, 1.0f, 1.0f, 1.0f };   // 青白色

	  lightningManager_->CreateLinearEffectAtPosition(
		 { -stageHalfWidth + frameWidth, 0.0f, 0.0f },
		 config,
		 gameObjects_
	  );

	  lightningManager_->CreateLinearEffectAtPosition(
		 { stageHalfWidth - frameWidth,0.0f, 0.0f },
		 config,
		 gameObjects_
	  );
   }
}

void GameScene::Update() {
   // BaseScene::Update()の最初でCleanupGameObjects()が呼ばれる
   BaseScene::Update();

   // カメラコントローラーの更新
   if (cameraController_) {
	  cameraController_->Update();
   }

   // 雷エフェクトの更新
   if (lightningManager_) {
	  lightningManager_->UpdateAllEffects();
   }

   // 削除予定の弾をリストから削除（gameObjects_からは次のフレームの最初に削除）
   bullets_.remove_if([](Bullet* bullet) {
	  return bullet == nullptr || !bullet->IsActive();
	  });

   // 新しいオブジェクトを追加
   if (!newGameObjectsQueue_.empty()) {
	  for (auto& newObj : newGameObjectsQueue_) {
		 gameObjects_.push_back(std::move(newObj));
	  }
	  newGameObjectsQueue_.clear();
   }

   if (playerHitPointUI_) {
	  playerHitPointUI_->SetHP(player_->GetHP());
   }

   if (bossHitPointUI_) {
	  bossHitPointUI_->SetHP(boss_->GetHP());
   }

   if (boss_->GetHP() <= 0 || player_->GetHP() <= 0) {
	  sceneManager_->ChangeScene("ResultScene");
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

void GameScene::RegisterAllColliders() {
   collisionManager_->Clear();
   collisionManager_->RegisterCollider(player_->GetCollider());
   collisionManager_->RegisterCollider(boss_->GetCollider());

   // 弾のコライダーを登録
   for (auto* bullet : bullets_) {
	  if (bullet && bullet->IsActive() && bullet->GetCollider()) {
		 collisionManager_->RegisterCollider(bullet->GetCollider());
	  }
   }
}

void GameScene::CheckCollisions() {
   collisionManager_->CheckAllCollisions();
}

std::unique_ptr<BehaviorTree> GameScene::CreateBossBehaviorTree() {
   return BehaviorTreeFactory::Create(
	  [this](BehaviorTreeBuilder& builder) {
		 builder.Selector()
			.Sequence()
			.Action<FleeFromPlayerAction>(boss_, player_)
			.Action<ChargeToPlayerAction>(boss_, player_)
			.Action<ShootEightWayAction>(boss_, [this](const Vector3& pos, const Vector3& direction, float speed) {
			CreateBullet(pos, direction, BulletType::ElasticSphere, speed);
			   })
			.End()
			.Action<FleeFromPlayerAction>(boss_, player_)
			.End();
	  },
	  "BossMainAI"
   );
}

void GameScene::InitializeFrames() {
   auto modelManager = engine_->GetComponent<ModelManager>();
   auto& textureManager = TextureManager::GetInstance();

   size_t row = static_cast<size_t>(GameSceneConfig::kStageSize.y / GameSceneConfig::kFrameSize.y);
   size_t col = static_cast<size_t>(GameSceneConfig::kStageSize.x / GameSceneConfig::kFrameSize.x);

   float startX = GameSceneConfig::kStageCenter.x - GameSceneConfig::kStageSize.x / 2.0f;
   float startY = GameSceneConfig::kStageCenter.y - GameSceneConfig::kStageSize.y / 2.0f;

   auto frameTexture = textureManager.Load("Resources/Textures/Frame.png");

   for (size_t y = 0; y <= row; ++y) {
	  for (size_t x = 0; x <= col; ++x) {

		 bool isEdge = (y == 0 || y == row || x == 0 || x == col);
		 if (!isEdge) continue;

		 bool isCorner = (y == 0 || y == row) && (x == 0 || x == col);

		 std::unique_ptr<Model> model;
		 float rotation = 0.0f;
		 using std::numbers::pi_v;

		 if (isCorner) {
			model = modelManager->CreateStaticModel("Resources/Models/FrameCorner/FrameCorner.obj");

			// 左下 → 右下 → 右上 → 左上 の順に +90°ずつ回転
			if (x == 0 && y == 0) {
			   rotation = 0.0f;                   // 左下
			} else if (x == col && y == 0) {
			   rotation = pi_v<float> / 2.0f;     // 右下
			} else if (x == col && y == row) {
			   rotation = pi_v<float>;            // 右上
			} else if (x == 0 && y == row) {
			   rotation = pi_v<float> *1.5f;     // 左上
			}

		 } else {
			model = modelManager->CreateStaticModel("Resources/Models/Frame/Frame.obj");

			// 上下は横向き（回転なし）
			// 左右は縦向き（+90°）
			if (y == 0 || y == row) {
			   rotation = 0.0f;
			} else {
			   rotation = pi_v<float> / 2.0f;
			}
		 }

		 auto frame = std::make_unique<Frame>();
		 frame->Initialize(std::move(model), frameTexture);

		 frame->GetTransform().translate = {
			startX + x * GameSceneConfig::kFrameSize.x,
			startY + y * GameSceneConfig::kFrameSize.y,
			0.0f
		 };

		 frame->GetTransform().rotate.z = rotation;

		 frame->GetTransform().SetRotationMode(WorldTransform::RotationMode::Euler);

		 frames_.push_back(frame.get());
		 gameObjects_.push_back(std::move(frame));
	  }
   }
}

Bullet* GameScene::CreateBullet(const Vector3& position, const Vector3& direction, BulletType type, float speed) {
   auto modelManager = engine_->GetComponent<ModelManager>();
   auto& textureManager = TextureManager::GetInstance();

   // タイプに応じたモデルとテクスチャのパス
   std::string modelPath;
   std::string texturePath;
   CollisionLayer collisionLayer;

   switch (type) {
	  case BulletType::LightningBullet:
		 modelPath = "Resources/Models/Ball/Ball.obj";
		 texturePath = "Resources/Textures/Ball.png";
		 collisionLayer = CollisionLayer::LightningBullet;
		 break;
	  case BulletType::ElasticSphere:
		 modelPath = "Resources/Models/Ball/Ball.obj";
		 texturePath = "Resources/Textures/Ball.png";
		 collisionLayer = CollisionLayer::ElasticSphere;
		 break;
	  default:
		 modelPath = "Resources/Models/Ball/Ball.obj";
		 texturePath = "Resources/Textures/Ball.png";
		 collisionLayer = CollisionLayer::LightningBullet;
		 break;
   }

   // 弾のモデルとテクスチャを読み込み
   auto bulletModel = modelManager->CreateStaticModel(modelPath);
   auto bulletTexture = textureManager.Load(texturePath);

   // 弾を生成
   auto bullet = std::make_unique<Bullet>();
   bullet->Initialize(std::move(bulletModel), bulletTexture, direction);
   bullet->SetWorldPosition(position);
   bullet->SetSpeed(speed);

   // コライダーレイヤーを設定
   if (bullet->GetCollider()) {
	  bullet->GetCollider()->SetLayer(collisionLayer);
   }

   Bullet* bulletPtr = bullet.get();
   bullets_.push_back(bulletPtr);
   newGameObjectsQueue_.push_back(std::move(bullet));

   return bulletPtr;
}
