#include "ChargeToPlayerAction.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/GameObject/Player/Player.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include <cmath>

ChargeToPlayerAction::ChargeToPlayerAction(Boss* boss, Player* player,
                                           float chargeSpeed, float chargeDuration)
   : BossActionNode(boss, "ChargeToPlayer"),
   player_(player),
   chargeSpeed_(chargeSpeed),
   chargeDuration_(chargeDuration),
   chargeMaxSpeed_(45.0f),
   chargeDamping_(0.02f),
   chargeDirection_{0.0f, 0.0f, 0.0f},
   isPreparationComplete_(false),
   preparationTime_(0.3f) {
}

void ChargeToPlayerAction::Reset() {
   BossActionNode::Reset();
   isPreparationComplete_ = false;
   chargeDirection_ = {0.0f, 0.0f, 0.0f};
   chargeTimer_.Reset();
   preparationTimer_.Reset();
}

void ChargeToPlayerAction::OnEnter() {
   // 突進の準備開始
   preparationTimer_.Start(preparationTime_);
   isPreparationComplete_ = false;
   
   // プレイヤーへの方向を計算
   chargeDirection_ = CalculateDirectionToPlayer();
   
   // デバッグログ
   // std::cout << "[ChargeAction] Preparation started" << std::endl;
}

NodeState ChargeToPlayerAction::OnExecute() {
   // 準備フェーズ
   if (!isPreparationComplete_) {
      PrepareCharge();
      
      if (preparationTimer_.IsFinished()) {
         isPreparationComplete_ = true;
         chargeTimer_.Start(chargeDuration_);
         
         // 突進開始時のパラメータ設定
         boss_->SetMaxSpeed(chargeMaxSpeed_);
         boss_->SetDamping(chargeDamping_);
         
         // デバッグログ
         // std::cout << "[ChargeAction] Charge started!" << std::endl;
      }
      
      return BossActionHelper::Running();
   }
   
   // 突進フェーズ
   if (!chargeTimer_.IsFinished()) {
      ExecuteCharge();
      return BossActionHelper::Running();
   }
   
   // 突進完了
   return BossActionHelper::Success();
}

void ChargeToPlayerAction::OnExit() {
   CompleteCharge();
   
   // デバッグログ
   // std::cout << "[ChargeAction] Charge completed" << std::endl;
}

void ChargeToPlayerAction::SetupStateMachine() {
   // 基底クラスのステートマシンをセットアップ
   BossActionNode::SetupStateMachine();
   
   // 必要に応じて追加の状態を定義
   // 例: 突進前の溜めアニメーション状態など
}

Vector3 ChargeToPlayerAction::CalculateDirectionToPlayer() const {
   if (!boss_ || !player_) {
      return {0.0f, 0.0f, 0.0f};
   }
   
   return boss_->GetDirectionToPlayer();
}

void ChargeToPlayerAction::PrepareCharge() {
   if (!boss_) return;
   
   // 準備中の演出（例: 体を縮める、光るなど）
   // ここでは簡単な例として、プレイヤーの方向を向く処理
   
   // プレイヤーへの方向を再計算（プレイヤーが動いた場合に対応）
   chargeDirection_ = CalculateDirectionToPlayer();
   
   // ボスをプレイヤーの方向に向ける（オプション）
   // TODO: ボスの向きを変更する処理を追加
}

void ChargeToPlayerAction::ExecuteCharge() {
   if (!boss_) return;
   
   // 突進の加速度を計算
   Vector2 acceleration2D = {
      chargeDirection_.x * chargeSpeed_,
      chargeDirection_.z * chargeSpeed_
   };
   
   // Bossの公開メソッドを使用して加速度を追加
   boss_->AddAcceleration(acceleration2D);
}

void ChargeToPlayerAction::CompleteCharge() {
   if (!boss_) return;
   
   // 突進終了後の処理
   // 移動パラメータを通常に戻す
   boss_->ResetMovementParameters();
}
