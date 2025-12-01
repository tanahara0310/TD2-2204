#pragma once
#include "ActionNode.h"
#include "Engine/Utility/Timer/GameTimer.h"

/// @brief 中心に向かって移動するアクション
class MoveToCenterAction : public BossActionNode {
public:
   /// @brief コンストラクタ
   /// @param boss ボスへの参照
   /// @param moveSpeed 移動速度
   /// @param duration 移動持続時間（秒）
   /// @param stopDistance 中心からこの距離に到達したら完了
   MoveToCenterAction(Boss* boss, 
                      float moveSpeed = 3000.0f, 
                      float duration = 2.0f,
                      float stopDistance = 5.0f);

   ~MoveToCenterAction() override = default;

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
   float moveSpeed_;        // 移動速度
   float duration_;         // 移動持続時間
   float stopDistance_;     // 停止距離
   
   GameTimer moveTimer_;    // 移動タイマー

   /// @brief 中心への方向ベクトルを計算
   Vector2 CalculateDirectionToCenter() const;

   /// @brief 移動の実行処理
   void ExecuteMove();
};
