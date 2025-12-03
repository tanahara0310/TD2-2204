#include "Bullet.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Application/TD2_2/Collider/SphereCollider.h"
#include <cmath>

void Bullet::Initialize(std::unique_ptr<Model> model, TextureManager::LoadedTexture texture, const Vector3& direction) {
   // 基底クラスの初期化
   GameObject::Initialize(std::move(model), texture);

   transform_.scale = { initialScale_, initialScale_, initialScale_ }; // 最初は小さく始める

   transform_.TransferMatrix();

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

   // ステートマシンの初期化
   InitializeStateMachine();
   
   // 初期状態をScaleUpに設定
   stateMachine_->RequestState("ScaleUp", 0);

   // コライダーの初期化
   InitializeCollider();

   // デフォルトの生存時間を設定（5秒）
   SetLifetime(5.0f);

   isActive_ = true;
}

void Bullet::Update() {
   if (!isActive_) {
	  return;
   }

   stateMachine_->Update();

   // 生存時間の更新
   if (lifetimeTimer_.IsActive()) {
	  lifetimeTimer_.Update(GameUtils::GetDeltaTime());
	  if (lifetimeTimer_.IsFinished()) {
		 isActive_ = false;
	  }
   }

   CheckOutOfBounds();

   // トランスフォームの更新
   transform_.TransferMatrix();
}

void Bullet::Draw(const ICamera* camera) {
   if (!model_ || !camera) {
	  return;
   }

   // モデルの描画
   model_->Draw(transform_, camera, texture_.gpuHandle);
}


void Bullet::SetLifetime(float lifetime) {
   lifetimeTimer_.Start(lifetime, false);
}

void Bullet::InitializeCollider() {
   // 球形コライダーを作成（半径は0.5）
   // レイヤーはデフォルトで設定し、CreateBullet側で上書きする
   auto sphereCollider = std::make_unique<SphereCollider>(this, 0.5f);
   sphereCollider->SetLayer(CollisionLayer::Default);
   AttachCollider(std::move(sphereCollider));
}

void Bullet::InitializeStateMachine() {
   AttachStateMachine();

   stateMachine_->AddState("ScaleUp",
	  std::bind(&Bullet::InitializeScaleUpState, this),
	  std::bind(&Bullet::ScaleUp, this));

   stateMachine_->AddState("Move",
	  std::bind(&Bullet::InitializeMoveState, this),
	  std::bind(&Bullet::Move, this));

   // 状態遷移ルール
   stateMachine_->AddTransitionRule("ScaleUp", { "Move" });
   stateMachine_->AddTransitionRule("Move", {});
}

void Bullet::InitializeScaleUpState() {
   scaleUpTimer_.Start(0.5f, false);
}

void Bullet::ScaleUp() {
   scaleUpTimer_.Update(GameUtils::GetDeltaTime());

   float progress = scaleUpTimer_.GetEasedProgress(EasingUtil::Type::EaseOutCubic);
   float scaleValue = GameUtils::Lerp(initialScale_, 1.0f, progress);
   transform_.scale = { scaleValue, scaleValue, scaleValue };
   if (scaleUpTimer_.IsFinished()) {
	  stateMachine_->RequestState("Move", 0);
   }
}

void Bullet::InitializeMoveState() {

}

void Bullet::Move() {

   // 位置を更新
   transform_.translate.x += velocity_.x * GameUtils::GetDeltaTime();
   transform_.translate.y += velocity_.y * GameUtils::GetDeltaTime();
}

void Bullet::CheckOutOfBounds() {
   // 画面外に出たら非アクティブにする
   const float kOutOfBoundsMargin = 0.0f; // 余裕を持たせる
   if (std::abs(transform_.translate.x) > GameSceneConfig::kMoveableAreaSize.x * 0.5f + kOutOfBoundsMargin ||
	   std::abs(transform_.translate.y) > GameSceneConfig::kMoveableAreaSize.y * 0.5f + kOutOfBoundsMargin) {
	  isActive_ = false;
   }
}
