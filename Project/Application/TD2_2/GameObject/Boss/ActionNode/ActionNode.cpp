#include "ActionNode.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/Utility/StateMachine.h"

BossActionNode::BossActionNode(Boss* boss, const std::string& actionName)
   : ActionNode(nullptr), boss_(boss), currentState_(ActionState::Idle), actionName_(actionName) {
   
   // ステートマシンの初期化
   stateMachine_ = std::make_unique<StateMachine>();
   SetupStateMachine();
}

NodeState BossActionNode::Tick() {
   UpdateState();
   
   switch (currentState_) {
      case ActionState::Idle:
         // まだ開始していない
         return NodeState::Running;
         
      case ActionState::Enter:
         // 開始処理を実行
         return NodeState::Running;
         
      case ActionState::Execute:
         // 実行中の処理を呼び出し
         return OnExecute();
         
      case ActionState::Exit:
         // 終了処理中
         return NodeState::Running;
         
      case ActionState::Completed:
         // 完了
         return NodeState::Success;
         
      default:
         return NodeState::Failure;
   }
}

void BossActionNode::Reset() {
   currentState_ = ActionState::Idle;
   if (stateMachine_) {
      stateMachine_->Clear();
   }
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
         // 完了時の処理
      },
      [this]() {
         // 何もしない（終了状態）
      }
   );

   // 初期状態を設定
   stateMachine_->RequestState("Idle", 0);
}

void BossActionNode::UpdateState() {
   if (!stateMachine_) return;
   
   stateMachine_->Update();
   
   // ステートマシンの状態をActionStateに同期
   const std::string& currentStateName = stateMachine_->GetCurrentState();
   
   if (currentStateName == "Idle") {
      currentState_ = ActionState::Idle;
   } else if (currentStateName == "Enter") {
      currentState_ = ActionState::Enter;
   } else if (currentStateName == "Execute") {
      currentState_ = ActionState::Execute;
   } else if (currentStateName == "Exit") {
      currentState_ = ActionState::Exit;
   } else if (currentStateName == "Completed") {
      currentState_ = ActionState::Completed;
   }
}
