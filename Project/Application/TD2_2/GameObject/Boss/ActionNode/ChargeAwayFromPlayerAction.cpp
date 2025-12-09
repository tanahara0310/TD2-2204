#include "ChargeAwayFromPlayerAction.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/GameObject/Player/Player.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include <cmath>
#include <iostream>
#include "MoveAction.h"

ChargeAwayFromPlayerAction::ChargeAwayFromPlayerAction(Boss* boss, Player* player,
                                                       float chargeSpeed, float chargeDuration)
   : BossActionNode(boss, "ChargeAwayFromPlayer"),
   player_(player),
   chargeSpeed_(chargeSpeed),
   chargeDuration_(chargeDuration),
   chargeMaxSpeed_(45.0f),
   chargeDamping_(0.02f),
   chargeDirection_{0.0f, 0.0f, 0.0f}
{
}

void ChargeAwayFromPlayerAction::Reset() {
   BossActionNode::Reset();
   chargeDirection_ = {0.0f, 0.0f, 0.0f};
   chargeTimer_.Reset();
}

void ChargeAwayFromPlayerAction::OnEnter() {
   
   // プレイヤーから離れる方向を計算
   chargeDirection_ = CalculateDirectionAwayFromPlayer();
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

NodeState ChargeAwayFromPlayerAction::OnExecute() {
   // 突進フェーズ
   if (!chargeTimer_.IsFinished()) {
      ExecuteCharge();
      return BossActionHelper::Running();
   }
   
   // 突進完了
   return BossActionHelper::Success();
}

void ChargeAwayFromPlayerAction::OnExit() {
   if (boss_) {
      boss_->SetIsCharging(false);
   }
}

void ChargeAwayFromPlayerAction::SetupStateMachine() {
   // 基底クラスのステートマシンをセットアップ
   BossActionNode::SetupStateMachine();
}

Vector3 ChargeAwayFromPlayerAction::CalculateDirectionAwayFromPlayer() const {
   if (!boss_ || !player_) {
      return {0.0f, 0.0f, 0.0f};
   }

   Vector3 bossPos = boss_->GetWorldPosition();
   Vector3 playerPos = player_->GetWorldPosition();
   // プレイヤーから離れる方向 = ボスから見てプレイヤーと反対方向
   Vector3 direction = {
      bossPos.x - playerPos.x,  // 反転
      bossPos.y - playerPos.y,  // 反転
      0.0f // Z軸は無視
   };

   Vector3 normalizedDir = MathCore::Vector::Normalize(direction);
   return normalizedDir;
}

void ChargeAwayFromPlayerAction::ExecuteCharge() {
   chargeTimer_.Update(GameUtils::GetDeltaTime());
}
