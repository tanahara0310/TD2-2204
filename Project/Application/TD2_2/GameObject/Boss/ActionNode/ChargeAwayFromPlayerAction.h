#pragma once
#include "ActionNode.h"
#include "Engine/Utility/Timer/GameTimer.h"
#include "Engine/Math/Vector/Vector3.h"

class Player;

/// @brief プレイヤーから離れる方向に突進するアクション（フェイント・逃げ用）
class ChargeAwayFromPlayerAction : public BossActionNode {
public:
   /// @brief コンストラクタ
   /// @param boss ボスへの参照
   /// @param player プレイヤーへの参照
   /// @param chargeSpeed 突進速度
   /// @param chargeDuration 突進持続時間（秒）
   ChargeAwayFromPlayerAction(Boss* boss, Player* player, 
                              float chargeSpeed = 5000.0f, 
                              float chargeDuration = 0.3f);

   ~ChargeAwayFromPlayerAction() override = default;

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
   Player* player_;              // プレイヤーへの参照
   float chargeSpeed_;           // 突進速度
   float chargeDuration_;        // 突進持続時間
   float chargeMaxSpeed_;        // 突進最大速度
   float chargeDamping_;         // 突進減衰率
   
   GameTimer chargeTimer_;       // 突進タイマー
   Vector3 chargeDirection_;     // 突進方向

   /// @brief プレイヤーから離れる方向ベクトルを計算
   Vector3 CalculateDirectionAwayFromPlayer() const;

   /// @brief 突進の実行処理
   void ExecuteCharge();
};
