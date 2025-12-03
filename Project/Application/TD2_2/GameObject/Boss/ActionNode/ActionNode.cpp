#include "ActionNode.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/Utility/StateMachine.h"

BossActionNode::BossActionNode(Boss* boss, const std::string& actionName)
   : boss_(boss), actionName_(actionName) {
   
   // ステートマシンの初期化
   stateMachine_ = std::make_unique<StateMachine>();
   SetupStateMachine();
   firstTick_ = true;
}

NodeState BossActionNode::Tick() {
   if (firstTick_) {
      firstTick_ = false;
      UpdateState(); // 内部でEnterやExecuteまで進む可能性がある
      return NodeState::Running; // 強制的にRunningを返す
   }

   UpdateState();

   if(!stateMachine_) {
      return NodeState::Failure;
   }

   const std::string& currentState = stateMachine_->GetCurrentState();

   if (currentState == "Completed") {
      stateMachine_->RequestState("Idle", 1);
      firstTick_ = true;
      return NodeState::Success;
   } else {
      return NodeState::Running;
   }
}

void BossActionNode::Reset() {
   if (stateMachine_) {
      stateMachine_->Clear();
   }

   SetupStateMachine();
   firstTick_ = true;
}

void BossActionNode::SetupStateMachine() {
   if (!stateMachine_) return;

   // Idle状態
   stateMachine_->AddState("Idle",
      [this]() {
         // Idle開始時の処理
      },
      [this]() {
         // Enterへ遷移
         stateMachine_->RequestState("Enter", 1);
      }
   );

   // Enter状態
   stateMachine_->AddState("Enter",
      [this]() {
         OnEnter();
      },
      [this]() {
         // Executeへ遷移
         stateMachine_->RequestState("Execute", 1);
      }
   );

   // Execute状態
   stateMachine_->AddState("Execute",
      [this]() {
         // Execute開始時の処理
      },
      [this]() {
         // OnExecuteの結果に応じて遷移
         NodeState result = OnExecute();
         if (result == NodeState::Success || result == NodeState::Failure) {
            stateMachine_->RequestState("Exit", 1);
         }
         // Runningの場合は継続
      }
   );

   // Exit状態
   stateMachine_->AddState("Exit",
      [this]() {
         OnExit();
      },
      [this]() {
         // Completedへ遷移
         stateMachine_->RequestState("Completed", 1);
      }
   );

   // Completed状態
   stateMachine_->AddState("Completed",
      [this]() {
         // 完了時の処理 - ここで待機し、自動的には遷移しない
      },
      [this]() {
      }
   );

   // 初期状態を設定
   stateMachine_->RequestState("Idle", 0);
}

void BossActionNode::UpdateState() {
   if (!stateMachine_) return;
   stateMachine_->Update();
}
