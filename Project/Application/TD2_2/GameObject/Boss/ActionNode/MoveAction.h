#pragma once
#include "ActionNode.h"
#include "Engine/Utility/Timer/GameTimer.h"

class MoveAction : public BossActionNode {
public:
   /// @brief コンストラクタ
   /// @param boss ボスへの参照
   MoveAction(Boss* boss);

   ~MoveAction() override = default;

   /// @brief リセット
   void Reset() override;

protected:
   /// @brief アクション開始時の処理
   void OnEnter() override;

   /// @brief アクション実行中の処理
   NodeState OnExecute() override;

   /// @brief アクション終了時の処理
   void OnExit() override;

   /// @brief ステートマシンのセットアップ
   void SetupStateMachine() override;

private:
   float moveSpeed_ = 10.0f;           // 移動速度
   GameTimer moveTimer_;       // 移動タイマー
   // バイアス
   float biasAmount_ = 0.75f;

   void ExecuteMove();
};
