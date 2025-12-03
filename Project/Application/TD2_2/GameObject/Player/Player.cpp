#include "Player.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include <algorithm>

// Avoid Windows min/max macro collisions
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#ifdef _DEBUG
#include <imgui.h>
#endif

void Player::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
   // 基底クラスの初期化を呼び出す
   GameObject::Initialize(std::move(model), texture);

   // キーコンフィグの初期化
   InitializeKeyConfig();

   // ステートマシンの初期化
   InitializeStateMachine();

   // 初期状態をMoveに設定
   stateMachine_->RequestState("Move", 0);

   // コライダーの初期化
   InitializeCollider();

   transform_.translate = { 15.0f, 0.0f, 0.0f };

   transform_.TransferMatrix();
}

void Player::Update() {
   if (keyConfig_->Get<bool>("Charge")) {
	  if (GetMoveDirection().Length() > 0.0f) {
		 stateMachine_->RequestState("Charge", 0);
	  }
   }

   // ダメージ壁との接触判定（ダメージ状態以外、かつ無敵時間でない場合のみ）
   if (stateMachine_->GetCurrentState() != "Damage" &&
       stateMachine_->GetCurrentState() != "Despawn" &&
       stateMachine_->GetCurrentState() != "Respawn" &&
       !IsInvincible()) {
      CheckDamageWallCollision();
   }

   stateMachine_->Update();

   GameObject::UpdateModelSwapAnimation();

   // 無敵時間の更新
   GameObject::UpdateInvincibility();

   UpdateRotation();

   UpdateMovement();

   transform_.TransferMatrix();
}

void Player::Draw(const ICamera* camera) {
   if (!model_ || !camera) {
	  return;
   }

   // モデルの描画
   model_->Draw(transform_, camera, texture_.gpuHandle);
}

void Player::OnCollisionEnter(GameObject* other) {

   if (IsInvincible() || stateMachine_->GetCurrentState() == "Respawn" || stateMachine_->GetCurrentState() == "Despawn") {
	  return;
   }

   if (hitEnemyFunction_) {
	  hitEnemyFunction_();
   }

   // 反発
   Vector3 toOther3 = other->GetWorldPosition() - GetWorldPosition();

   Vector2 toOther = Vector2{ toOther3.x, toOther3.y };
   Vector2 normal = toOther.Normalize();

   // 速度に応じて反発力を増減させる。両者の速度差を使う。
   Vector2 otherVel = { 0.0f, 0.0f };
   // other が Boss か Player かを判別して速度を取得
   if (auto p = dynamic_cast<Player*>(other)) {
	  otherVel = p->GetVelocity();
   } else if (auto b = dynamic_cast<Boss*>(other)) {
	  otherVel = b->GetVelocity();
   }

   Vector2 relativeVel = velocity_ - otherVel;
   float speed = relativeVel.Length();

   float response = stunPower_ + speed * collisionResponseScale_;

   // 突進中かつ相手に向かって突進している場合は反発を弱める
   if (isCharging_) {
	  Vector2 chargeDir = direction_.Normalize();
	  if (chargeDir.Length() > 0.0f) {
		 float dot = chargeDir.x * normal.x + chargeDir.y * normal.y; // cos(theta)
		 // dot が大きいほど相手方向に突進している
		 if (dot > 0.0f) {
			response *= 0.1f;
		 }
	  }
   }

   // clamp to max
   if (response > maxCollisionResponse_) response = maxCollisionResponse_;

   // 反対方向に加速度を与える
   acceleration_ -= normal * response;

   velocity_ *= 0.1f; // 衝突時に速度を半減

   stateMachine_->RequestState("Stun", 0);
}

void Player::OnCollisionStay(GameObject* other) {
   if (IsInvincible() || stateMachine_->GetCurrentState() == "Respawn" || stateMachine_->GetCurrentState() == "Despawn") {
	  return;
   }

   if (hitEnemyFunction_) {
	  hitEnemyFunction_();
   }

   // 反発
   Vector3 toOther3 = other->GetWorldPosition() - GetWorldPosition();

   Vector2 toOther = Vector2{ toOther3.x, toOther3.y };
   Vector2 normal = toOther.Normalize();

   Vector2 otherVel = { 0.0f, 0.0f };
   if (auto p = dynamic_cast<Player*>(other)) {
	  otherVel = p->GetVelocity();
   } else if (auto b = dynamic_cast<Boss*>(other)) {
	  otherVel = b->GetVelocity();
   }

   Vector2 relativeVel = velocity_ - otherVel;
   float speed = relativeVel.Length();

   float response = stunPower_ + speed * collisionResponseScale_;

   if (isCharging_) {
	  Vector2 chargeDir = direction_.Normalize();
	  if (chargeDir.Length() > 0.0f) {
		 float dot = chargeDir.x * normal.x + chargeDir.y * normal.y;
		 if (dot > 0.0f) {
			response *= 0.1f;
		 }
	  }
   }

   if (response > maxCollisionResponse_) response = maxCollisionResponse_;

   // 反対方向に加速度を与える
   acceleration_ -= normal * response;

   stateMachine_->RequestState("Stun", 0);
}

void Player::OnCollisionExit(GameObject* other) {
   (void)other;
}

void Player::InitializeKeyConfig() {
   // キーコンフィグの作成
   keyConfig_ = std::make_unique<KeyConfig>();

   // Moveアクションの追加とバインド設定
   keyConfig_->AddAction("Move", ActionType::Vector2);
   ActionBuilder(keyConfig_->GetAction("Move"))
	  .BindKeyboardWASD(DIK_W, DIK_S, DIK_A, DIK_D)
	  .BindGamepadLeftStick();

   // Chargeアクションの追加とバインド設定
   keyConfig_->AddAction("Charge", ActionType::Bool);
   ActionBuilder(keyConfig_->GetAction("Charge"))
	  .BindKey(DIK_SPACE)
	  .BindGamepadButton(GamepadButton::A);
}

void Player::InitializeStateMachine() {
   // ステートマシンの取り付け
   GameObject::AttachStateMachine();

   stateMachine_->AddState("Charge", std::bind(&Player::InitializeCharge, this), std::bind(&Player::Charge, this));
   stateMachine_->AddState("Move", std::bind(&Player::InitializeMove, this), std::bind(&Player::Move, this));
   stateMachine_->AddState("Stun", std::bind(&Player::InitializeStun, this), std::bind(&Player::Stun, this));
   stateMachine_->AddState("Damage", std::bind(&Player::InitializeDamage, this), std::bind(&Player::Damage, this));
   stateMachine_->AddState("Despawn", std::bind(&Player::InitializeDespawn, this), std::bind(&Player::Despawn, this));
   stateMachine_->AddState("Respawn", std::bind(&Player::InitializeRespawn, this), std::bind(&Player::Respawn, this));

   stateMachine_->AddTransitionRule("Charge", { "Move" ,"Stun" ,"Damage" });
   stateMachine_->AddTransitionRule("Move", { "Charge" ,"Stun" ,"Damage" });
   stateMachine_->AddTransitionRule("Stun", { "Move" ,"Damage" });
   stateMachine_->AddTransitionRule("Damage", { "Despawn" });
   stateMachine_->AddTransitionRule("Despawn", { "Respawn" });
   stateMachine_->AddTransitionRule("Respawn", { "Move" });
}

void Player::InitializeCollider() {
   AttachCollider(std::make_unique<SphereCollider>(this, 1.0f));
   collider_->SetLayer(CollisionLayer::Player);
}

void Player::UpdateMovement() {
   // velocity 更新
   velocity_.x += acceleration_.x * GameUtils::GetDeltaTime();
   velocity_.y += acceleration_.y * GameUtils::GetDeltaTime();

   float factor = std::pow(dampingPerSecond_, GameUtils::GetDeltaTime());
   velocity_.x *= factor;
   velocity_.y *= factor;

   velocity_.x = std::clamp(velocity_.x, -maxSpeed_, maxSpeed_);
   velocity_.y = std::clamp(velocity_.y, -maxSpeed_, maxSpeed_);

   // transform に反映
   transform_.translate.x += velocity_.x * GameUtils::GetDeltaTime();
   transform_.translate.y += velocity_.y * GameUtils::GetDeltaTime();

   transform_.translate.z = 0.0f; // Z座標は固定

   transform_.translate.x = std::clamp(transform_.translate.x, -GameSceneConfig::kMoveableAreaSize.x * 0.5f, GameSceneConfig::kMoveableAreaSize.x * 0.5f);
   transform_.translate.y = std::clamp(transform_.translate.y, -GameSceneConfig::kMoveableAreaSize.y * 0.5f, GameSceneConfig::kMoveableAreaSize.y * 0.5f);

   acceleration_ = { 0.0f, 0.0f };
}

Vector2 Player::GetMoveDirection() const {
   Vector2 moveInput = keyConfig_->Get<Vector2>("Move");
   return moveInput.Normalize();
}

void Player::UpdateRotation() {

   direction_.x = std::clamp(direction_.x, -1.0f, 1.0f);
   direction_.y = std::clamp(direction_.y, -1.0f, 1.0f);

   if (direction_.Length() == 0.0f) {
	  direction_ = velocity_.Normalize();
	  direction_.x = std::clamp(direction_.x, -0.2f, 0.2f);
	  direction_.y = std::clamp(direction_.y, -0.2f, 0.2f);
   }

   GameObject::TiltByVelocity(direction_);
   GameObject::UpdateRotation();
}

void Player::Move() {
   acceleration_ = GetMoveDirection() * moveSpeed_;

   direction_ = GetMoveDirection();
}

void Player::Charge() {

   chargeTimer_.Update(GameUtils::GetDeltaTime());
   if (chargeTimer_.IsFinished()) {
	  stateMachine_->RequestState("Move", 0);
   }
}

void Player::Stun() {

   stunTimer_.Update(GameUtils::GetDeltaTime());
   if (stunTimer_.IsFinished()) {
	  stateMachine_->RequestState("Move", 0);
   }
}

void Player::Damage() {
   if (damageFunction_) {
	  damageFunction_();
   }

   if (GameObject::UpdateShake()) return;

   // HPを減らしてデスポーンステートに遷移
   hp_--;
   stateMachine_->RequestState("Despawn", 0);
}

void Player::InitializeCharge() {
   acceleration_ = GetMoveDirection() * chargeSpeed_;
   dampingPerSecond_ = chargeDamping_;
   maxSpeed_ = chargeMaxSpeed_;
   velocity_ = { 0.0f, 0.0f };

   chargeTimer_.Start(chargeDuration_, false);

   StartRotateAroundAxis(chargeDuration_, 3.0f);

   direction_ = GetMoveDirection() * chargeSpeed_;

   isCharging_ = true;
}

void Player::InitializeMove() {
   dampingPerSecond_ = moveDamping_;
   maxSpeed_ = moveMaxSpeed_;
   StartModelSwapAnimation("Player1", "Player2", 0.02f, true);
   isCharging_ = false;
}

void Player::InitializeStun() {
   dampingPerSecond_ = stunDamping_;
   maxSpeed_ = stunMaxSpeed_;

   stunTimer_.Start(stunDuration_, false);

   StopModelSwapAnimation();

   ChangeToRegisteredModel("Damage");

   isCharging_ = false;
}

void Player::InitializeDamage() {
   if (startDamageFunction_) {
	  startDamageFunction_();
   }

   StopModelSwapAnimation();

   GameObject::StartShake(0.15f, 1.0f);

   velocity_ = { 0.0f, 0.0f };

   ChangeToRegisteredModel("Damage");

   isCharging_ = false;
}

void Player::InitializeDespawn() {
   despawnTimer_.Start(despawnDuration_, false);
   velocity_ = { 0.0f, 0.0f };
   acceleration_ = { 0.0f, 0.0f };
   
   StopModelSwapAnimation();
   ChangeToRegisteredModel("Damage");
   
   isCharging_ = false;
}

void Player::Despawn() {
   despawnTimer_.Update(GameUtils::GetDeltaTime());
   
   // スケールを0にイージング
   float progress = despawnTimer_.GetEasedProgress(EasingUtil::Type::EaseInCubic);
   float scale = GameUtils::Lerp(1.0f, 0.0f, progress);
   transform_.scale = { scale, scale, scale };
   
   if (despawnTimer_.IsFinished()) {
      stateMachine_->RequestState("Respawn", 0);
   }
}

void Player::InitializeRespawn() {
   respawnTimer_.Start(respawnDuration_, false);
   
   // ポジションをステージ中央に設定
   transform_.translate = { 15.0f, 0.0f, 0.0f };
   
   velocity_ = { 0.0f, 0.0f };
   acceleration_ = { 0.0f, 0.0f };

   StartModelSwapAnimation("Player1", "Player2", 0.02f, true);
}

void Player::Respawn() {
   respawnTimer_.Update(GameUtils::GetDeltaTime());
   
   // スケールを1に戻すイージング
   float progress = respawnTimer_.GetEasedProgress(EasingUtil::Type::EaseOutCubic);
   float scale = GameUtils::Lerp(0.0f, 1.0f, progress);
   transform_.scale = { scale, scale, scale };
   
   if (respawnTimer_.IsFinished()) {
      // リスポーン完了時に無敵時間を開始（2秒間、0.1秒間隔で点滅）
      StartInvincibility(2.0f, 0.1f);
      stateMachine_->RequestState("Move", 0);
   }
}

void Player::CheckDamageWallCollision() {
   // ダメージ壁の範囲を計算（フレームより1ブロック内側）
   float damageWallHalfWidth = (GameSceneConfig::kMoveableAreaSize.x * 0.5f) - GameSceneConfig::kFrameSize.x;
   float damageWallHalfHeight = (GameSceneConfig::kMoveableAreaSize.y * 0.5f) - GameSceneConfig::kFrameSize.y;
   
   // プレイヤーがダメージ壁に接触したか判定
   if (std::abs(transform_.translate.x) >= damageWallHalfWidth ||
       std::abs(transform_.translate.y) >= damageWallHalfHeight) {
      // ダメージステートに遷移
      stateMachine_->RequestState("Damage", 1);
   }
}