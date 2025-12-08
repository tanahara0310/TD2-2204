#include "SparkNode.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/GameObject/SparkColliderObject/SparkColliderObject.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Math/MathCore.h"
#include <numbers>
#include <algorithm>

SparkNode::SparkNode(Boss* boss, SparkColliderObject* sparkCollider) :
   BossActionNode(boss, "SparkNode"),
   sparkCollider_(sparkCollider) {
   // ステートマシンのセットアップ
   SetupSparkStateMachine();
}

void SparkNode::Reset() {
   BossActionNode::Reset();
   timer_.Reset();
   isFinished_ = false;

   // リセット時にスパークを非アクティブに
   if (sparkCollider_) {
      sparkCollider_->SetSparkActive(false);
   }
}

void SparkNode::OnEnter() {
   isFinished_ = false;
   sparkStateMachine_->RequestState("Startup", 0);
}

NodeState SparkNode::OnExecute() {
   sparkStateMachine_->Update();

   if (isFinished_) {
      return NodeState::Success;
   }

   return NodeState::Running;
}

void SparkNode::OnExit() {
   // スパークを非アクティブに
   if (sparkCollider_) {
      sparkCollider_->SetSparkActive(false);
	  sparkCollider_->SetPosition({ 0.0f, 0.0f, -1000.0f });
   }

   // スパークエフェクトを停止
   if (boss_) {
      boss_->StopSparkEffect();
   }
}

void SparkNode::SetupStateMachine() {
   BossActionNode::SetupStateMachine();
}

void SparkNode::SetupSparkStateMachine() {
   sparkStateMachine_ = std::make_unique<StateMachine>();

   sparkStateMachine_->AddState("Startup", std::bind(&SparkNode::InitializeStartup, this), std::bind(&SparkNode::Startup, this));
   sparkStateMachine_->AddState("Active", std::bind(&SparkNode::InitializeActive, this), std::bind(&SparkNode::Active, this));
   sparkStateMachine_->AddState("Recovery", std::bind(&SparkNode::InitializeRecovery, this), std::bind(&SparkNode::Recovery, this));

   // 遷移ルールを設定
   sparkStateMachine_->AddTransitionRule("Startup", { "Active" });
   sparkStateMachine_->AddTransitionRule("Active", { "Recovery" });
   sparkStateMachine_->AddTransitionRule("Recovery", { "Startup" });
}

void SparkNode::InitializeStartup() {
   // ボスの移動を停止
   if (boss_) {
      boss_->SetAcceleration({ 0.0f, 0.0f });
      boss_->SetVelocity({ 0.0f, 0.0f });
      boss_->SetDirection({ 0.0f, 0.0f });
   }

   // スパークを非アクティブに（念のため）
   if (sparkCollider_) {
      sparkCollider_->SetSparkActive(false);
   }

   // スタートアップタイマー開始
   timer_.Start(startupDuration_, false);
}

void SparkNode::Startup() {
   timer_.Update(GameUtils::GetDeltaTime());

   // スタートアップ時間が終了したらアクティブ状態に遷移
   if (timer_.IsFinished()) {
      sparkStateMachine_->RequestState("Active", 0);
   }
}

void SparkNode::InitializeActive() {
   // スパークコライダーの位置をボスの位置に設定し、アクティブに
   if (sparkCollider_ && boss_) {
      sparkCollider_->SetPosition(boss_->GetWorldPosition());
      sparkCollider_->SetSparkActive(true);
   }

   // スパークエフェクトを開始（Boss経由）
   if (boss_) {
      boss_->StartSparkEffect();
   }

   // アクティブタイマー開始
   timer_.Start(activeDuration_, false);
}

void SparkNode::Active() {
   timer_.Update(GameUtils::GetDeltaTime());

   // スパークコライダーの位置をボスの位置に追従させる
   if (sparkCollider_ && boss_) {
      sparkCollider_->SetPosition(boss_->GetWorldPosition());
   }

   // スパークエフェクトの位置を更新
   if (boss_) {
      boss_->UpdateSparkEffect();
   }

   // アクティブ時間が終了したらリカバリー状態に遷移
   if (timer_.IsFinished()) {
      sparkStateMachine_->RequestState("Recovery", 0);
   }
}

void SparkNode::InitializeRecovery() {
   // スパークを非アクティブに
   if (sparkCollider_) {
      sparkCollider_->SetSparkActive(false);
   }

   // スパークエフェクトを停止
   if (boss_) {
      boss_->StopSparkEffect();
   }

   // リカバリータイマー開始
   timer_.Start(recoveryDuration_, false);
}

void SparkNode::Recovery() {
   timer_.Update(GameUtils::GetDeltaTime());

   // リカバリー時間が終了したら完了
   if (timer_.IsFinished()) {
      isFinished_ = true;
   }
}