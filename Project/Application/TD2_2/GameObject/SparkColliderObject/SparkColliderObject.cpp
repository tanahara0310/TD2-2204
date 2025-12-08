#include "SparkColliderObject.h"
#include "Application/TD2_2/Collider/SphereCollider.h"
#include "Application/TD2_2/Collider/CollisionLayer.h"

void SparkColliderObject::Initialize(float radius) {
   // コライダーの初期化（Sparkレイヤー）
   AttachCollider(std::make_unique<SphereCollider>(this, radius));
   collider_->SetLayer(CollisionLayer::Spark);

   auto engine = GetEngineSystem();
   auto dxCommon = engine->GetComponent<DirectXCommon>();

   transform_.Initialize(dxCommon->GetDevice());

   transform_.translate = { 0.0f, 0.0f, -1000.0f }; // 初期位置を遠ざけておく
}

/// @brief 更新処理
void SparkColliderObject::Update() {
   transform_.TransferMatrix();
}
