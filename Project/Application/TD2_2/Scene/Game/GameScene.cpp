#include "GameScene.h"
#include "../../GameObject/Boss/ActionNode/ChargeToPlayerAction.h"
#include "../../GameObject/Boss/ActionNode/FleeFromPlayerAction.h"
#include "../../GameObject/Boss/ActionNode/MoveAction.h"
#include "../../GameObject/Boss/ActionNode/MoveToCenterAction.h"
#include "../../GameObject/Boss/ActionNode/ShootEightWayAction.h"
#include "../../GameObject/Boss/ActionNode/SparkNode.h"
#include "../../GameObject/Bullet/Bullet.h"
#include "../Config/GameSceneConfig.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/Debug/DebugCamera.h"
#include "Engine/Camera/ICamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Light/LightData.h"
#include "Engine/Graphics/Light/LightManager.h"
#include "Engine/Graphics/Render/RenderManager.h"
#include "Engine/Particle/ParticlePresetManager.h"
#include "EngineSystem/EngineSystem.h"
#include "MathCore.h"
#include "Scene/SceneManager.h"
#include <numbers>
#include "../../Camera/CinematicPresetManager.h"
#include "../../Camera/CinematicSequence.h"
#include <algorithm>

void GameScene::Initialize(EngineSystem* engine) {
   // 基底クラスの初期化
   BaseScene::Initialize(engine);

   // ゲームユーティリティの初期化
   GameUtils::Initialize(engine_);

   // ゲームオブジェクトの初期化
   auto modelManager = engine_->GetComponent<ModelManager>();
   auto& textureManager = TextureManager::GetInstance();

   auto& cinematicPresetManager = CinematicPresetManager::GetInstance();
   cinematicPresetManager.RegisterDefaultPresets();

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

		 if (damageSound_ && damageSound_->IsValid()) {
			damageSound_->Play(false);
		 }
		 });
	  player->SetHitEnemyFunction([this]() {
		 if (cameraController_) {
			cameraController_->StartShake(CameraController::ShakeIntensity::Medium);
		 }

		 if (hitSound_ && hitSound_->IsValid()) {
			hitSound_->Play(false);
		 }
		 });
	  player->SetStartChargeFunction([this]() {
		 if (chargeSound_ && chargeSound_->IsValid()) {
			chargeSound_->Play(false);
		 }
		 });

	  player->RegisterModelResource("Damage", "Resources/Models/Player/Damage/PlayerDamage.obj");
	  player->RegisterModelResource("Player1", "Resources/Models/Player/Player.obj");
	  player->RegisterModelResource("Player2", "Resources/Models/PlayerPropeller/PlayerPropeller.obj");

	  // プレイヤー用のダメージエフェクトを作成（球面配置）
	  LightningEffectManager::EffectConfig damageEffectConfig;
	  damageEffectConfig.useSphereDistribution = true;
	  damageEffectConfig.sphereRadius = 2.3f;
	  damageEffectConfig.sphereStartRadiusRatio = 0.75f; // 内側60%の位置から開始
	  damageEffectConfig.randomOffsetRange = 0.0f; // ランダムオフセット範囲
	  damageEffectConfig.lightningCount = 4;
	  damageEffectConfig.color = { 0.2f, 0.8f, 1.0f, 1.0f }; // 赤色
	  damageEffectConfig.noiseScale = 1.5f;
	  damageEffectConfig.noiseSpeed = 15.0f;
	  damageEffectConfig.segmentCount = 4; // セグメント数を減らして短くする
	  damageEffectConfig.voxelScale = { 2.0f, 2.0f, 2.0f }; // ボクセルスケールを小さく
	  damageEffectConfig.fadeInDuration = 0.25f;  // フェードイン時間
	  damageEffectConfig.fadeOutDuration = 0.35f; // フェードアウト時間

	  int damageEffectId = lightningManager_->CreateEffect(
		 player->GetWorldPosition(),
		 damageEffectConfig,
		 gameObjects_
	  );

	  player->SetStartEffectFunction([this, damageEffectId]() {
		 lightningManager_->SetEffectVisible(damageEffectId, true);
		 if (biribiriSound_ && biribiriSound_->IsValid()) {
			biribiriSound_->Play(false);
		 }
		 });

	  player->SetStopEffectFunction([this, damageEffectId]() {
		 lightningManager_->SetEffectVisible(damageEffectId, false);
		 });

	  player->SetUpdateEffectFunction([this, damageEffectId](const Vector3& position) {
		 lightningManager_->SetEffectPosition(damageEffectId, position);
		 });

	  player->SetEffectColorFunction([this, damageEffectId](const Vector4& color) {
		 lightningManager_->SetEffectColor(damageEffectId, color);
		 });

	  player->SetCollisionEffectFunction([this](const Vector3& position) {
		 EmitParticle(playerCollisionParticle_, position);
		 });

	  player->SetExplosionEffectFunction([this](const Vector3& position) {
		 if (playerExplosionParticle_) {
			EmitParticle(playerExplosionParticle_, position);
		 }
		 if (playerSmokeParticle_) {
			EmitParticle(playerSmokeParticle_, position);
		 }
		 }
	  );

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

	  // スパーク当たり判定用オブジェクトの生成
	  constexpr float kSparkRadius = 5.0f;
	  auto sparkColliderObj = std::make_unique<SparkColliderObject>();
	  sparkColliderObj->Initialize(kSparkRadius);
	  sparkCollider_ = sparkColliderObj.get();
	  gameObjects_.push_back(std::move(sparkColliderObj));

	  // ビヘイビアツリーの生成（sparkCollider_を使用するため、先にsparkCollider_を初期化）
	  bossBehaviorTree_ = CreateBossBehaviorTree();
	  boss->Initialize(std::move(bossModel), bossTexture);
	  boss->SetBehaviorTree(std::move(bossBehaviorTree_));
	  boss->RegisterModelResource("Damage", "Resources/Models/Boss/Damage/BossDamage.obj");
	  boss->RegisterModelResource("Boss1", "Resources/Models/Boss/Boss.obj");
	  boss->RegisterModelResource("Boss2", "Resources/Models/BossPropeller/BossPropeller.obj");
	  boss->SetStartDamageFunction([this]() {
		 if (damageSound_ && damageSound_->IsValid()) {
			damageSound_->Play(false);
		 }
		 });
	  boss->SetStartChargeFunction([this]() {
		 if (chargeSound_ && chargeSound_->IsValid()) {
			chargeSound_->Play(false);
		 }
		 });

	  boss->SetExplosionEffectFunction([this](const Vector3& position) {
		 if (bossExplosionParticle_) {
			EmitParticle(bossExplosionParticle_, position);
		 }
		 if (bossSmokeParticle_) {
			EmitParticle(bossSmokeParticle_, position);
		 }
		 });

	  // ボス用ダメージエフェクトの設定
	  LightningEffectManager::EffectConfig damageEffectConfig;
	  damageEffectConfig.useSphereDistribution = true;
	  damageEffectConfig.sphereRadius = 2.3f;
	  damageEffectConfig.sphereStartRadiusRatio = 0.75f; // 内側60%の位置から開始
	  damageEffectConfig.randomOffsetRange = 0.0f; // ランダムオフセット範囲
	  damageEffectConfig.lightningCount = 4;
	  damageEffectConfig.color = { 0.2f, 0.8f, 1.0f, 1.0f }; // 青色
	  damageEffectConfig.noiseScale = 1.5f;
	  damageEffectConfig.noiseSpeed = 15.0f;
	  damageEffectConfig.segmentCount = 4; // セグメント数を減らして短くする
	  damageEffectConfig.voxelScale = { 2.0f, 2.0f, 2.0f }; // ボクセルスケールを小さく
	  damageEffectConfig.fadeInDuration = 0.25f;  // フェードイン時間
	  damageEffectConfig.fadeOutDuration = 0.35f; // フェードアウト時間

	  int damageEffectId = lightningManager_->CreateEffect(
		 boss->GetWorldPosition(),
		 damageEffectConfig,
		 gameObjects_
	  );

	  boss->SetStartEffectFunction([this, damageEffectId]() {
		 lightningManager_->SetEffectVisible(damageEffectId, true);
		 if (biribiriSound_ && biribiriSound_->IsValid()) {
			biribiriSound_->Play(false);
		 }
		 });

	  boss->SetStopEffectFunction([this, damageEffectId]() {
		 lightningManager_->SetEffectVisible(damageEffectId, false);
		 });

	  boss->SetUpdateEffectFunction([this, damageEffectId](const Vector3& position) {
		 lightningManager_->SetEffectPosition(damageEffectId, position);
		 });

	  boss->SetEffectColorFunction([this, damageEffectId](const Vector4& color) {
		 lightningManager_->SetEffectColor(damageEffectId, color);
		 });

	  // スパークエフェクトの設定
	  LightningEffectManager::EffectConfig sparkEffectConfig;
	  sparkEffectConfig.useSphereDistribution = true;
	  sparkEffectConfig.sphereRadius = kSparkRadius + 0.5f; // 当たり判定と同じ半径
	  sparkEffectConfig.sphereStartRadiusRatio = 0.6f;
	  sparkEffectConfig.randomOffsetRange = 0.0f;
	  sparkEffectConfig.lightningCount = 20;
	  sparkEffectConfig.color = { 0.5f, 0.0f, 0.5f, 1.0f };
	  sparkEffectConfig.noiseScale = 2.0f;
	  sparkEffectConfig.noiseSpeed = 20.0f;
	  sparkEffectConfig.segmentCount = 5;
	  sparkEffectConfig.voxelScale = { 2.0f, 2.0f, 2.0f };
	  sparkEffectConfig.fadeInDuration = 0.25f;
	  sparkEffectConfig.fadeOutDuration = 0.35f;

	  sparkEffectId_ = lightningManager_->CreateEffect(
		 boss->GetWorldPosition(),
		 sparkEffectConfig,
		 gameObjects_
	  );

	  boss->SetStartSparkEffectFunction([this]() {
		 lightningManager_->SetEffectVisible(sparkEffectId_, true);
		 if (biribiriSound_ && biribiriSound_->IsValid()) {
			biribiriSound_->Play(false);
		 }
		 });

	  boss->SetStopSparkEffectFunction([this]() {
		 lightningManager_->SetEffectVisible(sparkEffectId_, false);
		 });

	  boss->SetUpdateSparkEffectFunction([this](const Vector3& position) {
		 lightningManager_->SetEffectPosition(sparkEffectId_, position);
		 });

	  gameObjects_.push_back(std::move(boss));
   }

   // 背景の生成と初期化
   {
	  auto backgroundModel = modelManager->CreateStaticModel("Resources/Models/Background/Background2.obj");
	  auto backgroundTexture = textureManager.Load("Resources/Textures/Background2.png");
	  auto background = std::make_unique<Background>();
	  background_ = background.get();
	  background->Initialize(std::move(backgroundModel), backgroundTexture);
	  gameObjects_.push_back(std::move(background));

   }

   // 雲
   {
	  for (int i = 0; i < clouds_.size(); i++) {
		 auto cloudModel = modelManager->CreateStaticModel("Resources/Models/Cloud/Cloud.obj");
		 auto cloudTexture = textureManager.Load("Resources/Textures/Cloud.png");
		 auto cloud = std::make_unique<Cloud>();
		 clouds_[i] = cloud.get();
		 cloud->Initialize(std::move(cloudModel), cloudTexture);
		 gameObjects_.push_back(std::move(cloud));
	  }
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
	  auto bossSprites = bossHitPointUI_->Initialize({ 580.0f, 300.0f }, SettingObject::BOSS, boss_->GetMaxHP());
	  // スプライトをgameObjects_に追加
	  for (auto& sprite : bossSprites) {
		 gameObjects_.push_back(std::move(sprite));
	  }
   }

   // UIの初期化
   {
	  // UIの生成と初期化
	  auto ui = std::make_unique<SpriteObject>();
	  ui->Initialize("Resources/Textures/ACharge.png");
	  ui->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
	  ui->SetAnchor({ 1.0f, 1.0f });
	  ui->GetTransform().translate = { 624.0f, -344.0f, 0.0f };
	  ui_ = ui.get();
	  gameObjects_.push_back(std::move(ui));

	  // スタートUIの生成と初期化
	  auto startUI = std::make_unique<SpriteObject>();
	  startUI->Initialize("Resources/Textures/GameSceneStartUI.png");
	  startUI->GetTransform().scale = { 1.0f, 1.0f, 1.0f };
	  startUI->GetTransform().translate = { 1280.0f, 0.0f, 0.0f };
	  startUI_ = startUI.get();
	  gameObjects_.push_back(std::move(startUI));
   }

   // サウンドの初期化
   {
	  auto soundManager = engine_->GetComponent<SoundManager>();
	  if (soundManager) {
		 bgmSound_ = soundManager->CreateSoundResource("Resources/Audio/BGM/GameSceneBGM.mp3");
		 hitSound_ = soundManager->CreateSoundResource("Resources/Audio/SE/hit.mp3");
		 damageSound_ = soundManager->CreateSoundResource("Resources/Audio/SE/damage.mp3");
		 chargeSound_ = soundManager->CreateSoundResource("Resources/Audio/SE/charge.mp3");
		 biribiriSound_ = soundManager->CreateSoundResource("Resources/Audio/SE/biribiri.mp3");
		 chargeSound_->SetVolume(0.5f);
		 damageSound_->SetVolume(1.0f);
		 biribiriSound_->SetVolume(0.3f);
	  }

	  if (bgmSound_ && bgmSound_->IsValid()) {
		 bgmSound_->Play(true);
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
	  collisionConfig_->SetCollisionEnabled(CollisionLayer::Player, CollisionLayer::Spark, true);
	  collisionConfig_->SetCollisionEnabled(CollisionLayer::Boss, CollisionLayer::LightningBullet, false);
	  collisionConfig_->SetCollisionEnabled(CollisionLayer::Boss, CollisionLayer::ElasticSphere, false);
	  collisionConfig_->SetCollisionEnabled(CollisionLayer::Boss, CollisionLayer::Spark, false);
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
	  cameraController_->SetSmoothSpeed(8.0f);
	  cameraController_->SetMarginDistance(5.0f);  // 20.0f → 5.0f に変更（近づく処理を有効化）
	  cameraController_->SetScreenPadding(0.35f);

	  cameraController_->SetStageBounds(
		 GameSceneConfig::kStageCenter.x - stageHalfWidth - frameWidth, GameSceneConfig::kStageCenter.x + stageHalfWidth + frameWidth,
		 GameSceneConfig::kStageCenter.y - stageHalfHeight - frameHeight, GameSceneConfig::kStageCenter.y + stageHalfHeight + frameHeight);
   }

   // 雷
   {
	  frameWidth = GameSceneConfig::kFrameSize.x;
	  frameHeight = GameSceneConfig::kFrameSize.y;

	  LightningEffectManager::EffectConfig config;
	  config.startOffset = { GameSceneConfig::kStageCenter.x - stageHalfWidth - frameWidth, 0.0f, 0.0f };
	  config.endOffset = { GameSceneConfig::kStageCenter.x + stageHalfWidth + frameWidth, 0.0f, 0.0f };
	  config.segmentCount = 12;
	  config.noiseScale = 2.5f;
	  config.noiseSpeed = 30.0f;
	  config.color = { 0.3f, 0.6f, 1.0f, 1.0f }; // 青白色

	  int id1 = lightningManager_->CreateEffect({ 0.0f, -stageHalfHeight + frameHeight, 0.0f }, config, gameObjects_);
	  lightningManager_->SetEffectVisible(id1, true);

	  int id2 = lightningManager_->CreateEffect({ 0.0f, stageHalfHeight - frameHeight, 0.0f }, config, gameObjects_);
	  lightningManager_->SetEffectVisible(id2, true);

	  config.segmentCount = 8;
	  config.noiseSpeed = 20.0f;
	  config.color = { 0.8f, 1.0f, 1.0f ,1.0f }; // 青白色

	  int id3 = lightningManager_->CreateEffect({ 0.0f, -stageHalfHeight + frameHeight, 0.0f }, config, gameObjects_);
	  lightningManager_->SetEffectVisible(id3, true);

	  int id4 = lightningManager_->CreateEffect({ 0.0f, stageHalfHeight - frameHeight, 0.0f }, config, gameObjects_);
	  lightningManager_->SetEffectVisible(id4, true);

	  config.startOffset = { 0.0f, -stageHalfHeight - frameHeight, 0.0f };
	  config.endOffset = { 0.0f, stageHalfHeight + frameHeight, 0.0f };
	  config.segmentCount = 6;
	  config.color = { 0.3f, 0.6f, 1.0f, 1.0f }; // 青白色
	  config.noiseSpeed = 30.0f;

	  int id5 = lightningManager_->CreateEffect({ -stageHalfWidth + frameWidth, 0.0f, 0.0f }, config, gameObjects_);
	  lightningManager_->SetEffectVisible(id5, true);

	  int id6 = lightningManager_->CreateEffect({ stageHalfWidth - frameWidth, 0.0f, 0.0f }, config, gameObjects_);
	  lightningManager_->SetEffectVisible(id6, true);

	  config.segmentCount = 5;
	  config.noiseSpeed = 20.0f;
	  config.color = { 0.8f, 1.0f, 1.0f, 1.0f }; // 青白色

	  int id7 = lightningManager_->CreateEffect({ -stageHalfWidth + frameWidth, 0.0f, 0.0f }, config, gameObjects_);
	  lightningManager_->SetEffectVisible(id7, true);

	  int id8 = lightningManager_->CreateEffect({ stageHalfWidth - frameWidth, 0.0f, 0.0f }, config, gameObjects_);
	  lightningManager_->SetEffectVisible(id8, true);
   }

   // プレイヤーの帯電ゲージ
   {
	  playerGauge_ = std::make_unique<GaugeUI>();
	  auto sprites = playerGauge_->Initialize(cameraManager_.get(), 5.0f);
	  playerGauge_->SetTarget(player_);
	  playerGauge_->SetSegmentColor({ 0.8f, 0.0f, 0.0f, 1.0f });

	  for (auto& sprite : sprites) {
		 gameObjects_.push_back(std::move(sprite));
	  }
   }

   // ボスの帯電ゲージ
   {
	  bossGauge_ = std::make_unique<GaugeUI>();
	  auto sprites = bossGauge_->Initialize(cameraManager_.get(), 5.0f);
	  bossGauge_->SetTarget(boss_);
	  bossGauge_->SetFillColor({ 0.5f, 0.0f, 0.5f, 1.0f });
	  bossGauge_->SetSegmentColor({ 0.8f, 0.0f, 0.0f, 1.0f });

	  for (auto& sprite : sprites) {
		 gameObjects_.push_back(std::move(sprite));
	  }
   }

   {
	  auto playerParticle = CreateParticleSystem("Resources/Presets/Particle/Collision.json");
	  playerCollisionParticle_ = playerParticle.get();
	  gameObjects_.push_back(std::move(playerParticle));

	  auto bossExplosionParticle = CreateParticleSystem("Resources/Presets/Particle/Explosion.json");
	  bossExplosionParticle_ = bossExplosionParticle.get();
	  gameObjects_.push_back(std::move(bossExplosionParticle));

	  auto playerExplosionParticle = CreateParticleSystem("Resources/Presets/Particle/Explosion.json");
	  playerExplosionParticle_ = playerExplosionParticle.get();
	  gameObjects_.push_back(std::move(playerExplosionParticle));

	  auto bossSmokeParticle = CreateParticleSystem("Resources/Presets/Particle/Smoke.json");
	  bossSmokeParticle_ = bossSmokeParticle.get();
	  gameObjects_.push_back(std::move(bossSmokeParticle));

	  auto playerSmokeParticle = CreateParticleSystem("Resources/Presets/Particle/Smoke.json");
	  playerSmokeParticle_ = playerSmokeParticle.get();
	  gameObjects_.push_back(std::move(playerSmokeParticle));
   }

   stateMachine_ = std::make_unique<StateMachine>();
   stateMachine_->AddState("Opening", std::bind(&GameScene::InitializeOpening, this), std::bind(&GameScene::Opening, this));
   stateMachine_->AddState("Main", std::bind(&GameScene::InitializeMain, this), std::bind(&GameScene::Main, this));
   stateMachine_->AddState("GameOver", std::bind(&GameScene::InitializeGameOver, this), std::bind(&GameScene::GameOver, this));
   stateMachine_->AddState("GameClear", std::bind(&GameScene::InitializeGameClear, this), std::bind(&GameScene::GameClear, this));

   stateMachine_->RequestState("Opening", 0);
}

void GameScene::Update() {

   time_ += GameUtils::GetDeltaTime();

#ifdef _DEBUG

   auto input = engine_->GetComponent<KeyboardInput>();


   if (input->IsKeyTriggered(DIK_0)) {
	  player_->DecreaseHP(1);
   }

   if (input->IsKeyTriggered(DIK_9)) {
	  boss_->DecreaseHP(1);
   }

#endif
   CheckParticleAutoDeactivate(playerCollisionParticle_);
   CheckParticleAutoDeactivate(bossExplosionParticle_);
   CheckParticleAutoDeactivate(playerExplosionParticle_);
   CheckParticleAutoDeactivate(bossSmokeParticle_);
   CheckParticleAutoDeactivate(playerSmokeParticle_);

   stateMachine_->Update();

}

void GameScene::Draw() {
   BaseScene::Draw();
}

void GameScene::Finalize() {}

void GameScene::RegisterAllColliders() {
   collisionManager_->Clear();
   collisionManager_->RegisterCollider(player_->GetCollider());
   collisionManager_->RegisterCollider(boss_->GetCollider());

   // スパークコライダーを登録
   if (sparkCollider_ && sparkCollider_->GetCollider()) {
	  collisionManager_->RegisterCollider(sparkCollider_->GetCollider());
   }

   // 弾のコライダーを登録
   for (auto* bullet : bullets_) {
	  if (bullet && bullet->IsActive() && bullet->GetCollider()) {
		 collisionManager_->RegisterCollider(bullet->GetCollider());
	  }
   }
}

void GameScene::CheckCollisions() { collisionManager_->CheckAllCollisions(); }

std::unique_ptr<BehaviorTree> GameScene::CreateBossBehaviorTree() {
   return BehaviorTreeFactory::Create(
	  [this](BehaviorTreeBuilder& builder) {
		 builder.Selector()
			.Sequence()
			.Action<FleeFromPlayerAction>(boss_, player_)
			/*.Action<ChargeToPlayerAction>(boss_, player_)
			.WeightedSelector()
			.WeightedAction<MoveToCenterAction>(0.3f, boss_)
			.WeightedAction<ShootEightWayAction>(0.2f, boss_, [this](const Vector3& pos, const Vector3& direction, float speed) { CreateBullet(pos, direction, BulletType::ElasticSphere, speed); })
			.WeightedAction<ChargeToPlayerAction>(0.4f, boss_, player_)
			.WeightedAction<SparkNode>(0.1f, boss_, sparkCollider_)
			.End()
			.End()
			.Action<FleeFromPlayerAction>(boss_, player_)*/
			.End();
	  },
	  "BossMainAI");
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
		 if (!isEdge)
			continue;

		 bool isCorner = (y == 0 || y == row) && (x == 0 || x == col);

		 std::unique_ptr<Model> model;
		 float rotation = 0.0f;
		 using std::numbers::pi_v;

		 if (isCorner) {
			model = modelManager->CreateStaticModel("Resources/Models/FrameCorner/FrameCorner.obj");

			// 左下 → 右下 → 右上 → 左上 の順に +90°ずつ回転
			if (x == 0 && y == 0) {
			   rotation = 0.0f; // 左下
			} else if (x == col && y == 0) {
			   rotation = pi_v<float> / 2.0f; // 右下
			} else if (x == col && y == row) {
			   rotation = pi_v<float>; // 右上
			} else if (x == 0 && y == row) {
			   rotation = pi_v<float> *1.5f; // 左上
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

		 frame->GetTransform().translate = { startX + x * GameSceneConfig::kFrameSize.x, startY + y * GameSceneConfig::kFrameSize.y, 0.0f };

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

void GameScene::StartUIAnimation() {
   if (!startUI_)
	  return;

   uiAnimationTimer_.Update(GameUtils::GetDeltaTime());

   float progress = uiAnimationTimer_.GetProgress();
   float easedT = EasingUtil::ApplyComposite(progress, EasingUtil::Type::EaseOutQuint, EasingUtil::Type::EaseInQuint, 0.5f);
   startUI_->GetTransform().translate.x = EasingUtil::Lerp(1280.0f, -1280.0f, easedT);
}

std::unique_ptr<ParticleSystem> GameScene::CreateParticleSystem(const std::string& presetPath) {
   auto dxCommon = engine_->GetComponent<DirectXCommon>();
   auto resourceFactory = engine_->GetComponent<ResourceFactory>();
   auto modelManager = engine_->GetComponent<ModelManager>();

   // ModelResourceを取得（必要に応じてモデルを読み込む）
   auto* voxelModelResource = modelManager->GetModelResource("Resources/Models/Voxel/Voxel.obj");
   if (!voxelModelResource) {
	  modelManager->LoadModelResource("Resources/Models/Voxel", "Voxel.obj");
	  voxelModelResource = modelManager->GetModelResource("Resources/Models/Voxel/Voxel.obj");
   }

   // パーティクルシステムを作成
   auto particleSystem = std::make_unique<ParticleSystem>();
   particleSystem->Initialize(dxCommon, resourceFactory);

   if (voxelModelResource) {
	  particleSystem->SetModelResource(voxelModelResource);
   }

   particleSystem->SetTexture("Resources/SampleResources/white1x1.png");

   // プリセットファイルから設定を読み込む
   ParticlePresetManager presetManager;
   presetManager.LoadPreset(particleSystem.get(), presetPath);

   // 初期状態を非アクティブに設定
   particleSystem->SetActive(false);

   return particleSystem;
}

void GameScene::EmitParticle(ParticleSystem* particleSystem, const Vector3& position) {
   if (!particleSystem) {
	  return;
   }

   particleSystem->SetActive(true);
   particleSystem->SetEmitterPosition(position);
   particleSystem->Clear();
   particleSystem->GetMainModule().Restart();
   particleSystem->Play();
}

void GameScene::CheckParticleAutoDeactivate(ParticleSystem* particleSystem) {
   if (!particleSystem || !particleSystem->IsActive()) {
	  return;
   }

   if (particleSystem->IsFinished()) {
	  particleSystem->SetActive(false);
   }
}

void GameScene::InitializeOpening() {
   uiAnimationTimer_.Start(3.0f);
   cameraController_->StartCinematicByName("Opening");
}

void GameScene::Opening() {
   BaseScene::Update();
   // カメラコントローラーの更新
   if (cameraController_) {
	  cameraController_->Update();
   }

   // 雷エフェクトの更新
   if (lightningManager_) {
	  lightningManager_->UpdateAllEffects();
   }

   StartUIAnimation();

   // 削除予定の弾をリストから削除（gameObjects_からは次のフレームの最初に削除）
   bullets_.remove_if([](Bullet* bullet) { return bullet == nullptr || !bullet->IsActive(); });

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

   // 帯電ゲージの更新
   if (playerGauge_) {
	  playerGauge_->SetValue(player_->GetStoredEnergy());
	  playerGauge_->Update();
   }

   if (bossGauge_) {
	  bossGauge_->SetValue(boss_->GetStoredEnergy());
	  bossGauge_->Update();
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

   if (uiAnimationTimer_.IsFinished()) {
	  stateMachine_->RequestState("Main", 0);
   }
}

void GameScene::InitializeMain() {

}

void GameScene::Main() {
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
   bullets_.remove_if([](Bullet* bullet) { return bullet == nullptr || !bullet->IsActive(); });

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

   // 帯電ゲージの更新
   if (playerGauge_) {
	  playerGauge_->SetValue(player_->GetStoredEnergy());
	  playerGauge_->Update();
   }

   if (bossGauge_) {
	  bossGauge_->SetValue(boss_->GetStoredEnergy());
	  bossGauge_->Update();
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

   // ゲームクリア／ゲームオーバーの判定
   if (boss_->GetHP() <= 0) {
	  stateMachine_->RequestState("GameClear", 0);
   } else if (player_->GetHP() <= 0) {
	  stateMachine_->RequestState("GameOver", 0);
   }
}

void GameScene::InitializeGameOver() {
   boss_->SetActive(false);

   for (auto& bullet : bullets_) {
	  bullet->SetActive(false);
   }

   // 削除予定の弾をリストから削除（gameObjects_からは次のフレームの最初に削除）
   bullets_.remove_if([](Bullet* bullet) { return bullet == nullptr || !bullet->IsActive(); });

   collisionManager_->Clear();

   // カット追跡変数をリセット
   lastCutIndex_ = -1;

   Vector3 currentPos = cameraController_->GetCurrentCameraPos();
   Vector3 playerPos = player_->GetWorldPosition();

   // ゲームオーバーシーケンスを作成（近づく→固定→離れる）
   auto gameOverSequence = std::make_shared<CinematicSequence>();

   // カット1: プレイヤーに近づく
   {
	  CinematicCut cut1;
	  cut1.config.type = CameraController::CinematicType::Dolly;
	  cut1.config.duration = 0.8f;
	  cut1.config.startPosition = currentPos;
	  cut1.config.endPosition = { playerPos.x, playerPos.y, playerPos.z - 8.0f };
	  cut1.config.startRotation = { 0.0f, 0.0f, 0.0f };
	  cut1.config.endRotation = { 0.0f, 0.0f, 0.0f };
	  cut1.config.useEasing = true;
	  cut1.config.easingType = "EaseInOutQuad";
	  cut1.name = "GameOver_Approach";
	  cut1.duration = 0.8f;
	  gameOverSequence->AddCut(cut1);
   }

   // カット2: プレイヤーの近くで固定
   {
	  CinematicCut cut2;
	  cut2.config.type = CameraController::CinematicType::FixedPosition;
	  cut2.config.duration = 1.0f;
	  cut2.config.startPosition = { playerPos.x, playerPos.y, playerPos.z - 8.0f };
	  cut2.config.endPosition = { playerPos.x, playerPos.y, playerPos.z - 8.0f };
	  cut2.config.targetPosition = playerPos;
	  cut2.config.useEasing = false;
	  cut2.name = "GameOver_Hold";
	  cut2.duration = 1.0f;
	  gameOverSequence->AddCut(cut2);
   }

   // カット3: プレイヤーから離れる
   {
	  CinematicCut cut3;
	  cut3.config.type = CameraController::CinematicType::Dolly;
	  cut3.config.duration = 0.4f;
	  cut3.config.startPosition = { playerPos.x, playerPos.y, playerPos.z - 8.0f };
	  cut3.config.endPosition = { playerPos.x, playerPos.y, playerPos.z - 12.0f };
	  cut3.config.startRotation = { 0.0f, 0.0f, 0.0f };
	  cut3.config.endRotation = { 0.0f, 0.0f, 0.0f };
	  cut3.config.useEasing = true;
	  cut3.config.easingType = "EaseOutQuad";
	  cut3.name = "GameOver_Retreat";
	  cut3.duration = 0.4f;
	  gameOverSequence->AddCut(cut3);
   }

   cameraController_->StartSequence(gameOverSequence);
}

void GameScene::GameOver() {

   BaseScene::Update();

   // カメラコントローラーの更新
   if (cameraController_) {
	  // カットの切り替えを検知してシェイクを発動
	  if (cameraController_->IsSequenceActive()) {
		 int currentCutIndex = cameraController_->GetSequenceCurrentCutIndex();

		 // カット2（インデックス1）からカット3（インデックス2）に切り替わった瞬間
		 if (lastCutIndex_ == 1 && currentCutIndex == 2) {
			cameraController_->StartShake(CameraController::ShakeIntensity::Large);
		 }

		 lastCutIndex_ = currentCutIndex;
	  } else {
		 // シーケンスが終了したらリセット
		 lastCutIndex_ = -1;
	  }

	  cameraController_->Update();
   }

   // 雷エフェクトの更新
   if (lightningManager_) {
	  lightningManager_->UpdateAllEffects();
   }

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

   // 帯電ゲージの更新
   if (playerGauge_) {
	  playerGauge_->SetValue(player_->GetStoredEnergy());
	  playerGauge_->Update();
   }

   if (bossGauge_) {
	  bossGauge_->SetValue(boss_->GetStoredEnergy());
	  bossGauge_->Update();
   }

   // シーケンス終了後にリザルトシーンへ遷移
   if (!cameraController_->IsSequenceActive()) {
	  sceneManager_->ChangeScene("ResultScene");

	  json clearTimeData = JsonManager::GetInstance().LoadJson("Resources/Data/CurrentClearTime.json");
	  clearTimeData["CurrentClearTime"] = time_;

	  JsonManager::GetInstance().SaveJson("Resources/Data/CurrentClearTime.json", clearTimeData);

	  json resultData = JsonManager::GetInstance().LoadJson("Resources/Data/result.json");
	  resultData["isWin"] = false;

	  JsonManager::GetInstance().SaveJson("Resources/Data/result.json", resultData);
   }
}

void GameScene::InitializeGameClear() {
   player_->SetActive(false);

   for (auto& bullet : bullets_) {
	  bullet->SetActive(false);
   }

   // 削除予定の弾をリストから削除（gameObjects_からは次のフレームの最初に削除）
   bullets_.remove_if([](Bullet* bullet) { return bullet == nullptr || !bullet->IsActive(); });

   // カット追跡変数をリセット
   lastCutIndex_ = -1;

   collisionManager_->Clear();

   Vector3 currentPos = cameraController_->GetCurrentCameraPos();
   Vector3 bossPos = boss_->GetWorldPosition();

   // ゲームクリアシーケンスを作成（近づく→固定→離れる）
   auto gameClearSequence = std::make_shared<CinematicSequence>();

   // カット1: ボスに近づく
   {
	  CinematicCut cut1;
	  cut1.config.type = CameraController::CinematicType::Dolly;
	  cut1.config.duration = 0.8f;
	  cut1.config.startPosition = currentPos;
	  cut1.config.endPosition = { bossPos.x, bossPos.y, bossPos.z - 8.0f };
	  cut1.config.startRotation = { 0.0f, 0.0f, 0.0f };
	  cut1.config.endRotation = { 0.0f, 0.0f, 0.0f };
	  cut1.config.useEasing = true;
	  cut1.config.easingType = "EaseInOutQuad";
	  cut1.name = "GameClear_Approach";
	  cut1.duration = 0.8f;
	  gameClearSequence->AddCut(cut1);
   }

   // カット2: ボスの近くで固定
   {
	  CinematicCut cut2;
	  cut2.config.type = CameraController::CinematicType::FixedPosition;
	  cut2.config.duration = 1.0f;
	  cut2.config.startPosition = { bossPos.x, bossPos.y, bossPos.z - 8.0f };
	  cut2.config.endPosition = { bossPos.x, bossPos.y, bossPos.z - 8.0f };
	  cut2.config.targetPosition = bossPos;
	  cut2.config.useEasing = false;
	  cut2.name = "GameClear_Hold";
	  cut2.duration = 1.0f;
	  gameClearSequence->AddCut(cut2);
   }

   // カット3: ボスから離れる
   {
	  CinematicCut cut3;
	  cut3.config.type = CameraController::CinematicType::Dolly;
	  cut3.config.duration = 0.4f;
	  cut3.config.startPosition = { bossPos.x, bossPos.y, bossPos.z - 8.0f };
	  cut3.config.endPosition = { bossPos.x, bossPos.y, bossPos.z - 12.0f };
	  cut3.config.startRotation = { 0.0f, 0.0f, 0.0f };
	  cut3.config.endRotation = { 0.0f, 0.0f, 0.0f };
	  cut3.config.useEasing = true;
	  cut3.config.easingType = "EaseOutQuad";
	  cut3.name = "GameClear_Retreat";
	  cut3.duration = 0.4f;
	  gameClearSequence->AddCut(cut3);
   }

   cameraController_->StartSequence(gameClearSequence);
}

void GameScene::GameClear() {
   BaseScene::Update();

   // カメラコントローラーの更新
   if (cameraController_) {
	  // カットの切り替えを検知してシェイクを発動
	  if (cameraController_->IsSequenceActive()) {
		 int currentCutIndex = cameraController_->GetSequenceCurrentCutIndex();

		 // カット2（インデックス1）からカット3（インデックス2）に切り替わった瞬間
		 if (lastCutIndex_ == 1 && currentCutIndex == 2) {
			cameraController_->StartShake(CameraController::ShakeIntensity::Large);
		 }

		 lastCutIndex_ = currentCutIndex;
	  } else {
		 // シーケンスが終了したらリセット
		 lastCutIndex_ = -1;
	  }

	  cameraController_->Update();
   }

   // 雷エフェクトの更新
   if (lightningManager_) {
	  lightningManager_->UpdateAllEffects();
   }

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

   // 帯電ゲージの更新
   if (playerGauge_) {
	  playerGauge_->SetValue(player_->GetStoredEnergy());
	  playerGauge_->Update();
   }

   if (bossGauge_) {
	  bossGauge_->SetValue(boss_->GetStoredEnergy());
	  bossGauge_->Update();
   }

   // シーケンス終了後にリザルトシーンへ遷移
   if (!cameraController_->IsSequenceActive()) {
	  sceneManager_->ChangeScene("ResultScene");

	  json clearTimeData = JsonManager::GetInstance().LoadJson("Resources/Data/CurrentClearTime.json");
	  clearTimeData["CurrentClearTime"] = time_;

	  JsonManager::GetInstance().SaveJson("Resources/Data/CurrentClearTime.json", clearTimeData);

	  json resultData = JsonManager::GetInstance().LoadJson("Resources/Data/result.json");
	  resultData["isWin"] = true;

	  JsonManager::GetInstance().SaveJson("Resources/Data/result.json", resultData);
   }
}
