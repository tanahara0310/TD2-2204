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
   
   // 安全な方向が見つからない場合は失敗
   if (chargeDirection_.x == 0.0f && chargeDirection_.y == 0.0f) {
      return;
   }
   
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
   // タイマーが開始されていない場合は失敗
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

   // 壁の範囲を計算
   float halfWidth = (GameSceneConfig::kMoveableAreaSize.x * 0.5f) - GameSceneConfig::kFrameSize.x - 3.0f;
   float halfHeight = (GameSceneConfig::kMoveableAreaSize.y * 0.5f) - GameSceneConfig::kFrameSize.y - 3.0f;

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

   // 各方向の候補を評価
   struct DirectionCandidate {
      Vector3 direction;
      float score;
   };

   std::vector<DirectionCandidate> candidates;

   // 候補1: 中央方向
   candidates.push_back({toCenterNorm, 1.0f});

   // 候補2: プレイヤーと反対方向
   candidates.push_back({awayFromPlayer, 0.8f});

   // 候補3: 中央方向 + プレイヤー逆方向のブレンド
   Vector3 blend1 = {
      toCenterNorm.x * 0.7f + awayFromPlayer.x * 0.3f,
      toCenterNorm.y * 0.7f + awayFromPlayer.y * 0.3f,
      0.0f
   };
   candidates.push_back({MathCore::Vector::Normalize(blend1), 0.9f});

   // 候補4: 中央方向 + プレイヤー逆方向のブレンド（逆比率）
   Vector3 blend2 = {
      toCenterNorm.x * 0.3f + awayFromPlayer.x * 0.7f,
      toCenterNorm.y * 0.3f + awayFromPlayer.y * 0.7f,
      0.0f
   };
   candidates.push_back({MathCore::Vector::Normalize(blend2), 0.85f});

   // 各候補を評価して、壁に近づかない方向を選ぶ
   float bestScore = -1.0f;
   Vector3 bestDirection = {0.0f, 0.0f, 0.0f};

   for (const auto& candidate : candidates) {
      // この方向に進んだ場合の予測位置
      float testDistance = 8.0f; // テスト用の距離
      Vector3 futurePos = {
         bossPos.x + candidate.direction.x * testDistance,
         bossPos.y + candidate.direction.y * testDistance,
         0.0f
      };

      // 壁までの距離を計算
      float futureDistLeft = futurePos.x - (-halfWidth);
      float futureDistRight = halfWidth - futurePos.x;
      float futureDistBottom = futurePos.y - (-halfHeight);
      float futureDistTop = halfHeight - futurePos.y;

      // 最小距離を計算
      float minFutureDist = futureDistLeft;
      if (futureDistRight < minFutureDist) minFutureDist = futureDistRight;
      if (futureDistBottom < minFutureDist) minFutureDist = futureDistBottom;
      if (futureDistTop < minFutureDist) minFutureDist = futureDistTop;

      // 壁に近づきすぎる場合はスキップ
      if (minFutureDist < 3.0f) {
         continue;
      }

      // スコアを計算（壁から遠いほど高スコア + 基礎スコア）
      float distScore = std::clamp(minFutureDist / 10.0f, 0.0f, 1.0f);
      float totalScore = candidate.score * distScore;

      if (totalScore > bestScore) {
         bestScore = totalScore;
         bestDirection = candidate.direction;
      }
   }

   // 有効な方向が見つからない場合
   if (bestScore < 0.0f) {
      return {0.0f, 0.0f, 0.0f};
   }

   return bestDirection;
}

void ChargeTowardsSafetyAction::ExecuteCharge() {
   chargeTimer_.Update(GameUtils::GetDeltaTime());
}
