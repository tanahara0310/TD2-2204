#include "Player.h"

void Player::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
   // 基底クラスの初期化を呼び出す
   GameObject::Initialize(std::move(model), texture);

   // キーコンフィグの初期化
   InitializeKeyConfig();

   // 行動リクエストマネージャーの初期化
   InitializeBehaviorRequestManager();

   // 初期行動をMoveに設定
   behavior_->Request("Move", 0);

   // コライダーの初期化
   InitializeCollider();
}

void Player::Update() {
   if (keyConfig_->Get<bool>("Charge")) {
	  behavior_->Request("Charge", 0);
   }

   behavior_->Update();

   UpdateMovement();
}

void Player::Draw(const ICamera* camera) {
   if (!model_ || !camera) {
	  return;
   }

   // モデルの描画
   model_->Draw(transform_, camera, texture_.gpuHandle);
}

void Player::OnCollisionEnter(GameObject* other) {
   // 反発
   Vector3 toOther = other->GetWorldPosition() - GetWorldPosition();

   // 反対方向に加速度を与える
   acceleration_ -= Vector2{ toOther.x, toOther.y }.Normalize() * stunPower_;

   velocity_ *= 0.5f; // 衝突時の速度を半減

   behavior_->Request("Stun", 0);
}

void Player::OnCollisionStay(GameObject* other) {
   // 反発
   Vector3 toOther = other->GetWorldPosition() - GetWorldPosition();

   // 反対方向に加速度を与える
   acceleration_ -= Vector2{ toOther.x, toOther.y }.Normalize() * stunPower_;

   behavior_->Request("Stun", 0);
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

void Player::InitializeBehaviorRequestManager() {
   // 行動リクエストマネージャーの取り付け
   GameObject::AttachBehaviorRequestManager();

   behavior_->AddBehavior("Charge", std::bind(&Player::InitializeChargeBehavior, this), std::bind(&Player::Charge, this));
   behavior_->AddBehavior("Move", std::bind(&Player::InitializeMoveBehavior, this), std::bind(&Player::Move, this));
   behavior_->AddBehavior("Stun", std::bind(&Player::InitializeStunBehavior, this), std::bind(&Player::Stun, this));

   behavior_->AddInterruptRule("Charge", { "Move" ,"Stun" });
   behavior_->AddInterruptRule("Move", { "Charge" ,"Stun" });
   behavior_->AddInterruptRule("Stun", { "Move" });
}

void Player::InitializeCollider() {
   AttachCollider(std::make_unique<SphereCollider>(this, 0.6f));
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

   transform_.translate.x = std::clamp(transform_.translate.x, -moveableAreaRadius_, moveableAreaRadius_);
   transform_.translate.y = std::clamp(transform_.translate.y, -moveableAreaRadius_, moveableAreaRadius_);

   acceleration_ = { 0.0f, 0.0f };

   transform_.TransferMatrix();
}

Vector2 Player::GetMoveDirection() const {
   Vector2 moveInput = keyConfig_->Get<Vector2>("Move");
   return moveInput.Normalize();
}

void Player::Move() {
   acceleration_ = GetMoveDirection() * moveSpeed_;
}

void Player::Charge() {
   chargeTimer_.Update(GameUtils::GetDeltaTime());
   if (chargeTimer_.IsFinished()) {
	  behavior_->Request("Move", 0);
   }
}

void Player::Stun() {
   stunTimer_.Update(GameUtils::GetDeltaTime());
   if (stunTimer_.IsFinished()) {
	  behavior_->Request("Move", 0);
   }
}

void Player::InitializeChargeBehavior() {
   acceleration_ = GetMoveDirection() * chargeSpeed_;
   dampingPerSecond_ = chargeDamping_;
   maxSpeed_ = chargeMaxSpeed_;
   velocity_ = { 0.0f, 0.0f };

   chargeTimer_.Start(chargeDuration_, false);
}

void Player::InitializeMoveBehavior() {
   dampingPerSecond_ = moveDamping_;
   maxSpeed_ = moveMaxSpeed_;
}

void Player::InitializeStunBehavior() {
   dampingPerSecond_ = stunDamping_;
   maxSpeed_ = stunMaxSpeed_;

   stunTimer_.Start(stunDuration_, false);
}
