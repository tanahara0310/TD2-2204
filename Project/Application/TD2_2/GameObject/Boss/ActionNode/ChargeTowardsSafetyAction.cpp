#include "ChargeTowardsSafetyAction.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/GameObject/Player/Player.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Application/TD2_2/Scene/Config/GameSceneConfig.h"
#include <cmath>
#include <iostream>
#include "MoveAction.h"

ChargeTowardsSafetyAction::ChargeTowardsSafetyAction(Boss* boss, Player* player,
                                                     float chargeSpeed, float chargeDuration)
   : BossActionNode(boss, "ChargeTowardsSafety"),
   player_(player),
   chargeSpeed_(chargeSpeed),
   chargeDuration_(chargeDuration),
   chargeMaxSpeed_(45.0f),
   chargeDamping_(0.02f),
   chargeDirection_{0.0f, 0.0f, 0.0f}
{
}

void ChargeTowardsSafetyAction::Reset() {
   BossActionNode::Reset();
   chargeDirection_ = {0.0f, 0.0f, 0.0f};
   chargeTimer_.Reset();
}

void ChargeTowardsSafetyAction::OnEnter() {
   
   // 安全な方向を計算
   chargeDirection_ = CalculateSafeDirection();
   // 突進タイマーを開始
   chargeTimer_.Start(chargeDuration_, false);
   
   if (boss_) {
      // 突進中の最大速度と減衰率を設定
      boss_->SetMaxSpeed(chargeMaxSpeed_);
      boss_->SetDampingPerSecond(chargeDamping_);
      boss_->SetDirection({ chargeDirection_.x, chargeDirection_.y });
      boss_->AddAcceleration({ chargeDirection_.x * chargeSpeed_, chargeDirection_.y * chargeSpeed_ });
      boss_->SetVelocity({ 0.0f, 0.0f });

      boss_->StartRotateAroundAxis(chargeDuration_, 2.0f);

      boss_->SetIsCharging(true);

      boss_->StartChargeFunction();
   }
   
}

NodeState ChargeTowardsSafetyAction::OnExecute() {
   // 突進フェーズ
   if (!chargeTimer_.IsFinished()) {
      ExecuteCharge();
      return BossActionHelper::Running();
   }
   
   // 突進完了
   return BossActionHelper::Success();
}

void ChargeTowardsSafetyAction::OnExit() {
   if (boss_) {
      boss_->SetIsCharging(false);
   }
}

void ChargeTowardsSafetyAction::SetupStateMachine() {
   // 基底クラスのステートマシンをセットアップ
   BossActionNode::SetupStateMachine();
}

Vector3 ChargeTowardsSafetyAction::CalculateSafeDirection() const {
   if (!boss_ || !player_) {
      return {0.0f, 0.0f, 0.0f};
   }

   Vector3 bossPos = boss_->GetWorldPosition();
   Vector3 playerPos = player_->GetWorldPosition();
   Vector3 center = {0.0f, 0.0f, 0.0f};

   // ボスから中央への方向
   Vector3 toCenter = {
      center.x - bossPos.x,
      center.y - bossPos.y,
      0.0f
   };

   // ボスからプレイヤーへの方向
   Vector3 toPlayer = {
      playerPos.x - bossPos.x,
      playerPos.y - bossPos.y,
      0.0f
   };

   // 中央方向を正規化
   Vector3 toCenterNorm = MathCore::Vector::Normalize(toCenter);
   Vector3 toPlayerNorm = MathCore::Vector::Normalize(toPlayer);

   // プレイヤーと反対方向のベクトル
   Vector3 awayFromPlayer = {
      -toPlayerNorm.x,
      -toPlayerNorm.y,
      0.0f
   };

   // 壁からの距離を計算
   float halfWidth = (GameSceneConfig::kMoveableAreaSize.x * 0.5f) - GameSceneConfig::kFrameSize.x;
   float halfHeight = (GameSceneConfig::kMoveableAreaSize.y * 0.5f) - GameSceneConfig::kFrameSize.y;

   float distToWallX = halfWidth - std::abs(bossPos.x);
   float distToWallY = halfHeight - std::abs(bossPos.y);

   // 壁に近い場合（危険）は中央方向を優先、そうでない場合はプレイヤーと反対方向を優先
   Vector3 safeDirection;
   if (distToWallX < 5.0f || distToWallY < 5.0f) {
      // 壁に非常に近い → 中央方向80%、プレイヤー逆20%
      safeDirection = {
         toCenterNorm.x * 0.8f + awayFromPlayer.x * 0.2f,
         toCenterNorm.y * 0.8f + awayFromPlayer.y * 0.2f,
         0.0f
      };
   } else {
      // まだ余裕がある → 中央方向50%、プレイヤー逆50%
      safeDirection = {
         toCenterNorm.x * 0.5f + awayFromPlayer.x * 0.5f,
         toCenterNorm.y * 0.5f + awayFromPlayer.y * 0.5f,
         0.0f
      };
   }

   Vector3 normalizedDir = MathCore::Vector::Normalize(safeDirection);
   return normalizedDir;
}

void ChargeTowardsSafetyAction::ExecuteCharge() {
   chargeTimer_.Update(GameUtils::GetDeltaTime());
}
