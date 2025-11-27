# ビヘイビアツリー使用例とヘルパー関数ガイド

## 目次
1. [新しいEvaluator一覧](#新しいevaluator一覧)
2. [複合Evaluatorヘルパー](#複合evaluatorヘルパー)
3. [BehaviorTreeBuilderヘルパーメソッド](#behaviortreebuilderヘルパーメソッド)
4. [実践例：ボス戦AI](#実践例ボス戦ai)

---

## 新しいEvaluator一覧

### 1. TimeBasedEvaluator
時間経過に基づいて評価値を変化させます。

```cpp
// 戦闘開始から180秒で評価値が0.5から1.0に上昇
auto timeEval = MakeTimeBasedEvaluator(
    0.5f,  // 開始値
    1.0f,  // 終了値
    [&boss]() { return boss.GetBattleTime(); },
    180.0f  // 経過時間
);
```

### 2. AngleEvaluator
角度範囲に基づいて評価します（視界判定に使用）。

```cpp
// 視界内（±45度）にいる場合は1.0、範囲外は0.0
auto angleEval = MakeAngleEvaluator(
    1.0f,   // 範囲内の値
    0.0f,   // 範囲外の値
    [&boss, &player]() { 
        return boss.GetAngleToTarget(&player); 
    },
    -45.0f,  // 最小角度
    45.0f    // 最大角度
);
```

### 3. CounterEvaluator
カウンター値に基づいて評価します（攻撃回数、連続ヒット数など）。

```cpp
// 攻撃回数が0～5回で評価値が0.0～1.0に変化
auto counterEval = MakeCounterEvaluator(
    0.0f,  // 最小値
    1.0f,  // 最大値
    [&boss]() { return boss.GetAttackCount(); },
    0,     // 最小カウント
    5      // 最大カウント
);
```

### 4. RandomEvaluator
ランダムな評価値を返します。

```cpp
// 0.0～1.0のランダム値
auto randomEval = MakeRandomEvaluator(0.0f, 1.0f);
```

### 5. CurveEvaluator
イージング関数を使用した滑らかな変化。

```cpp
// 進行度に応じてEaseInカーブで変化
auto curveEval = MakeCurveEvaluator(
    0.0f,  // 開始値
    1.0f,  // 終了値
    [&boss]() { return boss.GetPhaseProgress(); },
    CurveEvaluator::CurveType::EaseIn
);
```

---

## 複合Evaluatorヘルパー

### 1. MakeDistanceHpCompositeEvaluator
距離とHP割合を組み合わせた評価（近接攻撃判定用）。

```cpp
auto compositeEval = MakeDistanceHpCompositeEvaluator(
    &boss.GetPosition(),
    &player.GetPosition(),
    [&boss]() { return boss.GetHpRatio(); },
    5.0f,   // 近距離閾値
    15.0f,  // 遠距離閾値
    CompositeEvaluator::CombineMode::Product  // 積算モード
);

// 使用例：近くてHPが高い時に近接攻撃
builder.WeightedAction<MeleeAttackAction>(std::move(compositeEval), attackParams);
```

### 2. MakePhaseBasedEvaluator
距離、HP、時間を組み合わせた段階的難易度調整。

```cpp
auto phaseEval = MakePhaseBasedEvaluator(
    &boss.GetPosition(),
    &player.GetPosition(),
    [&boss]() { return boss.GetHpRatio(); },
    [&boss]() { return boss.GetBattleTime(); },
    180.0f  // 最大戦闘時間
);

// 距離30%、HP40%、時間30%の重み付け評価
builder.WeightedAction<SpecialAttackAction>(std::move(phaseEval));
```

### 3. MakeVisibilityAndRangeEvaluator
視界内かつ適切な距離にいるかを評価。

```cpp
auto visibilityEval = MakeVisibilityAndRangeEvaluator(
    &boss.GetPosition(),
    &player.GetPosition(),
    [&boss, &player]() { 
        return boss.CalculateAngleToTarget(&player); 
    },
    10.0f,  // 最適距離
    60.0f   // 視野角
);

// 視界内で適切な距離にいる場合に遠距離攻撃
builder.WeightedAction<RangedAttackAction>(std::move(visibilityEval));
```

---

## BehaviorTreeBuilderヘルパーメソッド

### 1. QuickSequence - 簡易シーケンス作成

```cpp
// 従来の記法
builder.Sequence()
    .Action<MoveAction>()
    .Action<AttackAction>()
    .Action<RetreatAction>()
    .End();

// QuickSequenceを使用
builder.QuickSequence(
    std::make_unique<MoveAction>(),
    std::make_unique<AttackAction>(),
    std::make_unique<RetreatAction>()
);
```

### 2. ConditionalSequence - 条件付きシーケンス

```cpp
// 条件を満たした場合のみ実行されるシーケンス
builder.ConditionalSequence([&boss, &player]() {
    return boss.CanSeePlayer(&player);
})
    .Action<ChaseAction>()
    .Action<AttackAction>()
    .End();
```

### 3. ConditionalAction - 条件付きアクション

```cpp
// 条件を満たした場合のみアクションを実行
builder.ConditionalAction<HeavyAttackAction>(
    [&boss]() { return boss.GetStamina() > 50.0f; },
    attackParams
);
```

### 4. DistanceBasedAction - 距離ベースアクション

```cpp
// 5～10m の距離範囲内でのみ実行
builder.DistanceBasedAction<RangedAttackAction>(
    &boss.GetPosition(),
    &player.GetPosition(),
    5.0f,   // 最小距離
    10.0f,  // 最大距離
    rangedParams
);
```

### 5. HpBasedAction - HPベースアクション

```cpp
// HP30%以下で発動する必殺技
builder.HpBasedAction<DesperationAttackAction>(
    [&boss]() { return boss.GetHpRatio(); },
    0.0f,   // 最小HP
    0.3f,   // 最大HP
    desperationParams
);
```

### 6. LoopSequence - ループシーケンス

```cpp
// 3回繰り返すコンボ攻撃
builder.LoopSequence(3)
    .Action<ComboAttack1>()
    .Action<ComboAttack2>()
    .Action<ComboAttack3>()
    .End()
    .End();
```

### 7. RandomSelector - ランダムセレクター

```cpp
// 均等確率でランダムに選択
builder.RandomSelector()
    .WeightedAction<Attack1>(1.0f)
    .WeightedAction<Attack2>(1.0f)
    .WeightedAction<Attack3>(1.0f)
    .End();
```

---

## 実践例：ボス戦AI

### 例1: 3段階フェーズシステム

```cpp
auto bossTree = BehaviorTreeBuilder()
    .Selector()
        // フェーズ3: HP 0-33% - 狂暴化
        .ConditionalSequence([&boss]() { return boss.GetHpRatio() <= 0.33f; })
            .WeightedSelector()
                .WeightedAction<BerserkRushAction>(
                    MakeDistanceHpCompositeEvaluator(
                        &boss.GetPosition(), &player.GetPosition(),
                        [&boss]() { return boss.GetHpRatio(); },
                        3.0f, 15.0f
                    )
                )
                .WeightedAction<DesperationBeamAction>(0.8f)
                .WeightedAction<AoEExplosionAction>(0.7f)
            .End()
        .End()
        
        // フェーズ2: HP 34-66% - 戦術的
        .ConditionalSequence([&boss]() { 
            float hp = boss.GetHpRatio();
            return hp > 0.33f && hp <= 0.66f; 
        })
            .WeightedSelector()
                .WeightedAction<TacticalRetreatAction>(
                    MakeDistanceEvaluator(0.2f, 0.8f, 
                        &boss.GetPosition(), &player.GetPosition(),
                        2.0f, 8.0f)
                )
                .WeightedAction<ComboAttackAction>(0.6f)
                .WeightedAction<GuardAction>(0.4f)
            .End()
        .End()
        
        // フェーズ1: HP 67-100% - 通常
        .WeightedSelector()
            .WeightedAction<NormalAttackAction>(
                MakeVisibilityAndRangeEvaluator(
                    &boss.GetPosition(), &player.GetPosition(),
                    [&boss, &player]() { return boss.GetAngleToTarget(&player); },
                    7.0f, 45.0f
                )
            )
            .WeightedAction<PatrolAction>(0.3f)
        .End()
    .End()
    .Build();
```

### 例2: 動的難易度調整システム

```cpp
auto adaptiveBossTree = BehaviorTreeBuilder()
    .Selector()
        // 時間経過で強化
        .Sequence()
            .Condition([&boss]() { return boss.GetBattleTime() > 120.0f; })
            .WeightedSelector()
                .WeightedAction<EnragedAttackAction>(
                    MakePhaseBasedEvaluator(
                        &boss.GetPosition(), &player.GetPosition(),
                        [&boss]() { return boss.GetHpRatio(); },
                        [&boss]() { return boss.GetBattleTime(); },
                        180.0f
                    )
                )
                .WeightedAction<PowerUpAction>(0.5f)
            .End()
        .End()
        
        // 距離ベース行動選択
        .Selector()
            .DistanceBasedAction<MeleeComboAction>(
                &boss.GetPosition(), &player.GetPosition(),
                0.0f, 5.0f, meleeParams
            )
            .DistanceBasedAction<RangedAttackAction>(
                &boss.GetPosition(), &player.GetPosition(),
                5.0f, 15.0f, rangedParams
            )
            .Action<ChaseAction>(chaseParams)
        .End()
    .End()
    .Build();
```

### 例3: カウンターシステム

```cpp
auto counterSystemTree = BehaviorTreeBuilder()
    .Selector()
        // 連続攻撃後はクールダウン
        .ConditionalAction<CooldownAction>(
            [&boss]() { return boss.GetComboCount() >= 3; },
            cooldownParams
        )
        
        // 攻撃カウントに応じた行動
        .WeightedSelector()
            .WeightedAction<LightAttackAction>(
                MakeCounterEvaluator(
                    1.0f, 0.2f,
                    [&boss]() { return boss.GetComboCount(); },
                    0, 5
                )
            )
            .WeightedAction<HeavyAttackAction>(
                MakeCounterEvaluator(
                    0.2f, 1.0f,
                    [&boss]() { return boss.GetComboCount(); },
                    0, 5
                )
            )
        .End()
    .End()
    .Build();
```

### 例4: 視界と距離を考慮したステルスボス

```cpp
auto stealthBossTree = BehaviorTreeBuilder()
    .Selector()
        // プレイヤーが視界内かつ適切な距離
        .Sequence()
            .Condition([&boss, &player]() {
                auto eval = MakeVisibilityAndRangeEvaluator(
                    &boss.GetPosition(), &player.GetPosition(),
                    [&boss, &player]() { return boss.GetAngleToTarget(&player); },
                    8.0f, 60.0f
                );
                return eval->Evaluate() > 0.8f;
            })
            .WeightedSelector()
                .WeightedAction<StealthAttackAction>(0.8f)
                .WeightedAction<NormalAttackAction>(0.5f)
            .End()
        .End()
        
        // 視界外：潜伏
        .Action<HideAction>(hideParams)
    .End()
    .Build();
```

---

## ベストプラクティス

### 1. Evaluatorの再利用
```cpp
// 共通のEvaluatorを作成して再利用
auto distanceEval = MakeDistanceEvaluator(
    1.0f, 0.0f, &boss.GetPosition(), &player.GetPosition(), 5.0f, 15.0f
);

builder.WeightedSelector()
    .WeightedNode(std::make_unique<Attack1>(), distanceEval->Evaluate())
    .WeightedNode(std::make_unique<Attack2>(), distanceEval->Evaluate() * 0.8f)
    .End();
```

### 2. 複合Evaluatorの階層化
```cpp
auto masterEval = MakeCompositeEvaluator(CompositeEvaluator::CombineMode::WeightedSum);

// サブEvaluatorを追加
masterEval->AddEvaluator(
    MakeDistanceEvaluator(1.0f, 0.0f, &boss.pos, &player.pos, 5.0f, 15.0f),
    0.4f  // 40%の重み
);

masterEval->AddEvaluator(
    MakeHpRatioEvaluator(1.0f, 0.2f, [&boss]() { return boss.GetHpRatio(); }),
    0.3f  // 30%の重み
);

masterEval->AddEvaluator(
    MakeTimeBasedEvaluator(0.5f, 1.0f, [&boss]() { return boss.GetBattleTime(); }, 180.0f),
    0.3f  // 30%の重み
);
```

### 3. デバッグ用ラッパー
```cpp
// Evaluator値をログ出力するラッパー
auto debugEval = std::make_unique<LambdaEvaluator>([eval = std::move(masterEval)]() {
    float value = eval->Evaluate();
    // デバッグ出力
    // std::cout << "Eval: " << value << std::endl;
    return value;
});
```

---

## パフォーマンス最適化のヒント

1. **Evaluatorのキャッシング**: 頻繁に評価される値はキャッシュする
2. **条件の早期リターン**: 重い計算は条件が満たされた場合のみ実行
3. **ノードの再利用**: 同じ構造のサブツリーは共有する
4. **評価頻度の調整**: 毎フレーム評価する必要がない場合は間引く

```cpp
// キャッシング例
class CachedDistanceEvaluator : public IEvaluator {
    mutable float cachedValue_ = 0.0f;
    mutable int frameCount_ = 0;
    
    float Evaluate() const override {
        // 5フレームごとに再計算
        if (frameCount_++ % 5 == 0) {
            cachedValue_ = CalculateDistance();
        }
        return cachedValue_;
    }
};
```
