#pragma once
#include "ActionNode.h"
#include "Engine/Utility/Timer/GameTimer.h"
#include "Engine/Math/Vector/Vector3.h"
#include <functional>

/// @brief ボスを中心に8方向に弾を発射するアクション
class ShootEightWayAction : public BossActionNode {
public:
   /// @brief 弾生成関数の型定義
   /// @param position 弾の初期位置
   /// @param direction 弾の進行方向
   /// @param speed 弾の速度
   using BulletSpawnFunction = std::function<void(const Vector3&, const Vector3&, float)>;

   /// @brief コンストラクタ
   /// @param boss ボスへの参照
   /// @param bulletSpawnFunc 弾生成関数
   /// @param offsetRadius 発射位置のオフセット半径
   /// @param shootInterval 連続発射の間隔（秒）、0なら一斉発射
   /// @param bulletSpeed 弾の速度（デフォルト: 20.0f）
   ShootEightWayAction(Boss* boss,
                       BulletSpawnFunction bulletSpawnFunc,
                       float offsetRadius = 4.0f,
                       float shootInterval = 0.0f,
                       float bulletSpeed = 20.0f);

   ~ShootEightWayAction() override = default;

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
   BulletSpawnFunction bulletSpawnFunc_;  // 弾生成関数
   float offsetRadius_;                   // 発射位置のオフセット半径
   float shootInterval_;                  // 発射間隔
   float bulletSpeed_;                    // 弾の速度

   GameTimer shootTimer_;               // 発射タイマー
   
   int currentBulletIndex_;               // 現在発射する弾のインデックス
   bool hasStartedShooting_;              // 発射を開始したか
   
   static constexpr int BULLET_COUNT = 8; // 弾の数

   /// @brief 8方向の弾を発射
   void ShootBullets();

   /// @brief 指定されたインデックスの弾を1つ発射
   void ShootSingleBullet(int index);

   /// @brief 指定されたインデックスの方向ベクトルを取得
   Vector3 GetDirectionForIndex(int index) const;
};
