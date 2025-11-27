#pragma once
#include "BehaviorTreeBuilder.h"
#include "Evaluator.h"
#include <memory>

// ボスAI作成のサンプルクラス
// 実際のゲームに合わせて修正して使用してください

class BossAIExamples {
public:
    // 例1: シンプルな3段階フェーズボス
    static std::unique_ptr<BaseNode> CreateThreePhaseBoSS(
        const Vector3* bossPos,
        const Vector3* playerPos,
        std::function<float()> getBossHp) {
        
        return BehaviorTreeBuilder()
            .Selector()
                // === フェーズ3: 狂暴化 (HP 0-33%) ===
                .ConditionalSequence([getBossHp]() { return getBossHp() <= 0.33f; })
                    .WeightedSelector()
                        // 近距離突進（距離とHPの複合評価）
                        .WeightedNode(
                            nullptr, // ここに実際のアクションノードを配置
                            MakeDistanceHpCompositeEvaluator(
                                bossPos, playerPos, getBossHp,
                                2.0f, 10.0f,
                                CompositeEvaluator::CombineMode::Product
                            )
                        )
                    .End()
                .End()
                
                // === フェーズ2: 戦術的 (HP 34-66%) ===
                .ConditionalSequence([getBossHp]() {
                    float hp = getBossHp();
                    return hp > 0.33f && hp <= 0.66f;
                })
                    .WeightedSelector()
                        // 中距離攻撃
                        .WeightedNode(nullptr, 0.7f)
                        // 防御
                        .WeightedNode(nullptr, 0.5f)
                    .End()
                .End()
                
                // === フェーズ1: 通常 (HP 67-100%) ===
                .WeightedSelector()
                    .WeightedNode(nullptr, 0.8f) // 通常攻撃
                    .WeightedNode(nullptr, 0.3f) // 巡回
                .End()
            .End()
            .Build();
    }

    // 例2: 距離ベース行動選択ボス
    static std::unique_ptr<BaseNode> CreateDistanceBasedBoss(
        const Vector3* bossPos,
        const Vector3* playerPos) {
        
        return BehaviorTreeBuilder()
            .Selector()
                // 近距離 (0-5m): 近接攻撃
                .Sequence()
                    .Condition([bossPos, playerPos]() {
                        Vector3 diff = *playerPos - *bossPos;
                        float distance = MathCore::Vector::Length(diff);
                        return distance <= 5.0f;
                    })
                    // .Action<MeleeAttackAction>()  // 実際のアクションを追加
                .End()
                
                // 中距離 (5-15m): 遠距離攻撃
                .Sequence()
                    .Condition([bossPos, playerPos]() {
                        Vector3 diff = *playerPos - *bossPos;
                        float distance = MathCore::Vector::Length(diff);
                        return distance > 5.0f && distance <= 15.0f;
                    })
                    // .Action<RangedAttackAction>()  // 実際のアクションを追加
                .End()
                
                // 遠距離 (15m以上): 追跡
                // .Action<ChaseAction>()  // 実際のアクションを追加
            .End()
            .Build();
    }

    // 例3: 時間経過で強化されるボス
    static std::unique_ptr<BaseNode> CreateTimeBasedBoss(
        const Vector3* bossPos,
        const Vector3* playerPos,
        std::function<float()> getBattleTime) {
        
        return BehaviorTreeBuilder()
            .Selector()
                // 2分経過後: 強化モード
                .Sequence()
                    .Condition([getBattleTime]() { 
                        return getBattleTime() > 120.0f; 
                    })
                    .WeightedSelector()
                        // 時間経過で評価値上昇
                        .WeightedNode(
                            nullptr, // 強化攻撃
                            MakeTimeBasedEvaluator(
                                0.3f, 1.0f, getBattleTime, 180.0f
                            )
                        )
                    .End()
                .End()
                
                // 通常モード
                .WeightedSelector()
                    .WeightedNode(nullptr, 0.6f)
                .End()
            .End()
            .Build();
    }

    // 例4: 視界と距離を考慮するステルスボス
    static std::unique_ptr<BaseNode> CreateStealthBoss(
        const Vector3* bossPos,
        const Vector3* playerPos,
        std::function<float()> getAngleToPlayer) {
        
        return BehaviorTreeBuilder()
            .Selector()
                // プレイヤーが視界内かつ適切な距離
                .Sequence()
                    .Condition([bossPos, playerPos, getAngleToPlayer]() {
                        // 視界チェック
                        float angle = getAngleToPlayer();
                        if (angle < -60.0f || angle > 60.0f) return false;
                        
                        // 距離チェック (5-12m)
                        Vector3 diff = *playerPos - *bossPos;
                        float distance = MathCore::Vector::Length(diff);
                        return distance >= 5.0f && distance <= 12.0f;
                    })
                    // .Action<StealthAttackAction>()
                .End()
                
                // 視界外: 潜伏・移動
                // .Action<StealthMoveAction>()
            .End()
            .Build();
    }

    // 例5: 攻撃カウンターシステムを持つボス
    static std::unique_ptr<BaseNode> CreateCounterBasedBoss(
        std::function<int()> getComboCount,
        std::function<float()> getCooldownTime) {
        
        return BehaviorTreeBuilder()
            .Selector()
                // クールダウン中
                .Sequence()
                    .Condition([getCooldownTime]() { 
                        return getCooldownTime() > 0.0f; 
                    })
                    // .Action<WaitAction>()
                .End()
                
                // 3連続攻撃後は休憩
                .ConditionalSequence([getComboCount]() { 
                    return getComboCount() >= 3; 
                })
                    // .Action<RestAction>()
                .End()
                
                // 通常攻撃（カウントに応じて強度変化）
                .WeightedSelector()
                    // 軽攻撃（コンボ初期に高評価）
                    .WeightedNode(
                        nullptr,
                        MakeCounterEvaluator(
                            1.0f, 0.2f, getComboCount, 0, 3
                        )
                    )
                    // 重攻撃（コンボ後半に高評価）
                    .WeightedNode(
                        nullptr,
                        MakeCounterEvaluator(
                            0.2f, 1.0f, getComboCount, 0, 3
                        )
                    )
                .End()
            .End()
            .Build();
    }

    // 例6: ランダム性を持つ予測困難なボス
    static std::unique_ptr<BaseNode> CreateRandomBoss(
        const Vector3* bossPos,
        const Vector3* playerPos) {
        
        return BehaviorTreeBuilder()
            .WeightedSelector()
                // 各攻撃にランダム性を追加
                .WeightedNode(
                    nullptr, // 攻撃1
                    MakeRandomEvaluator(0.3f, 0.8f)
                )
                .WeightedNode(
                    nullptr, // 攻撃2
                    MakeRandomEvaluator(0.4f, 0.9f)
                )
                .WeightedNode(
                    nullptr, // 攻撃3
                    MakeRandomEvaluator(0.2f, 0.7f)
                )
            .End()
            .Build();
    }

    // 例7: 複雑な複合評価を使用する高難易度ボス
    static std::unique_ptr<BaseNode> CreateAdvancedBoss(
        const Vector3* bossPos,
        const Vector3* playerPos,
        std::function<float()> getBossHp,
        std::function<float()> getBattleTime,
        std::function<float()> getAngleToPlayer,
        std::function<int()> getPlayerComboCount) {
        
        return BehaviorTreeBuilder()
            .Selector()
                // 必殺技発動条件（複数条件の組み合わせ）
                .Sequence()
                    .Condition([getBossHp, getBattleTime, bossPos, playerPos]() {
                        // HPが30%以下
                        if (getBossHp() > 0.3f) return false;
                        
                        // 戦闘時間が1分以上
                        if (getBattleTime() < 60.0f) return false;
                        
                        // プレイヤーが近距離
                        Vector3 diff = *playerPos - *bossPos;
                        float distance = MathCore::Vector::Length(diff);
                        return distance <= 8.0f;
                    })
                    // .Action<UltimateAttackAction>()
                .End()
                
                // 適応的な行動選択
                .WeightedSelector()
                    // 距離・HP・時間を統合評価
                    .WeightedNode(
                        nullptr,
                        MakePhaseBasedEvaluator(
                            bossPos, playerPos, getBossHp, getBattleTime, 180.0f
                        )
                    )
                    
                    // 視界と距離の複合評価
                    .WeightedNode(
                        nullptr,
                        MakeVisibilityAndRangeEvaluator(
                            bossPos, playerPos, getAngleToPlayer, 10.0f, 60.0f
                        )
                    )
                    
                    // プレイヤーの攻撃頻度に応じた防御
                    .WeightedNode(
                        nullptr,
                        MakeCounterEvaluator(
                            0.2f, 1.0f, getPlayerComboCount, 0, 5
                        )
                    )
                .End()
            .End()
            .Build();
    }

    // 例8: ヘルパーメソッドを活用した簡潔な記述
    static std::unique_ptr<BaseNode> CreateHelperBasedBoss(
        const Vector3* bossPos,
        const Vector3* playerPos,
        std::function<float()> getBossHp) {
        
        return BehaviorTreeBuilder()
            .Selector()
                // 条件付きアクションヘルパーを使用
                // .ConditionalAction<DesperationAttack>(
                //     [getBossHp]() { return getBossHp() <= 0.2f; }
                // )
                
                // 距離ベースアクションヘルパーを使用
                // .DistanceBasedAction<MeleeAttack>(
                //     bossPos, playerPos, 0.0f, 5.0f
                // )
                
                // HPベースアクションヘルパーを使用
                // .HpBasedAction<HealAction>(
                //     getBossHp, 0.3f, 0.5f
                // )
                
                // デフォルト行動
                // .Action<IdleAction>()
            .End()
            .Build();
    }

    // 例9: ループとリピートを活用したパターン攻撃
    static std::unique_ptr<BaseNode> CreatePatternBoss() {
        return BehaviorTreeBuilder()
            .Sequence()
                // 3回繰り返すコンボ攻撃
                .LoopSequence(3)
                    // .Action<ComboAttack1>()
                    .Wait(0.5f)  // 攻撃間隔
                    // .Action<ComboAttack2>()
                    .Wait(0.5f)
                .End()
                .End()
                
                // クールダウン
                .Wait(2.0f)
                
                // 必殺技
                // .Action<FinisherAttack>()
            .End()
            .Build();
    }

    // 例10: デコレーターを活用した制御
    static std::unique_ptr<BaseNode> CreateDecoratorBoss() {
        return BehaviorTreeBuilder()
            .Selector()
                // 攻撃が失敗しても続行（Succeeder）
                .Succeeder()
                    // .Action<RiskyAttack>()
                .End()
                
                // 成功するまでリトライ（Retry）
                .Retry()
                    // .Action<ConnectionAttack>()
                .End()
                
                // 条件を反転（Inverter）
                .Inverter()
                    // .Condition([]() { return IsPlayerNear(); })
                .End()
            .End()
            .Build();
    }
};
