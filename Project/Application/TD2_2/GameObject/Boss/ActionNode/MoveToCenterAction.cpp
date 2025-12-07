#include "MoveToCenterAction.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Math/MathCore.h"

MoveToCenterAction::MoveToCenterAction(Boss* boss, float moveSpeed, float duration, float stopDistance)
   : BossActionNode(boss, "MoveToCenterAction")
   , moveSpeed_(moveSpeed)
   , duration_(duration)
   , stopDistance_(stopDistance) {
}

void MoveToCenterAction::Reset() {
   BossActionNode::Reset();
   moveTimer_.Reset();
}

void MoveToCenterAction::OnEnter() {
   // タイマー開始
   moveTimer_.Start(duration_, false);
}

NodeState MoveToCenterAction::OnExecute() {
   // 中心からの距離を確認
   Vector3 bossPos = boss_->GetWorldPosition();
   Vector2 bossPos2D = { bossPos.x, bossPos.y };
   Vector2 centerPos = { 0.0f, 0.0f };
   
   Vector2 toCenter = centerPos - bossPos2D;
   float distanceToCenter = toCenter.Length();

   // 停止距離に到達したら成功
   if (distanceToCenter <= stopDistance_) {
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

void MoveToCenterAction::OnExit() {
   // 加速度をリセット
   boss_->SetAcceleration({ 0.0f, 0.0f });
}

void MoveToCenterAction::SetupStateMachine() {
   BossActionNode::SetupStateMachine();
}

Vector2 MoveToCenterAction::CalculateDirectionToCenter() const {
   Vector3 bossPos = boss_->GetWorldPosition();
   Vector2 bossPos2D = { bossPos.x, bossPos.y };
   Vector2 centerPos = { 0.0f, 0.0f };
   
   Vector2 direction = centerPos - bossPos2D;
   if (direction.Length() > 0.0f) {
      direction = direction.Normalize();
   }
   
   return direction;
}

void MoveToCenterAction::ExecuteMove() {
   // タイマー更新
   moveTimer_.Update(GameUtils::GetDeltaTime());
   
   // 中心への方向を計算
   Vector2 direction = CalculateDirectionToCenter();
   
   // 加速度を設定
   Vector2 acceleration = direction * moveSpeed_;
   boss_->SetAcceleration(acceleration);
}
