#include "FleeFromPlayerAction.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/GameObject/Player/Player.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Math/MathCore.h"
#include <algorithm>
#include <cmath>

// Windowsのmin/maxマクロとの競合を回避
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

FleeFromPlayerAction::FleeFromPlayerAction(Boss* boss, Player* player, float moveSpeed, float avoidanceStrength)
   : BossActionNode(boss, "FleeFromPlayerAction")
   , player_(player)
   , moveSpeed_(moveSpeed)
   , duration_(GameUtils::RandomFloat(2.0f, 4.0f))
   , avoidanceStrength_(avoidanceStrength)
   , currentDirection_({ 0.0f, 0.0f }) {}

void FleeFromPlayerAction::Reset() {
   BossActionNode::Reset();
   moveTimer_.Reset();
   currentDirection_ = { 0.0f, 0.0f };
}

void FleeFromPlayerAction::OnEnter() {
   duration_ = GameUtils::RandomFloat(2.0f, 4.0f);

   // タイマー開始
   moveTimer_.Start(duration_, false);

   // 初期方向を中央に設定
   Vector3 bossPos = boss_->GetWorldPosition();
   Vector2 bossPos2D = { bossPos.x, bossPos.y };
   Vector2 centerPos = { 0.0f, 0.0f };
   Vector2 toCenter = centerPos - bossPos2D;
   if (toCenter.Length() > 0.0f) {
	  currentDirection_ = toCenter.Normalize();
   }

   if (boss_) {
	  boss_->SetMaxSpeed(5.0f);
	  boss_->SetDampingPerSecond(0.7f);  // 減衰を弱める（0.3f → 0.7f）
	  boss_->StartEffect();
   }
}

NodeState FleeFromPlayerAction::OnExecute() {
   // 中央に到達したか確認（停止距離: 2.0f）
   float distanceToCenter = CalculateDistanceToCenter();
   if (distanceToCenter <= 2.0f) {
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

void FleeFromPlayerAction::OnExit() {
   // 加速度をリセット
   boss_->SetAcceleration({ 0.0f, 0.0f });
   currentDirection_ = { 0.0f, 0.0f };
}

void FleeFromPlayerAction::SetupStateMachine() {
   BossActionNode::SetupStateMachine();
}

Vector2 FleeFromPlayerAction::CalculateMoveDirection() {
   Vector3 bossPos = boss_->GetWorldPosition();
   Vector3 playerPos = player_->GetWorldPosition();

   Vector2 boss2D = { bossPos.x, bossPos.y };
   Vector2 player2D = { playerPos.x, playerPos.y };
   Vector2 centerPos = { 0.0f, 0.0f };

   Vector2 toCenter = (centerPos - boss2D).Normalize();
   Vector2 toPlayer = player2D - boss2D;

   float distanceToPlayer = toPlayer.Length();
   Vector2 toPlayerDir = (distanceToPlayer > 0.0f) ? toPlayer.Normalize() : Vector2(0, 0);

   float idealAngle = std::atan2(toCenter.y, toCenter.x);
   float currentAngle = std::atan2(currentDirection_.y, currentDirection_.x);

   //-------------------------------------
   // 回避角度を算出
   //-------------------------------------
   float avoidanceAngle = 0.0f;
   const float maxAvoidAngle = 45.0f / 180.0f * DirectX::XM_PI;  // 45度に修正

   if (distanceToPlayer < 8.0f) {
	  // 経路前方にいる？
	  float dot = toCenter.Dot(toPlayerDir);

	  if (dot > 0.2f) {
		 float cross = toCenter.x * toPlayerDir.y - toCenter.y * toPlayerDir.x;
		 float sign = (cross > 0.0f) ? -1.0f : 1.0f;

		 float distFactor = std::clamp((5.0f - distanceToPlayer) / 5.0f, 0.0f, 1.0f);

		 avoidanceAngle = sign * distFactor * avoidanceStrength_ * maxAvoidAngle;
	  }
   }

   float targetAngle = idealAngle + avoidanceAngle;

   //-------------------------------------
   // 角度を補間
   //-------------------------------------
   // プレイヤーがいない場合は素早く理想角度に戻す
   float lerpSpeed = (distanceToPlayer < 5.0f && std::abs(avoidanceAngle) > 0.01f) ? 0.12f : 0.25f;

   float angleDiff = targetAngle - currentAngle;

   // 角度差を -π ~ π の範囲に正規化
   while (angleDiff > DirectX::XM_PI) angleDiff -= 2.0f * DirectX::XM_PI;
   while (angleDiff < -DirectX::XM_PI) angleDiff += 2.0f * DirectX::XM_PI;

   float newAngle = currentAngle + angleDiff * lerpSpeed;

   currentDirection_ = Vector2(std::cos(newAngle), std::sin(newAngle));

   return currentDirection_;
}

float FleeFromPlayerAction::CalculateDistanceToPlayer() const {
   Vector3 bossPos = boss_->GetWorldPosition();
   Vector3 playerPos = player_->GetWorldPosition();

   Vector2 bossPos2D = { bossPos.x, bossPos.y };
   Vector2 playerPos2D = { playerPos.x, playerPos.y };

   Vector2 toPlayer = playerPos2D - bossPos2D;
   return toPlayer.Length();
}

float FleeFromPlayerAction::CalculateDistanceToCenter() const {
   Vector3 bossPos = boss_->GetWorldPosition();
   Vector2 bossPos2D = { bossPos.x, bossPos.y };
   Vector2 centerPos = { 0.0f, 0.0f };

   Vector2 toCenter = centerPos - bossPos2D;
   return toCenter.Length();
}

void FleeFromPlayerAction::ExecuteMove() {
   // タイマー更新
   moveTimer_.Update(GameUtils::GetDeltaTime());

   // プレイヤーを避けながら中央に向かう方向を計算
   Vector2 direction = CalculateMoveDirection();

   // 加速度を設定
   Vector2 acceleration = direction * moveSpeed_;
   boss_->SetAcceleration(acceleration);

   // ボスの向きを設定
   boss_->SetDirection(direction);
}
