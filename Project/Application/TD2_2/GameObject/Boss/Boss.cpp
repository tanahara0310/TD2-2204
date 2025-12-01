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

   // コライダーの初期化
   InitializeCollider();

   transform_.translate = { -5.0f, -5.0f, 0.0f };
}

void Boss::Update() {
   // ビヘイビアツリーの実行
   if (behaviorTree_) {
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
            if (dot > 0.7f) {
               response *= 0.3f; // 例: 30% に低減
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
      // プレイヤーが突進中かどうか判定し、かつプレイヤーの突進方向がボスに向かっているなら発動
      if (p->IsCharging()) {
         Vector2 playerDir = (p->GetVelocity().Length() > 0.0f) ? p->GetVelocity().Normalize() : Vector2{0.0f,0.0f};
         Vector2 towardBoss = Vector2{ GetWorldPosition().x - p->GetWorldPosition().x, GetWorldPosition().y - p->GetWorldPosition().y }.Normalize();
         float dotPB = playerDir.x * towardBoss.x + playerDir.y * towardBoss.y;
         if (dotPB > 0.7f) {
            // 例: 2秒間、中心バイアスを 0.5 にする（通常 1.0 -> 小さいほど強いバイアス）
            StartKnockbackBias(2.0f, 0.5f);
         }
      }
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
            if (dot > 0.7f) {
               response *= 0.3f;
            }
         }
      }

      response = (std::min)(response, maxCollisionResponse_);

      acceleration_ -= normal * response;
   }
}

void Boss::OnCollisionExit(GameObject* other) {
   (void)other;
}

void Boss::SetBehaviorTree(std::unique_ptr<BehaviorTree> tree) {
   behaviorTree_ = std::move(tree);
}

void Boss::InitializeCollider() {
   AttachCollider(std::make_unique<SphereCollider>(this, 0.9f));
   collider_->SetLayer(CollisionLayer::Boss);
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
