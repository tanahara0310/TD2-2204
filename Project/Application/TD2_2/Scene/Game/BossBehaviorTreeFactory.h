#pragma once
#include <memory>
#include <functional>
#include "Application/TD2_2/AI/BehaviorTree/BehaviorTree.h"
#include "Engine/Math/Vector/Vector3.h"

// Forward declarations
class Boss;
class Player;
class SparkColliderObject;
struct BossAIParameters;
enum class BulletType;

/// @brief ボスのビヘイビアツリー生成を担当するファクトリクラス
class BossBehaviorTreeFactory {
public:
   /// @brief ビヘイビアツリーを生成
   /// @param boss ボスへの参照
   /// @param player プレイヤーへの参照
   /// @param sparkCollider スパークコライダーへの参照
   /// @param aiParams AIパラメータ
   /// @param createBulletCallback 弾生成コールバック
   /// @return 生成されたビヘイビアツリー
   static std::unique_ptr<BehaviorTree> Create(
      Boss* boss,
      Player* player,
      SparkColliderObject* sparkCollider,
      const BossAIParameters& aiParams,
      std::function<void(const Vector3&, const Vector3&, BulletType, float)> createBulletCallback
   );

private:
   // 各HPフェーズのビヘイビアツリー構築
   static void BuildHP5Phase(class BehaviorTreeBuilder& builder, Boss* boss, Player* player, 
                             const std::function<int()>& getBossHP);
   
   static void BuildHP4Phase(class BehaviorTreeBuilder& builder, Boss* boss, Player* player,
                             const std::function<int()>& getBossHP);
   
   static void BuildHP3Phase(class BehaviorTreeBuilder& builder, Boss* boss, Player* player,
                             const BossAIParameters& aiParams,
                             std::function<void(const Vector3&, const Vector3&, BulletType, float)> createBulletCallback,
                             const std::function<int()>& getBossHP);
   
   static void BuildHP2Phase(class BehaviorTreeBuilder& builder, Boss* boss, Player* player,
                             SparkColliderObject* sparkCollider,
                             const BossAIParameters& aiParams,
                             const std::function<int()>& getBossHP);
   
   static void BuildHP1Phase(class BehaviorTreeBuilder& builder, Boss* boss, Player* player,
                             SparkColliderObject* sparkCollider,
                             const BossAIParameters& aiParams,
                             std::function<void(const Vector3&, const Vector3&, BulletType, float)> createBulletCallback,
                             const std::function<int()>& getBossHP);
};
