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
   // 評価関数（距離判定を修正）
   
   // 近い（2.0f～5.0f）
   auto createCloseToPlayerEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition() - player->GetWorldPosition());
         if (dist < 2.0f) return 0.5f;
         if (dist > 5.0f) return std::clamp(1.0f - (dist - 5.0f) / 10.0f, 0.0f, 0.5f);
         return 1.0f;
      });
   };
   
   // 壁からの距離評価関数
   auto createDangerZoneEvaluator = [boss]() {
      return std::make_unique<LambdaEvaluator>([boss]() {
         Vector3 bossPos = boss->GetWorldPosition();
         float halfWidth = (GameSceneConfig::kMoveableAreaSize.x * 0.5f) - GameSceneConfig::kFrameSize.x - 3.0f;
         float halfHeight = (GameSceneConfig::kMoveableAreaSize.y * 0.5f) - GameSceneConfig::kFrameSize.y - 3.0f;
         float distX = halfWidth - std::abs(bossPos.x);
         float distY = halfHeight - std::abs(bossPos.y);
         return (distX < 10.0f || distY < 10.0f) ? 1.0f : 0.0f;
      });
   };

   // ボスが外側 & プレイヤーが近い（修正：5.0f以内）
   auto createBossOuterAndPlayerCloseEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         Vector3 bossPos = boss->GetWorldPosition();
         Vector3 playerPos = player->GetWorldPosition();
         Vector3 center = {0.0f, 0.0f, 0.0f};
         
         float bossDistFromCenter = MathCore::Vector::Length(bossPos - center);
         float playerDistFromCenter = MathCore::Vector::Length(playerPos - center);
         float distToPlayer = MathCore::Vector::Length(bossPos - playerPos);
         
         bool isOuter = bossDistFromCenter >= playerDistFromCenter;
         bool isPlayerClose = distToPlayer < 5.0f; // 修正: 15.0f → 5.0f
         
         return (isOuter && isPlayerClose) ? 1.0f : 0.0f;
      });
   };

   builder.Sequence()
      .Condition([getBossHP]() { return getBossHP() == 4; })
      .WeightedSelector()
      
      // ピンチ時は安全方向に突進（ウェイトを減らす）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeTowardsSafetyAction>(boss, player));
            return seq;
         }(),
         [createDangerZoneEvaluator, createBossOuterAndPlayerCloseEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Max);
            comp->AddEvaluator(createDangerZoneEvaluator(), 2.0f); // 3.0f → 2.0f
            comp->AddEvaluator(createBossOuterAndPlayerCloseEval(), 2.0f); // 3.0f → 2.0f
            return comp;
         }()
      )
      
      // 基本は逃げ（増加）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            return seq;
         }(),
         0.8f // 0.7f → 0.8f
      )
      
      // 中央移動（増加）
      .WeightedAction<MoveToCenterAction>(0.6f, boss) // 0.5f → 0.6f
      
      // 逃げ→突進（近い時）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [createCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createCloseToPlayerEval(), 0.5f); // 0.4f → 0.5f
            return comp;
         }()
      )
      
      .End()
      .End();
}

void BossBehaviorTreeFactory::BuildHP3Phase(BehaviorTreeBuilder& builder, Boss* boss, Player* player,
                                            const BossAIParameters& aiParams,
                                            std::function<void(const Vector3&, const Vector3&, BulletType, float)> createBulletCallback,
                                            const std::function<int()>& getBossHP) {
   // 評価関数（距離判定を修正）
   
   // 非常に近い（0～2.0f）：突進に最適
   auto createVeryCloseToPlayerEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition() - player->GetWorldPosition());
         // 2.0f以下で1.0f、5.0f以上で0.0f
         return std::clamp(1.0f - (dist - 2.0f) / 3.0f, 0.0f, 1.0f);
      });
   };
   
   // 近い（2.0f～5.0f）：突進・ショットのバランス
   auto createCloseToPlayerEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition() - player->GetWorldPosition());
         // 2.0f～5.0fの範囲で1.0f、範囲外は低い
         if (dist < 2.0f) return 0.5f;
         if (dist > 5.0f) return std::clamp(1.0f - (dist - 5.0f) / 10.0f, 0.0f, 0.5f);
         return 1.0f;
      });
   };
   
   // 遠い（5.0f以上）：位置取り重視
   auto createFarFromPlayerEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition() - player->GetWorldPosition());
         // 5.0f以下で0.0f、10.0f以上で1.0f
         return std::clamp((dist - 5.0f) / 5.0f, 0.0f, 1.0f);
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
         return (distX < 10.0f || distY < 10.0f) ? 1.0f : 0.0f;
      });
   };

   // 【NEW】ボスが外側 & プレイヤーが近い（修正：5.0f以内）
   auto createBossOuterAndPlayerCloseEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         Vector3 bossPos = boss->GetWorldPosition();
         Vector3 playerPos = player->GetWorldPosition();
         Vector3 center = {0.0f, 0.0f, 0.0f};
         
         float bossDistFromCenter = MathCore::Vector::Length(bossPos - center);
         float playerDistFromCenter = MathCore::Vector::Length(playerPos - center);
         float distToPlayer = MathCore::Vector::Length(bossPos - playerPos);
         
         bool isOuter = bossDistFromCenter >= playerDistFromCenter;
         bool isPlayerClose = distToPlayer < 5.0f; // 修正: 15.0f → 5.0f
         
         return (isOuter && isPlayerClose) ? 1.0f : 0.0f;
      });
   };

   builder.Sequence()
      .Condition([getBossHP]() { return getBossHP() == 3; })
      .WeightedSelector()
      
      // ピンチ時は安全方向に突進（壁に近い or 外側で近い時）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeTowardsSafetyAction>(boss, player));
            return seq;
         }(),
         [createDangerZoneEval, createBossOuterAndPlayerCloseEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Max);
            comp->AddEvaluator(createDangerZoneEval(), 1.0f);
            comp->AddEvaluator(createBossOuterAndPlayerCloseEval(), 1.0f);
            return comp;
         }()
      )
      
      // 非常に近距離突進（0～2.0f）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createVeryCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createVeryCloseToPlayerEval(), aiParams.hp3.chargeOnly);
            return comp;
         }()
      )
      
      // 突進→逃げ（2.0f～5.0f）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createCloseToPlayerEval(), aiParams.hp3.chargeAndFlee);
            return comp;
         }()
      )
      
      // 突進×2（2.0f～5.0f）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createCloseToPlayerEval(), aiParams.hp3.doubleCharge);
            return comp;
         }()
      )
      
      // ショット（遠い時優先）
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
         [aiParams, createNearCenterEval, createFarFromPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createNearCenterEval(), aiParams.hp3.shootOnly);
            comp->AddEvaluator(createFarFromPlayerEval(), 0.5f); // 遠い時ボーナス
            return comp;
         }()
      )
      
      // ショット→突進（近い時）
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
         [aiParams, createCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createCloseToPlayerEval(), aiParams.hp3.shootAndCharge);
            return comp;
         }()
      )
      
      // 中央移動→突進（遠い時は中央へ）
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
   // 評価関数（距離判定を修正）
   
   // 非常に近い（0～2.0f）
   auto createVeryCloseToPlayerEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition() - player->GetWorldPosition());
         return std::clamp(1.0f - (dist - 2.0f) / 3.0f, 0.0f, 1.0f);
      });
   };
   
   // 近い（2.0f～5.0f）
   auto createCloseToPlayerEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         float dist = MathCore::Vector::Length(boss->GetWorldPosition() - player->GetWorldPosition());
         if (dist < 2.0f) return 0.5f;
         if (dist > 5.0f) return std::clamp(1.0f - (dist - 5.0f) / 10.0f, 0.0f, 0.5f);
         return 1.0f;
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
         return (distX < 10.0f || distY < 10.0f) ? 1.0f : 0.0f;
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

   // 【NEW】ボスが外側 & プレイヤーが近い（修正：5.0f以内）
   auto createBossOuterAndPlayerCloseEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         Vector3 bossPos = boss->GetWorldPosition();
         Vector3 playerPos = player->GetWorldPosition();
         Vector3 center = {0.0f, 0.0f, 0.0f};
         
         float bossDistFromCenter = MathCore::Vector::Length(bossPos - center);
         float playerDistFromCenter = MathCore::Vector::Length(playerPos - center);
         float distToPlayer = MathCore::Vector::Length(bossPos - playerPos);
         
         bool isOuter = bossDistFromCenter >= playerDistFromCenter;
         bool isPlayerClose = distToPlayer < 5.0f; // 修正: 15.0f → 5.0f
         
         return (isOuter && isPlayerClose) ? 1.0f : 0.0f;
      });
   };

   builder.Sequence()
      .Condition([getBossHP]() { return getBossHP() == 2; })
      .WeightedSelector()
      
      // ピンチ時は安全方向に突進（ウェイトを減らして連続選択を防ぐ）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeTowardsSafetyAction>(boss, player));
            return seq;
         }(),
         [createDangerZoneEval, createBossOuterAndPlayerCloseEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Max);
            comp->AddEvaluator(createDangerZoneEval(), 2.0f); // 3.0f → 2.0f
            comp->AddEvaluator(createBossOuterAndPlayerCloseEval(), 2.0f); // 3.0f → 2.0f
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
         [aiParams, createEnergyLowEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createEnergyLowEval(), aiParams.hp2.fleeWhenLowEnergy);
            return comp;
         }()
      )
      
      // エネルギー溜まったら突進（近い時）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createEnergyReadyEval, createCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createEnergyReadyEval(), aiParams.hp2.energyChargeCombo);
            comp->AddEvaluator(createCloseToPlayerEval(), 1.0f);
            return comp;
         }()
      )
      
      // スタン中は追撃（近い時）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createStunBiasEval, createCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createStunBiasEval(), aiParams.hp2.stunPursuitBias);
            comp->AddEvaluator(createCloseToPlayerEval(), 1.0f);
            return comp;
         }()
      )
      
      // 単純な逃げ（追加：デフォルト行動）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            return seq;
         }(),
         0.6f // 0.5f → 0.6f（増加）
      )
      
      // 中央移動（追加：位置取り）
      .WeightedNode(
         [boss]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<MoveToCenterAction>(boss));
            return seq;
         }(),
         0.5f // 0.4f → 0.5f（増加）
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
      
      // スパークコンボ近距離版（Charge×2→Spark→Charge）- 非常に近い時
      .WeightedNode(
         [boss, player, sparkCollider]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<SparkNode>(boss, sparkCollider));
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createVeryCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createVeryCloseToPlayerEval(), aiParams.hp2.sparkComboClose);
            return comp;
         }()
      )
      
      // 突進→逃げ（近い時）
      .WeightedNode(
         [boss, player]() {
            auto seq = std::make_unique<SequenceNode>();
            seq->AddChild(std::make_unique<ChargeToPlayerAction>(boss, player));
            seq->AddChild(std::make_unique<FleeFromPlayerAction>(boss, player));
            return seq;
         }(),
         [aiParams, createCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createCloseToPlayerEval(), aiParams.hp2.chargeAndFlee);
            return comp;
         }()
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
         return (distX < 10.0f || distY < 10.0f) ? 1.0f : 0.0f;
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

   // 【NEW】ボスがプレイヤーより内側にいるかの評価
   auto createBossInnerThanPlayerEval = [boss, player]() {
      return std::make_unique<LambdaEvaluator>([boss, player]() {
         Vector3 bossPos = boss->GetWorldPosition();
         Vector3 playerPos = player->GetWorldPosition();
         Vector3 center = {0.0f, 0.0f, 0.0f};
         float bossDistFromCenter = MathCore::Vector::Length(bossPos - center);
         float playerDistFromCenter = MathCore::Vector::Length(playerPos - center);
         // ボスが内側にいる場合のみ1.0f
         return (bossDistFromCenter < playerDistFromCenter) ? 1.0f : 0.0f;
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
      
      // フェイント→ショット（内側 & プレイヤーが近い時のみ）
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
         [aiParams, createBossInnerThanPlayerEval, createCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createBossInnerThanPlayerEval(), aiParams.hp1.feintShoot);
            comp->AddEvaluator(createCloseToPlayerEval(), 1.0f);
            return comp;
         }()
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
      
      // ショット→突進（プレイヤーが近い時）
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
         [aiParams, createCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createCloseToPlayerEval(), aiParams.hp1.shootCharge);
            return comp;
         }()
      )
      
      // 逃げ→ショット×2（内側 & プレイヤーが近い時のみ）
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
         [aiParams, createBossInnerThanPlayerEval, createCloseToPlayerEval]() {
            auto comp = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
            comp->AddEvaluator(createBossInnerThanPlayerEval(), aiParams.hp1.retreatDoubleShoot);
            comp->AddEvaluator(createCloseToPlayerEval(), 1.0f);
            return comp;
         }()
      )
      
      .End()
      .End();
}
