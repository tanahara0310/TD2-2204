#include "Player.h"

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
}

void Player::Update() {
   if (keyConfig_->Get<bool>("Charge")) {
      stateMachine_->RequestState("Charge", 0);
   }

   stateMachine_->Update();

   UpdateMovement();
}

void Player::Draw(const ICamera* camera) {
   if (!model_ || !camera) {
      return;
   }

   // モデルの描画
   model_->Draw(transform_, camera, texture_.gpuHandle);
}

bool Player::DrawImGui() {
#ifdef _DEBUG
   bool changed = false;
   
   if (ImGui::TreeNode(GetObjectName())) {
      // アクティブ状態
      bool active = IsActive();
      if (ImGui::Checkbox("アクティブ", &active)) {
         SetActive(active);
         changed = true;
      }
      
      // 現在のステート表示
      if (stateMachine_) {
         ImGui::Text("現在のステート: %s", stateMachine_->GetCurrentState().c_str());
      }
      
      // 速度情報
      ImGui::Separator();
      ImGui::Text("速度: (%.2f, %.2f)", velocity_.x, velocity_.y);
      ImGui::Text("加速度: (%.2f, %.2f)", acceleration_.x, acceleration_.y);
      ImGui::Text("最大速度: %.2f", maxSpeed_);
      
      // 移動パラメータ
      if (ImGui::TreeNode("移動パラメータ")) {
         ImGui::DragFloat("移動速度", &moveSpeed_, 1.0f, 0.0f, 100.0f);
         ImGui::DragFloat("移動減衰率", &moveDamping_, 0.01f, 0.0f, 1.0f);
         ImGui::DragFloat("移動最大速度", &moveMaxSpeed_, 0.1f, 0.0f, 50.0f);
         ImGui::TreePop();
      }
      
      // 突進パラメータ
      if (ImGui::TreeNode("突進パラメータ")) {
         ImGui::DragFloat("突進速度", &chargeSpeed_, 10.0f, 0.0f, 10000.0f);
         ImGui::DragFloat("突進減衰率", &chargeDamping_, 0.001f, 0.0f, 1.0f);
         ImGui::DragFloat("突進持続時間", &chargeDuration_, 0.01f, 0.0f, 2.0f);
         ImGui::DragFloat("突進最大速度", &chargeMaxSpeed_, 0.1f, 0.0f, 100.0f);
         ImGui::TreePop();
      }
      
      // スタンパラメータ
      if (ImGui::TreeNode("スタンパラメータ")) {
         ImGui::DragFloat("スタン反発力", &stunPower_, 10.0f, 0.0f, 5000.0f);
         ImGui::DragFloat("スタン持続時間", &stunDuration_, 0.01f, 0.0f, 2.0f);
         ImGui::DragFloat("スタン減衰率", &stunDamping_, 0.001f, 0.0f, 1.0f);
         ImGui::DragFloat("スタン最大速度", &stunMaxSpeed_, 0.1f, 0.0f, 100.0f);
         ImGui::TreePop();
      }
      
      // 移動エリア
      ImGui::DragFloat("移動可能範囲", &moveableAreaRadius_, 1.0f, 0.0f, 100.0f);
      
      // トランスフォーム
      if (transform_.DrawImGui(GetObjectName())) {
         transform_.TransferMatrix();
         changed = true;
      }
      
      ImGui::TreePop();
   }
   
   return changed;
#else
   return false;
#endif
}

void Player::OnCollisionEnter(GameObject* other) {
   // 反発
   Vector3 toOther = other->GetWorldPosition() - GetWorldPosition();

   // 反対方向に加速度を与える
   acceleration_ -= Vector2{ toOther.x, toOther.y }.Normalize() * stunPower_;

   velocity_ *= 0.5f; // 衝突時の速度を半減

   stateMachine_->RequestState("Stun", 0);
}

void Player::OnCollisionStay(GameObject* other) {
   // 反発
   Vector3 toOther = other->GetWorldPosition() - GetWorldPosition();

   // 反対方向に加速度を与える
   acceleration_ -= Vector2{ toOther.x, toOther.y }.Normalize() * stunPower_;

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

   stateMachine_->AddState("Charge", std::bind(&Player::InitializeChargeBehavior, this), std::bind(&Player::Charge, this));
   stateMachine_->AddState("Move", std::bind(&Player::InitializeMoveBehavior, this), std::bind(&Player::Move, this));
   stateMachine_->AddState("Stun", std::bind(&Player::InitializeStunBehavior, this), std::bind(&Player::Stun, this));

   stateMachine_->AddTransitionRule("Charge", { "Move" ,"Stun" });
   stateMachine_->AddTransitionRule("Move", { "Charge" ,"Stun" });
   stateMachine_->AddTransitionRule("Stun", { "Move" });
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
	  stateMachine_->RequestState("Move", 0);
   }
}

void Player::Stun() {
   stunTimer_.Update(GameUtils::GetDeltaTime());
   if (stunTimer_.IsFinished()) {
	  stateMachine_->RequestState("Move", 0);
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
