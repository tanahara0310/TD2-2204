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
   behavior_->AddBehavior("Move", nullptr, std::bind(&Player::Move, this));

   behavior_->AddInterruptRule("Charge", { "Move" });
   behavior_->AddInterruptRule("Move", { "Charge" });
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

   acceleration_ = { 0.0f, 0.0f };

   transform_.TransferMatrix();
}

Vector2 Player::GetMoveDirection() const {
   Vector2 moveInput = keyConfig_->Get<Vector2>("Move");
   return GameUtils::Normalize(moveInput);
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
