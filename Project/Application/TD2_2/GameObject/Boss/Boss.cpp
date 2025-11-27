#include "Boss.h"
#include "Application/TD2_2/GameObject/Player/Player.h"
#include <cmath>

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
   
   // 移動処理
   UpdateMovement();
}

void Boss::Draw(const ICamera* camera) {
   if (!model_ || !camera) {
      return;
   }

   // モデルの描画
   model_->Draw(transform_, camera, texture_.gpuHandle);
}

bool Boss::DrawImGui() {
#ifdef _DEBUG
   bool changed = false;
   
   if (ImGui::TreeNode(GetObjectName())) {
      // アクティブ状態
      bool active = IsActive();
      if (ImGui::Checkbox("アクティブ", &active)) {
         SetActive(active);
         changed = true;
      }
      
      // 速度情報
      ImGui::Separator();
      ImGui::Text("速度: (%.2f, %.2f)", velocity_.x, velocity_.y);
      ImGui::Text("加速度: (%.2f, %.2f)", acceleration_.x, acceleration_.y);
      ImGui::Text("最大速度: %.2f", maxSpeed_);
      
      // ビヘイビアツリー情報
      if (behaviorTree_) {
         ImGui::Separator();
         ImGui::Text("ビヘイビアツリー: %s", behaviorTree_->GetName().c_str());
         ImGui::Text("実行回数: %u", behaviorTree_->GetTickCount());
      }
      
      // プレイヤー関連情報
      if (player_) {
         ImGui::Separator();
         ImGui::Text("プレイヤーへの距離: %.2f", GetDistanceToPlayer());
         ImGui::Text("プレイヤーへの角度: %.2f°", GetAngleToPlayer());
      }
      
      // 移動パラメータ
      if (ImGui::TreeNode("移動パラメータ")) {
         ImGui::DragFloat("移動速度", &moveSpeed_, 0.1f, 0.0f, 10.0f);
         ImGui::DragFloat("移動減衰率", &moveDamping_, 0.01f, 0.0f, 1.0f);
         ImGui::DragFloat("移動最大速度", &moveMaxSpeed_, 0.1f, 0.0f, 50.0f);
         ImGui::TreePop();
      }
      
      // 突進パラメータ
      if (ImGui::TreeNode("突進パラメータ")) {
         ImGui::DragFloat("突進速度", &chargeSpeed_, 100.0f, 0.0f, 100000.0f);
         ImGui::DragFloat("突進減衰率", &chargeDamping_, 0.001f, 0.0f, 1.0f);
         ImGui::DragFloat("突進持続時間", &chargeDuration_, 0.01f, 0.0f, 2.0f);
         ImGui::DragFloat("突進最大速度", &chargeMaxSpeed_, 0.1f, 0.0f, 100.0f);
         ImGui::TreePop();
      }
      
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

void Boss::OnCollisionEnter(GameObject* other) {
   (void)other;
}

void Boss::OnCollisionStay(GameObject* other) {
   (void)other;
}

void Boss::OnCollisionExit(GameObject* other) {
   (void)other;
}

void Boss::SetBehaviorTree(std::unique_ptr<BehaviorTree> tree) {
   behaviorTree_ = std::move(tree);
}

void Boss::AddAcceleration(const Vector2& accel) {
   acceleration_.x += accel.x;
   acceleration_.y += accel.y;
}

void Boss::SetVelocity(const Vector2& vel) {
   velocity_ = vel;
}

void Boss::SetMaxSpeed(float maxSpeed) {
   maxSpeed_ = maxSpeed;
}

void Boss::SetDamping(float damping) {
   dampingPerSecond_ = damping;
}

void Boss::ResetMovementParameters() {
   maxSpeed_ = 20.0f;
   dampingPerSecond_ = 0.8f;
}

float Boss::GetDistanceToPlayer() const {
   if (!player_) return 0.0f;
   
   Vector3 diff = player_->GetWorldPosition() - GetWorldPosition();
   return std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
}

Vector3 Boss::GetDirectionToPlayer() const {
   if (!player_) return {0.0f, 0.0f, 0.0f};
   
   Vector3 diff = player_->GetWorldPosition() - GetWorldPosition();
   float length = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
   
   if (length > 0.0001f) {
      return {diff.x / length, diff.y / length, diff.z / length};
   }
   
   return {0.0f, 0.0f, 0.0f};
}

float Boss::GetAngleToPlayer() const {
   if (!player_) return 0.0f;
   
   Vector3 direction = GetDirectionToPlayer();
   
   // XZ平面上の角度を計算（Y軸まわりの回転）
   float angle = std::atan2(direction.x, direction.z);
   
   // ラジアンから度数法に変換
   return angle * 180.0f / 3.14159265f;
}

void Boss::InitializeCollider() {
   AttachCollider(std::make_unique<SphereCollider>(this, 0.6f));
   collider_->SetLayer(CollisionLayer::Boss);
}

void Boss::UpdateMovement() {
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

void Boss::Move() {
   acceleration_ = Vector2(1.0f, 1.0f).Normalize() * moveSpeed_;
}
