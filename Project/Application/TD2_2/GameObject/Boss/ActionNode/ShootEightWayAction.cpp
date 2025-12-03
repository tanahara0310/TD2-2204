#include "ShootEightWayAction.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Math/MathCore.h"
#include <numbers>

ShootEightWayAction::ShootEightWayAction(Boss* boss, BulletSpawnFunction bulletSpawnFunc, float offsetRadius, float shootInterval, float bulletSpeed)
   : BossActionNode(boss, "ShootEightWayAction")
   , bulletSpawnFunc_(bulletSpawnFunc)
   , offsetRadius_(offsetRadius)
   , shootInterval_(shootInterval)
   , bulletSpeed_(bulletSpeed)
   , currentBulletIndex_(0)
   , hasStartedShooting_(false) {}

void ShootEightWayAction::Reset() {
   BossActionNode::Reset();

   hasStartedShooting_ = false;
}

void ShootEightWayAction::OnEnter() {
   hasStartedShooting_ = false;
   shootTimer_.Start(shootInterval_, false);

   if (boss_) {
	  // ボスの動きを一時停止させる場合はここで設定
	  boss_->SetVelocity({ 0.0f, 0.0f });
	  boss_->SetAcceleration({ 0.0f, 0.0f });
   }
}

NodeState ShootEightWayAction::OnExecute() {
   shootTimer_.Update(GameUtils::GetDeltaTime());

   if (hasStartedShooting_) {
	  if (shootTimer_.IsFinished()) {
		 hasStartedShooting_ = false;
 		 shootTimer_.Reset();
		 return NodeState::Success;
	  }
   } else {
	  ShootBullets();
	  hasStartedShooting_ = true;
	  return NodeState::Running;
   }

   return NodeState::Running;
}

void ShootEightWayAction::OnExit() {}

void ShootEightWayAction::SetupStateMachine() {
   BossActionNode::SetupStateMachine();
}

void ShootEightWayAction::ShootBullets() {
   // 8方向全てに弾を発射
   for (int i = 0; i < BULLET_COUNT; ++i) {
	  ShootSingleBullet(i);
   }
}

void ShootEightWayAction::ShootSingleBullet(int index) {
   if (!bulletSpawnFunc_) {
	  return;
   }

   // ボスの位置を取得
   Vector3 bossPos = boss_->GetWorldPosition();

   // 方向ベクトルを取得
   Vector3 direction = GetDirectionForIndex(index);

   // オフセット位置を計算
   Vector3 spawnOffset = direction * offsetRadius_;
   Vector3 spawnPosition = bossPos + spawnOffset;

   // 弾を生成（速度パラメータも渡す）
   bulletSpawnFunc_(spawnPosition, direction, bulletSpeed_);
}

Vector3 ShootEightWayAction::GetDirectionForIndex(int index) const {
   // 8方向に均等に配置（45度ごと）
   // index 0: 右（0度）
   // index 1: 右下（45度）
   // index 2: 下（90度）
   // index 3: 左下（135度）
   // index 4: 左（180度）
   // index 5: 左上（225度）
   // index 6: 上（270度）
   // index 7: 右上（315度）

   float angleInDegrees = static_cast<float>(index) * 45.0f;
   float angleInRadians = angleInDegrees * std::numbers::pi_v<float> / 180.0f;

   // XY平面での方向ベクトル（Z=0）
   float x = std::cos(angleInRadians);
   float y = std::sin(angleInRadians);

   Vector3 direction = { x, y, 0.0f };
   return MathCore::Vector::Normalize(direction);
}
