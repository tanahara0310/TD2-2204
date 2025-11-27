#pragma once

#include "Application/TD2_2/AI/Node/LeafNode.h"
#include "Application/TD2_2/Utility/StateMachine.h"
#include <memory>
#include <string>

// 前方宣言
class Boss;

/// @brief ボスのアクションノード基底クラス
/// ビヘイビアツリーのリーフノードとして機能し、内部でステートマシンを持つ
class BossActionNode : public ActionNode {
public:
   /// @brief アクション状態
   enum class ActionState {
      Idle,       // 待機（未実行）
      Enter,      // 開始処理
      Execute,    // 実行中
      Exit,       // 終了処理
      Completed   // 完了
   };

   /// @brief コンストラクタ
   /// @param boss ボスへの参照
   /// @param actionName アクション名（デバッグ用）
   explicit BossActionNode(Boss* boss, const std::string& actionName = "UnnamedAction");
   
   virtual ~BossActionNode() = default;

   /// @brief ビヘイビアツリーから呼び出されるTick
   /// @return ノードの実行結果
   NodeState Tick() override final;

   /// @brief アクションをリセット（再実行可能にする）
   virtual void Reset();

   /// @brief 現在のアクション状態を取得
   ActionState GetActionState() const { return currentState_; }

   /// @brief アクション名を取得
   const std::string& GetActionName() const { return actionName_; }

   /// @brief アクションが完了したか
   bool IsCompleted() const { return currentState_ == ActionState::Completed; }

   /// @brief アクションが実行中か
   bool IsExecuting() const { return currentState_ == ActionState::Execute; }

protected:
   Boss* boss_;                          // ボスへの参照
   ActionState currentState_;            // 現在の状態
   std::string actionName_;              // アクション名
   std::unique_ptr<StateMachine> stateMachine_;  // 内部ステートマシン

   /// @brief アクション開始時の処理（派生クラスで実装）
   virtual void OnEnter() {}

   /// @brief アクション実行中の処理（派生クラスで実装）
   /// @return NodeState (Success, Failure, Running)
   virtual NodeState OnExecute() = 0;

   /// @brief アクション終了時の処理（派生クラスで実装）
   virtual void OnExit() {}

   /// @brief ステートマシンのセットアップ（派生クラスでオーバーライド可能）
   virtual void SetupStateMachine();

private:
   /// @brief 内部状態を更新
   void UpdateState();
};

/// @brief アクション実行結果のヘルパー関数
namespace BossActionHelper {
   /// @brief 成功を返す
   inline NodeState Success() { return NodeState::Success; }
   
   /// @brief 失敗を返す
   inline NodeState Failure() { return NodeState::Failure; }
   
   /// @brief 実行中を返す
   inline NodeState Running() { return NodeState::Running; }
}
