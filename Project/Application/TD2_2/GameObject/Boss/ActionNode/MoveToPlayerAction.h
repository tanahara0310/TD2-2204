#pragma once
#include "ActionNode.h"
#include "Engine/Utility/Timer/GameTimer.h"

class Player;

/// @brief プレイヤーに向かって移動するアクション
class MoveToPlayerAction : public BossActionNode {
public:
   /// @brief コンストラクタ
   /// @param boss ボスへの参照
   /// @param player プレイヤーへの参照
   /// @param moveSpeed 移動速度
   /// @param duration 移動持続時間（秒）
   /// @param stopDistance プレイヤーからこの距離に到達したら完了
   MoveToPlayerAction(Boss* boss, 
                      Player* player,
                      float moveSpeed = 2000.0f, 
                      float duration = 2.0f,
                      float stopDistance = 10.0f);

   ~MoveToPlayerAction() override = default;

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
   Player* player_;         // プレイヤーへの参照
   float moveSpeed_;        // 移動速度
   float duration_;         // 移動持続時間
   float stopDistance_;     // 停止距離
   
   GameTimer moveTimer_;    // 移動タイマー

   /// @brief プレイヤーへの方向ベクトルを計算
   Vector2 CalculateDirectionToPlayer() const;

   /// @brief プレイヤーからの距離を計算
   float CalculateDistanceToPlayer() const;

   /// @brief 移動の実行処理
   void ExecuteMove();
};
