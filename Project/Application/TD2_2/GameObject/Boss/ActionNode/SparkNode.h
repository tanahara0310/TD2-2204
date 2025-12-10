#pragma once
#include "ActionNode.h"
#include "Engine/Utility/Timer/GameTimer.h"
#include "Engine/Math/Vector/Vector3.h"
#include <functional>

class SparkColliderObject;

class SparkNode : public BossActionNode {
public:
   /// @brief コンストラクタ
	 /// @param boss ボスへの参照
	 /// @param sparkCollider スパーク当たり判定用オブジェクト
   SparkNode(Boss* boss, SparkColliderObject* sparkCollider);

   ~SparkNode() override = default;

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
   GameTimer timer_;

   bool isFinished_ = false;  

   std::unique_ptr<StateMachine> sparkStateMachine_;

   // スパーク設定
   float startupDuration_ = 1.0f;    // スタートアップ時間（秒）
   float activeDuration_ = 0.5f;     // アクティブ時間（秒）
   float recoveryDuration_ = 0.5f;   // リカバリー時間（秒）

   // スパークコライダーオブジェクト（外部で管理）
   SparkColliderObject* sparkCollider_ = nullptr;

private:
   void SetupSparkStateMachine();

   void InitializeStartup();

   void Startup();

   void InitializeActive();

   void Active();

   void InitializeRecovery();

   void Recovery();
};