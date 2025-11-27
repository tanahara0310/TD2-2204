#pragma once

// ===================================================================
// ボスアクションノードの使用例（BehaviorTreeクラス使用版）
// ===================================================================

/*

// GameScene.hでの宣言例
#include "Application/TD2_2/GameObject/Boss/BehaviorTree/BehaviorTree.h"

class GameScene : public BaseScene {
private:
    std::unique_ptr<Boss> boss_;
    std::unique_ptr<Player> player_;
    
    std::unique_ptr<BehaviorTree> CreateBossBehaviorTree();
};

// GameScene.cppでの実装例
#include "Application/TD2_2/GameObject/Boss/ActionNode/ChargeToPlayerAction.h"

void GameScene::Initialize() {
    // ボスとプレイヤーの初期化
    boss_ = std::make_unique<Boss>();
    boss_->Initialize(bossModel, bossTexture);
    
    player_ = std::make_unique<Player>();
    player_->Initialize(playerModel, playerTexture);
    
    // ボスにプレイヤーへの参照を設定
    boss_->SetPlayer(player_.get());
    
    // ビヘイビアツリーの構築と設定
    boss_->SetBehaviorTree(CreateBossBehaviorTree());
}

void GameScene::Update() {
    // プレイヤーの更新
    if (player_) {
        player_->Update();
    }
    
    // ボスの更新（内部でビヘイビアツリーが実行される）
    if (boss_) {
        boss_->Update();
    }
}

std::unique_ptr<BehaviorTree> GameScene::CreateBossBehaviorTree() {
    // BehaviorTreeFactoryを使用して構築
    return BehaviorTreeFactory::Create(
        [this](BehaviorTreeBuilder& builder) {
            builder.Selector()
                // 距離が10m以内なら突進攻撃
                .Sequence()
                    .Condition([this]() {
                        return boss_->GetDistanceToPlayer() <= 10.0f;
                    })
                    // ChargeToPlayerActionを追加する場合は
                    // アクションノードを継承した実装が必要
                .End()
                
                // それ以外は待機
                .Action<IdleAction>()
            .End();
        },
        "BossMainAI"  // ツリーの名前
    );
}

// ===================================================================
// より複雑なビヘイビアツリーの例
// ===================================================================

std::unique_ptr<BehaviorTree> CreateComplexBossAI(Boss* boss, Player* player) {
    return BehaviorTreeFactory::Create(
        [boss, player](BehaviorTreeBuilder& builder) {
            builder.Selector()
                // === 超近距離（0-3m）: 近接攻撃 ===
                .Sequence()
                    .Condition([boss]() {
                        return boss->GetDistanceToPlayer() <= 3.0f;
                    })
                    // TODO: MeleeAttackActionを追加
                .End()
                
                // === 近距離（3-10m）: 突進攻撃 ===
                .Sequence()
                    .Condition([boss]() {
                        float dist = boss->GetDistanceToPlayer();
                        return dist > 3.0f && dist <= 10.0f;
                    })
                    // ChargeToPlayerActionをラムダで作成
                    // .Action<ChargeToPlayerAction>(boss, player, 50000.0f, 0.5f)
                .End()
                
                // === 遠距離（10m以上）: 追跡 ===
                // TODO: ChaseActionを追加
                
                // === デフォルト: 待機 ===
                // TODO: IdleActionを追加
            .End();
        },
        "ComplexBossAI"
    );
}

// ===================================================================
// フェーズシステムを持つボスAIの例
// ===================================================================

std::unique_ptr<BehaviorTree> CreatePhasedBossAI(
    Boss* boss, 
    Player* player,
    std::function<float()> getBossHp) {
    
    return BehaviorTreeFactory::Create(
        [boss, player, getBossHp](BehaviorTreeBuilder& builder) {
            builder.Selector()
                // === フェーズ3: HP 0-33% - 狂暴化 ===
                .Sequence()
                    .Condition([getBossHp]() {
                        return getBossHp() <= 0.33f;
                    })
                    .WeightedSelector()
                        // 連続突進
                        .Repeater(3)  // 3回繰り返す
                            // ChargeAction
                        .End()
                        // 範囲攻撃
                        // TODO: AOEAttackActionを追加
                    .End()
                .End()
                
                // === フェーズ2: HP 34-66% - 戦術的 ===
                .Sequence()
                    .Condition([getBossHp]() {
                        float hp = getBossHp();
                        return hp > 0.33f && hp <= 0.66f;
                    })
                    .WeightedSelector()
                        // 突進
                        // ChargeAction
                        // 近接
                        // MeleeAction
                    .End()
                .End()
                
                // === フェーズ1: HP 67-100% - 通常 ===
                .WeightedSelector()
                    // 通常攻撃パターン
                .End()
            .End();
        },
        "PhasedBossAI"
    );
}

// ===================================================================
// 動的にツリーを切り替える例
// ===================================================================

class Boss : public GameObject {
private:
    std::unique_ptr<BehaviorTree> currentTree_;
    int currentPhase_ = 1;
    
public:
    void UpdatePhase() {
        int newPhase = CalculatePhaseFromHP();
        
        if (newPhase != currentPhase_) {
            currentPhase_ = newPhase;
            
            // フェーズに応じてツリーを切り替え
            switch (currentPhase_) {
                case 1:
                    SetBehaviorTree(CreatePhase1AI());
                    break;
                case 2:
                    SetBehaviorTree(CreatePhase2AI());
                    break;
                case 3:
                    SetBehaviorTree(CreatePhase3AI());
                    break;
            }
        }
    }
    
private:
    int CalculatePhaseFromHP() {
        float hpRatio = GetHpRatio();
        if (hpRatio <= 0.33f) return 3;
        if (hpRatio <= 0.66f) return 2;
        return 1;
    }
    
    std::unique_ptr<BehaviorTree> CreatePhase1AI() {
        return BehaviorTreeFactory::Create(
            [this](BehaviorTreeBuilder& builder) {
                builder.WeightedSelector()
                    // 通常攻撃
                .End();
            },
            "Phase1AI"
        );
    }
    
    std::unique_ptr<BehaviorTree> CreatePhase2AI() {
        return BehaviorTreeFactory::Create(
            [this](BehaviorTreeBuilder& builder) {
                builder.WeightedSelector()
                    // 戦術的な攻撃
                .End();
            },
            "Phase2AI"
        );
    }
    
    std::unique_ptr<BehaviorTree> CreatePhase3AI() {
        return BehaviorTreeFactory::Create(
            [this](BehaviorTreeBuilder& builder) {
                builder.WeightedSelector()
                    // 狂暴化攻撃
                .End();
            },
            "Phase3AI"
        );
    }
};

// ===================================================================
// Evaluatorを使用した高度なAI
// ===================================================================

std::unique_ptr<BehaviorTree> CreateAdaptiveBossAI(
    Boss* boss, 
    Player* player,
    std::function<float()> getBossHp,
    std::function<float()> getBattleTime) {
    
    return BehaviorTreeFactory::Create(
        [boss, player, getBossHp, getBattleTime](BehaviorTreeBuilder& builder) {
            builder.WeightedSelector()
                // 距離とHPを考慮した突進攻撃
                .WeightedNode(
                    // std::make_unique<ChargeToPlayerAction>(boss, player),
                    nullptr,  // 仮のnullptr
                    MakeDistanceHpCompositeEvaluator(
                        &boss->GetWorldPosition(),
                        &player->GetWorldPosition(),
                        getBossHp,
                        5.0f, 15.0f,
                        CompositeEvaluator::CombineMode::Product
                    )
                )
                
                // 時間経過で強化される攻撃
                .WeightedNode(
                    // std::make_unique<PowerfulAttackAction>(boss, player),
                    nullptr,  // 仮のnullptr
                    MakePhaseBasedEvaluator(
                        &boss->GetWorldPosition(),
                        &player->GetWorldPosition(),
                        getBossHp,
                        getBattleTime,
                        180.0f
                    )
                )
            .End();
        },
        "AdaptiveBossAI"
    );
}

*/
