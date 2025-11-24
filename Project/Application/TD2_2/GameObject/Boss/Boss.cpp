#include "Boss.h"

void Boss::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture) {
   // 基底クラスの初期化を呼び出す
   GameObject::Initialize(std::move(model), texture);

   // コライダーの初期化
   InitializeCollider();

   transform_.translate = { -5.0f, -5.0f, 0.0f };
}

void Boss::Update() {
  // Move();
   UpdateMovement();
}

void Boss::Draw(const ICamera* camera) {
   if (!model_ || !camera) {
	  return;
   }

   // モデルの描画
   model_->Draw(transform_, camera, texture_.gpuHandle);
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
