#include "BossBehaviorTreeFactory.h"
#include "GameScene.h"
#include "../../GameObject/Boss/Boss.h"
#include "../../GameObject/Player/Player.h"
#include "../../GameObject/SparkColliderObject/SparkColliderObject.h"
#include "../../GameObject/Boss/ActionNode/ChargeToPlayerAction.h"
#include "../../GameObject/Boss/ActionNode/ChargeAwayFromPlayerAction.h"
#include "../../GameObject/Boss/ActionNode/ChargeTowardsSafetyAction.h"
#include "../../GameObject/Boss/ActionNode/FleeFromPlayerAction.h"
#include "../../GameObject/Boss/ActionNode/MoveToCenterAction.h"
#include "../../GameObject/Boss/ActionNode/ShootEightWayAction.h"
#include "../../GameObject/Boss/ActionNode/SparkNode.h"
#include "../../GameObject/Bullet/Bullet.h"
#include "../Config/GameSceneConfig.h"
#include "../../AI/BehaviorTree/BehaviorTreeBuilder.h"
#include "../../AI/Node/Evaluator.h"
#include "MathCore.h"
#include <algorithm>

std::unique_ptr<BehaviorTree> BossBehaviorTreeFactory::Create(
   Boss* boss,
   Player* player,
   SparkColliderObject* sparkCollider,
   const BossAIParameters& aiParams,
   std::function<void(const Vector3&, const Vector3&, BulletType, float)> createBulletCallback
) {
   auto getBossHP = [boss]() { return boss->GetHP(); };

   return BehaviorTreeFactory::Create(
      [boss, player, sparkCollider, aiParams, createBulletCallback, getBossHP](BehaviorTreeBuilder& builder) {
         builder.Selector(); // Root Selector

         // 各HPフェーズを構築
         BuildHP5Phase(builder, boss, player, getBossHP);
         BuildHP4Phase(builder, boss, player, getBossHP);
         BuildHP3Phase(builder, boss, player, aiParams, createBulletCallback, getBossHP);
         BuildHP2Phase(builder, boss, player, sparkCollider, aiParams, getBossHP);
         BuildHP1Phase(builder, boss, player, sparkCollider, aiParams, createBulletCallback, getBossHP);

         // Fallback
         builder.Sequence()
            .Condition([getBossHP]() { return true; })
            .Action<FleeFromPlayerAction>(boss, player)
            .End();

         builder.End(); // Root Selector終了
      },
      "BossStrategicAI"
   );
}

void BossBehaviorTreeFactory::BuildHP5Phase(BehaviorTreeBuilder& builder, Boss* boss, Player* player,
                                            const std::function<int()>& getBossHP) {
   builder.Sequence()
      .Condition([getBossHP]() { return getBossHP() == 5; })
      .Action<FleeFromPlayerAction>(boss, player)
      .End();
}

void BossBehaviorTreeFactory::BuildHP4Phase(BehaviorTreeBuilder& builder, Boss* boss, Player* player,
                                            const std::function<int()>& getBossHP) {
   // 壁からの距離評価関数
   auto createDangerZoneEvaluator = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         Vector3 bossPos = boss->GetWorldPosition();
         float halfWidth = (GameSceneConfig::kMoveableAreaSize.x * 0.5f) - GameSceneConfig::kFrameSize.x - 3.0f;
         float halfHeight = (GameSceneConfig::kMoveableAreaSize.y * 0.5f) - GameSceneConfig::kFrameSize.y - 3.0f;
         float distX = halfWidth - std::abs(bossPos.x);
         float distY = halfHeight - std::abs(bossPos.y);
         return (distX < 8.0f || distY < 8.0f) ? 1.0f : 0.0f;
      });
   };

   builder.Sequence()
      .Condition([getBossHP]() { return getBossHP() == 4; })
      .WeightedSelector()
      
      // ピンチ時は安全方向に突進
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeTowardsSafetyAction>(boss, player));
            return seq;
         }(),
         createDangerZoneEvaluator()
      )
      
      // 通常時
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         0.5f
      )
      
      .WeightedAction<MoveToCenterAction>(0.3f, boss)
      
      .End()
      .End();
}

void BossBehaviorTreeFactory::BuildHP3Phase(BehaviorTreeBuilder& builder, Boss* boss, Player* player,
                                            const BossAIParameters& aiParams,
                                            std::function<void(const Vector3&, const Vector3&, BulletType, float)> createBulletCallback,
                                            const std::function<int()>& getBossHP) {
   // 評価関数
   auto createCloseToPlayerEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition() - player->GetWorldPosition());
         return std::clamp(1.0f - (dist - 15.0f) / 15.0f, 0.0f, 1.0f);
      });
   };

   auto createNearCenterEval = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition());
         return std::clamp(1.0f - (dist / 30.0f), 0.0f, 1.0f);
      });
   };

   auto createFarCenterEval = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition());
         return std::clamp(dist / 30.0f, 0.0f, 1.0f);
      });
   };

   auto createDangerZoneEval = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         Vector3 bossPos = boss->GetWorldPosition();
         float halfWidth = (GameSceneConfig::kMoveableAreaSize.x * 0.5f) - GameSceneConfig::kFrameSize.x - 3.0f;
         float halfHeight = (GameSceneConfig::kMoveableAreaSize.y * 0.5f) - GameSceneConfig::kFrameSize.y - 3.0f;
         float distX = halfWidth - std::abs(bossPos.x);
         float distY = halfHeight - std::abs(bossPos.y);
         return (distX < 8.0f || distY < 8.0f) ? 1.0f : 0.0f;
      });
   };

   builder.Sequence()
      .Condition([getBossHP]() { return getBossHP() == 3; })
      .WeightedSelector()
      
      // ピンチ時は安全方向に突進
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeTowardsSafetyAction>(boss, player));
            return seq;
         }(),
         createDangerZoneEval()
      )
      
      // 近距離突進
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createCloseToPlayerEval(), aiParams.hp3.chargeOnly);
            return comp;
         }()
      )
      
      // 突進→逃げ
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            return seq;
         }(),
         aiParams.hp3.chargeAndFlee
      )
      
      // 突進×2
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         aiParams.hp3.doubleCharge
      )
      
      // ショット
      .WeightedNode(
         [boss, createBulletCallback]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ShootEightWayAction>(boss, 
               [createBulletCallback](const Vector3& pos, const Vector3& dir, float speed) {
                  createBulletCallback(pos, dir, BulletType::ElasticSphere, speed);
               }
            ));
            return seq;
         }(),
         [aiParams, createNearCenterEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createNearCenterEval(), aiParams.hp3.shootOnly);
            return comp;
         }()
      )
      
      // ショット→突進
      .WeightedNode(
         [boss, player, createBulletCallback]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ShootEightWayAction>(boss,
               [createBulletCallback](const Vector3& pos, const Vector3& dir, float speed) {
                  createBulletCallback(pos, dir, BulletType::ElasticSphere, speed);
               }
            ));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         aiParams.hp3.shootAndCharge
      )
      
      // 中央移動→突進
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<MoveToCenterAction>(boss));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createFarCenterEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createFarCenterEval(), aiParams.hp3.moveCenterAndCharge);
            return comp;
         }()
      )
      
      .End()
      .End();
}

void BossBehaviorTreeFactory::BuildHP2Phase(BehaviorTreeBuilder& builder, Boss* boss, Player* player,
                                            SparkColliderObject* sparkCollider,
                                            const BossAIParameters& aiParams,
                                            const std::function<int()>& getBossHP) {
   // 評価関数
   auto createCloseToPlayerEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition() - player->GetWorldPosition());
         return std::clamp(1.0f - (dist - 15.0f) / 15.0f, 0.0f, 1.0f);
      });
   };

   auto createStunBiasEval = [player]() {
      return std::make_unique<LambdaEvaluator>([player]() {
         return player->IsStunned() ? 1.0f : 0.1f;
      });
   };

   auto createDangerZoneEval = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         Vector3 bossPos = boss->GetWorldPosition();
         float halfWidth = (GameSceneConfig::kMoveableAreaSize.x * 0.5f) - GameSceneConfig::kFrameSize.x - 3.0f;
         float halfHeight = (GameSceneConfig::kMoveableAreaSize.y * 0.5f) - GameSceneConfig::kFrameSize.y - 3.0f;
         float distX = halfWidth - std::abs(bossPos.x);
         float distY = halfHeight - std::abs(bossPos.y);
         return (distX < 8.0f || distY < 8.0f) ? 1.0f : 0.0f;
      });
   };

   auto createEnergyLowEval = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         return boss->IsEnergyLow() ? 1.0f : 0.1f;
      });
   };

   auto createEnergyReadyEval = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         return boss->IsEnergyReady() ? 1.0f : 0.2f;
      });
   };

   builder.Sequence()
      .Condition([getBossHP]() { return getBossHP() == 2; })
      .WeightedSelector()
      
      // ピンチ時は安全方向に突進
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeTowardsSafetyAction>(boss, player));
            return seq;
         }(),
         createDangerZoneEval()
      )
      
      // エネルギー低時は逃げる
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createEnergyLowEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createEnergyLowEval(), aiParams.hp2.fleeWhenLowEnergy);
            return comp;
         }()
      )
      
      // エネルギー溜まったら突進（スタン時間延長）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createEnergyReadyEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createEnergyReadyEval(), aiParams.hp2.energyChargeCombo);
            return comp;
         }()
      )
      
      // スタン中は追撃
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         createStunBiasEval()
      )
      
      // スパークコンボ（逃げ→Charge×2→Spark→Charge）
      .WeightedNode(
         [boss, player, sparkCollider]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<SparkNode>(boss, sparkCollider));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         aiParams.hp2.sparkCombo
      )
      
      // スパークコンボ近距離版（Charge×2→Spark→Charge）
      .WeightedNode(
         [boss, player, sparkCollider]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<SparkNode>(boss, sparkCollider));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createCloseToPlayerEval(), aiParams.hp2.sparkComboClose);
            return comp;
         }()
      )
      
      // 突進→逃げ
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            return seq;
         }(),
         aiParams.hp2.chargeAndFlee
      )
      
      .End()
      .End();
}

void BossBehaviorTreeFactory::BuildHP1Phase(BehaviorTreeBuilder& builder, Boss* boss, Player* player,
                                            SparkColliderObject* sparkCollider,
                                            const BossAIParameters& aiParams,
                                            std::function<void(const Vector3&, const Vector3&, BulletType, float)> createBulletCallback,
                                            const std::function<int()>& getBossHP) {
   // 評価関数（HP1用）
   auto createCloseToPlayerEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition() - player->GetWorldPosition());
         return std::clamp(1.0f - (dist - 15.0f) / 15.0f, 0.0f, 1.0f);
      });
   };

   auto createCloseRangeEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition() - player->GetWorldPosition());
         return std::clamp(1.0f - (dist - 20.0f) / 20.0f, 0.0f, 1.0f);
      });
   };

   auto createMediumRangeEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition() - player->GetWorldPosition());
         if (dist < 15.0f) return std::clamp(dist / 15.0f, 0.0f, 1.0f);
         if (dist > 25.0f) return std::clamp(1.0f - (dist - 25.0f) / 15.0f, 0.0f, 1.0f);
         return 1.0f;
      });
   };

   auto createNearCenterEval = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition());
         return std::clamp(1.0f - (dist / 30.0f), 0.0f, 1.0f);
      });
   };

   auto createFarCenterEval = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition());
         return std::clamp(dist / 30.0f, 0.0f, 1.0f);
      });
   };

   auto createStunBiasEval = [player]() {
      return std::make_unique<LambdaEvaluator>([player]() {
         return player->IsStunned() ? 1.0f : 0.1f;
      });
   };

   auto createNonStunBiasEval = [player]() {
      return std::make_unique<LambdaEvaluator>([player]() {
         return player->IsStunned() ? 0.0f : 1.0f;
      });
   };

   auto createDangerZoneEval = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         Vector3 bossPos = boss->GetWorldPosition();
         float halfWidth = (GameSceneConfig::kMoveableAreaSize.x * 0.5f) - GameSceneConfig::kFrameSize.x - 3.0f;
         float halfHeight = (GameSceneConfig::kMoveableAreaSize.y * 0.5f) - GameSceneConfig::kFrameSize.y - 3.0f;
         float distX = halfWidth - std::abs(bossPos.x);
         float distY = halfHeight - std::abs(bossPos.y);
         return (distX < 8.0f || distY < 8.0f) ? 1.0f : 0.0f;
      });
   };

   auto createEnergyReadyEval = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         return boss->IsEnergyReady() ? 1.0f : 0.2f;
      });
   };

   auto createEnergyLowEval = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         return boss->IsEnergyLow() ? 1.0f : 0.1f;
      });
   };

   builder.Sequence()
      .Condition([getBossHP]() { return getBossHP() <= 1; })
      .WeightedSelector()
      
      // ピンチ時は安全方向に突進
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeTowardsSafetyAction>(boss, player));
            return seq;
         }(),
         [aiParams, createDangerZoneEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createDangerZoneEval(), aiParams.hp1.safetyCharge);
            return comp;
         }()
      )
      
      // エネルギー低時は逃げる
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            return seq;
         }(),
         [createEnergyLowEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createEnergyLowEval(), 1.5f);
            return comp;
         }()
      )
      
      // エネルギー溜まったら突進
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createEnergyReadyEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createEnergyReadyEval(), aiParams.hp1.energyReadyCharge);
            return comp;
         }()
      )
      
      // スタン中は追撃
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createStunBiasEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createStunBiasEval(), aiParams.hp1.quadChargeStunBias);
            return comp;
         }()
      )
      
      // 中央確保
      .WeightedNode(
         [boss]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<MoveToCenterAction>(boss));
            return seq;
         }(),
         [aiParams, createFarCenterEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createFarCenterEval(), aiParams.hp1.centerBias);
            return comp;
         }()
      )
      
      // スパークコンボ（Charge×2→Spark→Charge）
      .WeightedNode(
         [boss, player, sparkCollider]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<SparkNode>(boss, sparkCollider));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createCloseToPlayerEval, createNonStunBiasEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createCloseToPlayerEval(), aiParams.hp1.sparkComboCloseBias);
            comp->AddEvaluator(createNonStunBiasEval(), aiParams.hp1.sparkComboNonStunBias);
            return comp;
         }()
      )
      
      // ショット→逃げ
      .WeightedNode(
         [boss, player, createBulletCallback]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ShootEightWayAction>(boss,
               [createBulletCallback](const Vector3& pos, const Vector3& dir, float speed) {
                  createBulletCallback(pos, dir, BulletType::ElasticSphere, speed);
               }
            ));
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createMediumRangeEval, createNearCenterEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createMediumRangeEval(), aiParams.hp1.shootMediumBias);
            comp->AddEvaluator(createNearCenterEval(), aiParams.hp1.shootCenterBias);
            return comp;
         }()
      )
      
      // 突進×2
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createCloseRangeEval, createNearCenterEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createCloseRangeEval(), aiParams.hp1.tripleChargeCloseBias);
            comp->AddEvaluator(createNearCenterEval(), aiParams.hp1.tripleChargeCenterBias);
            return comp;
         }()
      )
      
      // フェイント→ショット
      .WeightedNode(
         [boss, player, createBulletCallback]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeAwayFromPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ShootEightWayAction>(boss,
               [createBulletCallback](const Vector3& pos, const Vector3& dir, float speed) {
                  createBulletCallback(pos, dir, BulletType::ElasticSphere, speed);
               }
            ));
            return seq;
         }(),
         aiParams.hp1.feintShoot
      )
      
      // スパークコンボ遠距離版
      .WeightedNode(
         [boss, player, sparkCollider]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<SparkNode>(boss, sparkCollider));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createFarCenterEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createFarCenterEval(), aiParams.hp1.sparkComboFarBias);
            return comp;
         }()
      )
      
      // ショット→突進
      .WeightedNode(
         [boss, player, createBulletCallback]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ShootEightWayAction>(boss,
               [createBulletCallback](const Vector3& pos, const Vector3& dir, float speed) {
                  createBulletCallback(pos, dir, BulletType::ElasticSphere, speed);
               }
            ));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         aiParams.hp1.shootCharge
      )
      
      // 逃げ→ショット×2
      .WeightedNode(
         [boss, player, createBulletCallback]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeAwayFromPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ShootEightWayAction>(boss,
               [createBulletCallback](const Vector3& pos, const Vector3& dir, float speed) {
                  createBulletCallback(pos, dir, BulletType::ElasticSphere, speed);
               }
            ));
            seq->AddChild(std::make_unique<ShootEightWayAction>(boss,
               [createBulletCallback](const Vector3& pos, const Vector3& dir, float speed) {
                  createBulletCallback(pos, dir, BulletType::ElasticSphere, speed);
               }
            ));
            return seq;
         }(),
         aiParams.hp1.retreatDoubleShoot
      )
      
      .End()
      .End();
}
