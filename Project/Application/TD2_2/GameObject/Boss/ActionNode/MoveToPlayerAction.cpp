#include "MoveToPlayerAction.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/GameObject/Player/Player.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Math/MathCore.h"

MoveToPlayerAction::MoveToPlayerAction(Boss* boss, Player* player, float moveSpeed, float duration, float stopDistance)
   : BossActionNode(boss, "MoveToPlayerAction")
   , player_(player)
   , moveSpeed_(moveSpeed)
   , duration_(duration)
   , stopDistance_(stopDistance) {
}

void MoveToPlayerAction::Reset() {
   BossActionNode::Reset();
   moveTimer_.Reset();
}

void MoveToPlayerAction::OnEnter() {
   // タイマー開始
   moveTimer_.Start(duration_, false);
}

NodeState MoveToPlayerAction::OnExecute() {
   // プレイヤーからの距離を確認
   float distanceToPlayer = CalculateDistanceToPlayer();

   // 停止距離に到達したら成功
   if (distanceToPlayer <= stopDistance_) {
      return NodeState::Success;
   }

   // タイマーが終了したら完了
   if (moveTimer_.IsFinished()) {
      return NodeState::Success;
   }

   // 移動処理を実行
   ExecuteMove();

   return NodeState::Running;
}

void MoveToPlayerAction::OnExit() {
   // 加速度をリセット
   boss_->SetAcceleration({ 0.0f, 0.0f });
}

void MoveToPlayerAction::SetupStateMachine() {
   BossActionNode::SetupStateMachine();
}

Vector2 MoveToPlayerAction::CalculateDirectionToPlayer() const {
   Vector3 bossPos = boss_->GetWorldPosition();
   Vector3 playerPos = player_->GetWorldPosition();
   
   Vector2 bossPos2D = { bossPos.x, bossPos.z };
   Vector2 playerPos2D = { playerPos.x, playerPos.z };
   
   Vector2 direction = playerPos2D - bossPos2D;
   if (direction.Length() > 0.0f) {
      direction = direction.Normalize();
   }
   
   return direction;
}

float MoveToPlayerAction::CalculateDistanceToPlayer() const {
   Vector3 bossPos = boss_->GetWorldPosition();
   Vector3 playerPos = player_->GetWorldPosition();
   
   Vector2 bossPos2D = { bossPos.x, bossPos.z };
   Vector2 playerPos2D = { playerPos.x, playerPos.z };
   
   Vector2 toPlayer = playerPos2D - bossPos2D;
   return toPlayer.Length();
}

void MoveToPlayerAction::ExecuteMove() {
   // タイマー更新
   moveTimer_.Update(GameUtils::GetDeltaTime());
   
   // プレイヤーへの方向を計算
   Vector2 direction = CalculateDirectionToPlayer();
   
   // 加速度を設定
   Vector2 acceleration = direction * moveSpeed_;
   boss_->SetAcceleration(acceleration);
}
