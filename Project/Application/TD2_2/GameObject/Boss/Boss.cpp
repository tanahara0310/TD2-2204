#include "Boss.h"
#include "Application/TD2_2/GameObject/Player/Player.h"
#include <cmath>
#include "Application/TD2_2/AI/BehaviorTree/BehaviorTree.h"
#include <algorithm>

#ifdef _DEBUG
#include <imgui.h>
#endif

void Boss::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
   // 基底クラスの初期化を呼び出す
   GameObject::Initialize(std::move(model), texture);

   // ステートマシンの初期化
   InitializeStateMachine();

   // 初期状態をNormalに設定
   stateMachine_->RequestState("Normal", 0);

   // コライダーの初期化
   InitializeCollider();

   transform_.translate = { -5.0f, -5.0f, 0.0f };
}

void Boss::Update() {
   // ステートマシンの更新
   stateMachine_->Update();

   // ビヘイビアツリーの実行（Normal状態のみ）
   if (stateMachine_->GetCurrentState() == "Normal" && behaviorTree_) {
      behaviorTree_->Tick();
   }

   UpdateRotation();

   UpdateMovement();

   transform_.TransferMatrix();
}

void Boss::Draw(const ICamera* camera) {
   if (!model_ || !camera) {
      return;
   }

   // モデルの描画
   model_->Draw(transform_, camera, texture_.gpuHandle);
}

bool Boss::DrawImGui() {
   return false;
}

void Boss::OnCollisionEnter(GameObject* other) {
   // プレイヤーと衝突したら反発する
   if (auto p = dynamic_cast<Player*>(other)) {
      Vector3 toOther = p->GetWorldPosition() - GetWorldPosition();
      Vector2 normal = Vector2{ toOther.x, toOther.y }.Normalize();

      Vector2 relativeVel = velocity_ - p->GetVelocity();
      float speed = relativeVel.Length();
      float response = stunPower_ + speed * collisionResponseScale_;

      // 突進中かつプレイヤーに向かって突進している場合は反発を弱める
      if (isCharging_) {
         Vector2 chargeDir = direction_.Normalize();
         if (chargeDir.Length() > 0.0f) {
            float dot = chargeDir.x * normal.x + chargeDir.y * normal.y;
            if (dot > 0.0f) {
               response *= 0.1f;
            }
         }
      }

      response = (std::min)(response, maxCollisionResponse_);

      acceleration_ -= normal * response;

      // 速度反射
      float dot = velocity_.x * normal.x + velocity_.y * normal.y;
      velocity_ = velocity_ - normal * (dot * 1.5f);
      velocity_ *= 0.5f;

      // プレイヤーに突進されて吹き飛ばされた場合は中心バイアスを強める
      if (p->IsCharging()) {
         Vector2 playerDir = (p->GetVelocity().Length() > 0.0f) ? p->GetVelocity().Normalize() : Vector2{0.0f,0.0f};
         Vector2 towardBoss = Vector2{ GetWorldPosition().x - p->GetWorldPosition().x, GetWorldPosition().y - p->GetWorldPosition().y }.Normalize();
         float dotPB = playerDir.x * towardBoss.x + playerDir.y * towardBoss.y;
         if (dotPB > 0.7f) {
            StartKnockbackBias(2.0f, 0.5f);
         }
      }

      // スタン状態に遷移
      stateMachine_->RequestState("Stun", 0);
   }
}

void Boss::OnCollisionStay(GameObject* other) {
   if (auto p = dynamic_cast<Player*>(other)) {
      Vector3 toOther = p->GetWorldPosition() - GetWorldPosition();
      Vector2 normal = Vector2{ toOther.x, toOther.y }.Normalize();

      Vector2 relativeVel = velocity_ - p->GetVelocity();
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

      response = (std::min)(response, maxCollisionResponse_);

      acceleration_ -= normal * response;

      // スタン状態に遷移
      stateMachine_->RequestState("Stun", 0);
   }
}

void Boss::OnCollisionExit(GameObject* other) {
   (void)other;
}

void Boss::SetBehaviorTree(std::unique_ptr<BehaviorTree> tree) {
   behaviorTree_ = std::move(tree);
}

void Boss::InitializeCollider() {
   AttachCollider(std::make_unique<SphereCollider>(this, 1.2f));
   collider_->SetLayer(CollisionLayer::Boss);
}

void Boss::InitializeStateMachine() {
   // ステートマシンの取り付け
   GameObject::AttachStateMachine();

   // 通常状態
   stateMachine_->AddState("Normal", 
      std::bind(&Boss::InitializeNormal, this), 
      std::bind(&Boss::Normal, this));

   // スタン状態
   stateMachine_->AddState("Stun", 
      std::bind(&Boss::InitializeStun, this), 
      std::bind(&Boss::Stun, this));

   // 状態遷移ルール
   stateMachine_->AddTransitionRule("Normal", { "Stun" });
   stateMachine_->AddTransitionRule("Stun", { "Normal" });
}

void Boss::UpdateMovement() {
   // knockback bias timer update
   knockbackBiasTimer_.Update(GameUtils::GetDeltaTime());
   if (!knockbackBiasTimer_.IsFinished()) {
      // timer running - keep multiplier
   } else {
      // expired -> reset
      knockbackBiasMultiplier_ = 1.0f;
   }

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
}

void Boss::StartKnockbackBias(float duration, float multiplier) {
   knockbackBiasMultiplier_ = multiplier;
   knockbackBiasTimer_.Start(duration, false);
}

void Boss::UpdateRotation() {
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

void Boss::InitializeNormal() {
   // 通常の減衰率と速度に戻す
   dampingPerSecond_ = 0.8f;
   maxSpeed_ = 20.0f;

   // 突進フラグをリセット
   isCharging_ = false;
}

void Boss::Normal() {
   // 通常状態では特に処理なし
   // ビヘイビアツリーがUpdate()で実行される
}

void Boss::InitializeStun() {
   // スタン用の減衰率と速度を設定
   dampingPerSecond_ = stunDamping_;
   maxSpeed_ = stunMaxSpeed_;

   // スタンタイマーを開始
   stunTimer_.Start(stunDuration_, false);

   // モデルを変更（ダメージ表現）
   //GameObject::ChangeModelResource("Resources/Models/Player/Damage/PlayerDamage.obj");

   // 突進フラグをリセット
   isCharging_ = false;
}

void Boss::Stun() {
   stunTimer_.Update(GameUtils::GetDeltaTime());
   
   if (stunTimer_.IsFinished()) {
      // スタン終了、通常状態に戻る
      stateMachine_->RequestState("Normal", 0);
   }
}
