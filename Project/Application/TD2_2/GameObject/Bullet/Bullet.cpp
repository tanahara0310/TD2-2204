#include "Bullet.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Application/TD2_2/Collider/SphereCollider.h"
#include <cmath>

void Bullet::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture, const Vector3& direction) {
   // 基底クラスの初期化
   GameObject::Initialize(std::move(model), texture);

   // 方向ベクトルを正規化して速度ベクトルを設定
   float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
   if (length > 0.0f) {
	  velocity_ = {
		 (direction.x / length) * speed_,
		 (direction.y / length) * speed_,
		 (direction.z / length) * speed_
	  };
   } else {
	  // 方向ベクトルが0の場合はデフォルトで前方に飛ぶ
	  velocity_ = { 0.0f, 0.0f, speed_ };
   }

   // コライダーの初期化
   InitializeCollider();

   // デフォルトの生存時間を設定（5秒）
   SetLifetime(5.0f);
}

void Bullet::Update() {
   if (!isActive_) {
	  return;
   }

   // デルタタイムを取得
   float deltaTime = GameUtils::GetDeltaTime();

   // 位置を更新
   transform_.translate.x += velocity_.x * deltaTime;
   transform_.translate.y += velocity_.y * deltaTime;

   // 生存時間の更新
   if (lifetimeTimer_.IsActive()) {
	  lifetimeTimer_.Update(deltaTime);
	  if (lifetimeTimer_.IsFinished()) {
		 isActive_ = false;
	  }
   }

   // トランスフォームの更新
   transform_.TransferMatrix();
}

void Bullet::SetLifetime(float lifetime) {
   lifetimeTimer_.Start(lifetime, false);
}

void Bullet::InitializeCollider() {
   // 球形コライダーを作成（半径は0.5）
   auto sphereCollider = std::make_unique<SphereCollider>(this, 0.5f);
   sphereCollider->SetLayer(CollisionLayer::BossBullet);
   AttachCollider(std::move(sphereCollider));
}
