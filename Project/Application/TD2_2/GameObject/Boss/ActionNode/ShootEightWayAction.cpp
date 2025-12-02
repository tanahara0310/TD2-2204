#include "ShootEightWayAction.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Math/MathCore.h"
#include <numbers>

ShootEightWayAction::ShootEightWayAction(Boss* boss, BulletSpawnFunction bulletSpawnFunc, float offsetRadius, float shootInterval)
   : BossActionNode(boss, "ShootEightWayAction")
   , bulletSpawnFunc_(bulletSpawnFunc)
   , offsetRadius_(offsetRadius)
   , shootInterval_(shootInterval)
   , currentBulletIndex_(0)
   , hasStartedShooting_(false) {
}

void ShootEightWayAction::Reset() {
   BossActionNode::Reset();
   shootTimer_.Reset();
   currentBulletIndex_ = 0;
   hasStartedShooting_ = false;
}

void ShootEightWayAction::OnEnter() {
   currentBulletIndex_ = 0;
   hasStartedShooting_ = false;
   
   // 間隔発射の場合はタイマーを設定
   if (shootInterval_ > 0.0f) {
      shootTimer_.Start(shootInterval_, false);
   }
}

NodeState ShootEightWayAction::OnExecute() {
   // 一斉発射の場合
   if (shootInterval_ <= 0.0f) {
      if (!hasStartedShooting_) {
         ShootBullets();
         hasStartedShooting_ = true;
      }
      return NodeState::Success;
   }
   
   // タイマー更新
   shootTimer_.Update(GameUtils::GetDeltaTime());
   
   // 連続発射の場合
   if (shootTimer_.IsFinished()) {
      // 1つ弾を発射
      ShootSingleBullet(currentBulletIndex_);
      currentBulletIndex_++;
      
      // 全ての弾を発射したら完了
      if (currentBulletIndex_ >= BULLET_COUNT) {
         return NodeState::Success;
      }
      
      // 次の発射のためにタイマーをリセット
      shootTimer_.Reset();
      shootTimer_.Start(shootInterval_, false);
   }
   
   return NodeState::Running;
}

void ShootEightWayAction::OnExit() {
   // 特に処理なし
}

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
   
   // 弾を生成
   bulletSpawnFunc_(spawnPosition, direction);
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
   
   // XZ平面での方向ベクトル（Y=0）
   float x = std::cos(angleInRadians);
   float z = std::sin(angleInRadians);
   
   Vector3 direction = { x, 0.0f, z };
   return MathCore::Vector::Normalize(direction);
}
