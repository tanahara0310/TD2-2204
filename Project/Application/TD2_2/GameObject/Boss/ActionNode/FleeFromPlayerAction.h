#pragma once
#include "ActionNode.h"
#include "Engine/Utility/Timer/GameTimer.h"

class Player;

/// @brief 常に中央に向かいつつ、プレイヤーとの衝突を避けるアクション
/// 中央への移動を最優先とし、プレイヤーが経路上にいる場合のみ回避動作を行う
class FleeFromPlayerAction : public BossActionNode {
public:
   /// @brief コンストラクタ
   /// @param boss ボスへの参照
   /// @param player プレイヤーへの参照
   /// @param moveSpeed 移動速度
   /// @param duration 移動持続時間（秒）
   /// @param avoidanceStrength プレイヤー回避の強さ（0.0～1.0）
   FleeFromPlayerAction(Boss* boss, 
                        Player* player,
                        float moveSpeed = 10.0f, 
                        float avoidanceStrength = 0.6f);

   ~FleeFromPlayerAction() override = default;

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
   Player* player_;                  // プレイヤーへの参照
   float moveSpeed_;                 // 移動速度
   float duration_;                  // 移動持続時間
   float avoidanceStrength_;         // プレイヤー回避の強さ（0.0～1.0）
   
   GameTimer moveTimer_;             // 移動タイマー
   Vector2 currentDirection_;        // 現在の移動方向（滑らかに更新）

   /// @brief 常に中央に向かいつつプレイヤーを避ける方向ベクトルを計算
   Vector2 CalculateMoveDirection();

   /// @brief プレイヤーからの距離を計算
   float CalculateDistanceToPlayer() const;
   
   /// @brief 中央からの距離を計算
   float CalculateDistanceToCenter() const;

   /// @brief 移動の実行処理
   void ExecuteMove();
};
