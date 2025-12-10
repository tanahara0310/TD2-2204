#include "ChargeAwayFromPlayerAction.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/GameObject/Player/Player.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Application/TD2_2/Scene/Config/GameSceneConfig.h"
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
   if (!boss_ || !player_) {
      return;
   }

   // 安全チェック: ボスがプレイヤーより内側にいるか確認
   Vector3 bossPos = boss_->GetWorldPosition();
   Vector3 playerPos = player_->GetWorldPosition();
   Vector3 center = {0.0f, 0.0f, 0.0f};

   // 中心からの距離を比較
   float bossDistFromCenter = MathCore::Vector::Length(bossPos - center);
   float playerDistFromCenter = MathCore::Vector::Length(playerPos - center);

   // ボスがプレイヤーより外側にいる場合は実行しない（自滅防止）
   if (bossDistFromCenter >= playerDistFromCenter) {
      // 失敗を設定して即座に終了
      return;
   }

   // 壁との距離チェック（閾値を緩和: 10.0f → 6.0f）
   float halfWidth = (GameSceneConfig::kMoveableAreaSize.x * 0.5f) - GameSceneConfig::kFrameSize.x - 3.0f;
   float halfHeight = (GameSceneConfig::kMoveableAreaSize.y * 0.5f) - GameSceneConfig::kFrameSize.y - 3.0f;
   float distToWallX = halfWidth - std::abs(bossPos.x);
   float distToWallY = halfHeight - std::abs(bossPos.y);

   // 壁に非常に近い場合のみ実行しない（自滅防止）
   if (distToWallX < 6.0f || distToWallY < 6.0f) {
      return;
   }

   // プレイヤーから離れる方向を計算
   chargeDirection_ = CalculateDirectionAwayFromPlayer();

   // 離れる方向が壁に向かっている場合も確認（閾値を緩和: 5.0f → 3.0f）
   Vector3 futurePos = {
      bossPos.x + chargeDirection_.x * 5.0f,
      bossPos.y + chargeDirection_.y * 5.0f,
      0.0f
   };

   float futureDistX = halfWidth - std::abs(futurePos.x);
   float futureDistY = halfHeight - std::abs(futurePos.y);

   // 進行方向が壁に非常に近づく場合のみ実行しない
   if (futureDistX < 3.0f || futureDistY < 3.0f) {
      return;
   }

   // 安全確認完了、突進開始
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
   // タイマーが開始されていない場合は失敗（安全チェックで弾かれた）
   if (chargeTimer_.GetElapsedTime() == 0.0f && chargeTimer_.IsFinished()) {
      return BossActionHelper::Failure();
   }

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
