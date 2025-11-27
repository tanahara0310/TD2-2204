#pragma once
#include "ActionNode.h"
#include "Engine/Utility/Timer/GameTimer.h"

class Player;

/// @brief プレイヤーに向かって突進するアクション
class ChargeToPlayerAction : public BossActionNode {
public:
   /// @brief コンストラクタ
   /// @param boss ボスへの参照
   /// @param player プレイヤーへの参照
   /// @param chargeSpeed 突進速度
   /// @param chargeDuration 突進持続時間（秒）
   ChargeToPlayerAction(Boss* boss, Player* player, 
                        float chargeSpeed = 50000.0f, 
                        float chargeDuration = 0.5f);

   ~ChargeToPlayerAction() override = default;

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
   
   bool isPreparationComplete_;  // 準備完了フラグ
   float preparationTime_;       // 準備時間
   GameTimer preparationTimer_;  // 準備タイマー

   /// @brief プレイヤーへの方向ベクトルを計算
   Vector3 CalculateDirectionToPlayer() const;

   /// @brief 突進の準備処理
   void PrepareCharge();

   /// @brief 突進の実行処理
   void ExecuteCharge();

   /// @brief 突進後の処理
   void CompleteCharge();
};
