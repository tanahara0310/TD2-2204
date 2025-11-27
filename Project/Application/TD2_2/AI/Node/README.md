# ビヘイビアツリーAIシステム - 拡張機能

このドキュメントでは、ボス戦AI構築のために追加された拡張Evaluatorとヘルパーメソッドについて説明します。

## ?? 目次

1. [新規追加Evaluator](#新規追加evaluator)
2. [複合Evaluatorヘルパー](#複合evaluatorヘルパー)
3. [BehaviorTreeBuilderヘルパーメソッド](#behaviortreebuilderヘルパーメソッド)
4. [クイックスタート](#クイックスタート)
5. [実用例](#実用例)

---

## ?? 新規追加Evaluator

### 基本Evaluator

| Evaluator | 説明 | 用途 |
|-----------|------|------|
| `TimeBasedEvaluator` | 時間経過に基づく評価 | 戦闘時間による難易度調整 |
| `AngleEvaluator` | 角度範囲に基づく評価 | 視界判定、背後攻撃判定 |
| `CounterEvaluator` | カウンター値に基づく評価 | コンボ数、攻撃回数管理 |
| `RandomEvaluator` | ランダム評価値 | 予測困難な行動パターン |
| `CurveEvaluator` | イージング曲線評価 | 滑らかな値の変化 |

### 作成ヘルパー関数

```cpp
// 時間ベース
auto timeEval = MakeTimeBasedEvaluator(
    0.5f, 1.0f,  // 開始値→終了値
    [&boss]() { return boss.GetBattleTime(); },
    180.0f  // 経過時間（秒）
);

// 角度ベース（視界判定）
auto angleEval = MakeAngleEvaluator(
    1.0f, 0.0f,  // 範囲内→範囲外
    [&boss, &player]() { return boss.GetAngleToTarget(&player); },
    -45.0f, 45.0f  // 視野角
);

// カウンターベース
auto counterEval = MakeCounterEvaluator(
    0.0f, 1.0f,  // 最小値→最大値
    [&boss]() { return boss.GetAttackCount(); },
    0, 5  // カウント範囲
);

// ランダム
auto randomEval = MakeRandomEvaluator(0.3f, 0.8f);

// カーブ（イージング）
auto curveEval = MakeCurveEvaluator(
    0.0f, 1.0f,
    [&boss]() { return boss.GetPhaseProgress(); },
    CurveEvaluator::CurveType::EaseInOut
);
```

---

## ?? 複合Evaluatorヘルパー

### 1. MakeDistanceHpCompositeEvaluator

距離とHP割合を組み合わせた評価（近接攻撃判定に最適）

```cpp
auto eval = MakeDistanceHpCompositeEvaluator(
    &boss.pos, &player.pos,
    [&boss]() { return boss.GetHpRatio(); },
    5.0f,   // 近距離
    15.0f,  // 遠距離
    CompositeEvaluator::CombineMode::Product  // 積算
);

// 使用例：近くてHPが高い時に近接攻撃の評価値が高くなる
builder.WeightedAction<MeleeAttack>(std::move(eval));
```

**評価モード：**
- `Product`: 両方の条件を満たす必要がある（AND的）
- `Sum`: どちらかの条件を満たせば良い（OR的）
- `WeightedSum`: 重み付き平均

### 2. MakePhaseBasedEvaluator

距離、HP、時間を統合した段階的難易度調整

```cpp
auto eval = MakePhaseBasedEvaluator(
    &boss.pos, &player.pos,
    [&boss]() { return boss.GetHpRatio(); },
    [&boss]() { return boss.GetBattleTime(); },
    180.0f  // 最大戦闘時間
);

// 距離30%、HP40%、時間30%の重み付け評価
// 戦闘が進むほど攻撃的になる
builder.WeightedAction<AggressiveAttack>(std::move(eval));
```

### 3. MakeVisibilityAndRangeEvaluator

視界内かつ適切な距離の複合判定

```cpp
auto eval = MakeVisibilityAndRangeEvaluator(
    &boss.pos, &player.pos,
    [&boss, &player]() { return boss.GetAngleToTarget(&player); },
    10.0f,  // 最適距離
    60.0f   // 視野角
);

// 視界内で適切な距離にいる場合に遠距離攻撃
builder.WeightedAction<RangedAttack>(std::move(eval));
```

---

## ??? BehaviorTreeBuilderヘルパーメソッド

### 簡易構築ヘルパー

#### QuickSequence / QuickSelector

```cpp
// 従来の記法
builder.Sequence()
    .Action<Move>()
    .Action<Attack>()
    .Action<Retreat>()
    .End();

// QuickSequenceを使用（より簡潔）
builder.QuickSequence(
    std::make_unique<Move>(),
    std::make_unique<Attack>(),
    std::make_unique<Retreat>()
);
```

### 条件付きヘルパー

#### ConditionalSequence

```cpp
// 条件を満たした場合のみ実行されるシーケンス
builder.ConditionalSequence([&boss, &player]() {
    return boss.CanSeePlayer(&player);
})
    .Action<Chase>()
    .Action<Attack>()
    .End();
```

#### ConditionalAction

```cpp
// 1行で条件付きアクションを記述
builder.ConditionalAction<HeavyAttack>(
    [&boss]() { return boss.GetStamina() > 50.0f; },
    attackParams
);
```

### 距離・HP条件ヘルパー

#### DistanceBasedAction

```cpp
// 特定の距離範囲でのみ実行
builder.DistanceBasedAction<RangedAttack>(
    &boss.pos, &player.pos,
    5.0f, 10.0f,  // 5～10mの範囲
    rangedParams
);
```

#### HpBasedAction

```cpp
// HP割合に応じた行動
builder.HpBasedAction<DesperationAttack>(
    [&boss]() { return boss.GetHpRatio(); },
    0.0f, 0.3f,  // HP30%以下
    desperationParams
);
```

### 制御フローヘルパー

#### LoopSequence

```cpp
// N回繰り返すシーケンス
builder.LoopSequence(3)
    .Action<ComboAttack1>()
    .Action<ComboAttack2>()
    .End()
    .End();
```

#### RandomSelector / PrioritySelector

```cpp
// ランダムセレクター（均等確率）
builder.RandomSelector()
    .WeightedAction<Attack1>(1.0f)
    .WeightedAction<Attack2>(1.0f)
    .WeightedAction<Attack3>(1.0f)
    .End();

// 優先度セレクター（上から順に評価）
builder.PrioritySelector()
    .Action<HighPriorityAction>()
    .Action<MediumPriorityAction>()
    .Action<LowPriorityAction>()
    .End();
```

---

## ?? クイックスタート

### 最小構成のボスAI

```cpp
auto CreateSimpleBoss(const Vector3* bossPos, const Vector3* playerPos) {
    return BehaviorTreeBuilder()
        .Selector()
            // 近距離：近接攻撃
            .DistanceBasedAction<MeleeAttack>(
                bossPos, playerPos, 0.0f, 5.0f
            )
            
            // 中距離：遠距離攻撃
            .DistanceBasedAction<RangedAttack>(
                bossPos, playerPos, 5.0f, 15.0f
            )
            
            // 遠距離：追跡
            .Action<Chase>()
        .End()
        .Build();
}
```

### 3段階フェーズボス

```cpp
auto CreatePhaseBoSS(
    const Vector3* bossPos, 
    const Vector3* playerPos,
    std::function<float()> getHpRatio) {
    
    return BehaviorTreeBuilder()
        .Selector()
            // フェーズ3: HP 0-33%
            .ConditionalSequence([getHpRatio]() { 
                return getHpRatio() <= 0.33f; 
            })
                .WeightedSelector()
                    .WeightedAction<BerserkAttack>(1.0f)
                    .WeightedAction<DespAttack>(0.8f)
                .End()
            .End()
            
            // フェーズ2: HP 34-66%
            .ConditionalSequence([getHpRatio]() { 
                float hp = getHpRatio();
                return hp > 0.33f && hp <= 0.66f; 
            })
                .WeightedSelector()
                    .WeightedAction<TacticalAttack>(0.7f)
                    .WeightedAction<DefenseAction>(0.5f)
                .End()
            .End()
            
            // フェーズ1: HP 67-100%
            .WeightedSelector()
                .WeightedAction<NormalAttack>(0.6f)
                .WeightedAction<Patrol>(0.3f)
            .End()
        .End()
        .Build();
}
```

---

## ?? 実用例

詳細な使用例は以下のファイルを参照してください：

- **`BehaviorTreeUsageExamples.md`**: 詳細な使用例とベストプラクティス
- **`BossAIExamples.h`**: 10種類の実装例

### 含まれる例

1. シンプルな3段階フェーズボス
2. 距離ベース行動選択ボス
3. 時間経過で強化されるボス
4. 視界と距離を考慮するステルスボス
5. 攻撃カウンターシステムを持つボス
6. ランダム性を持つ予測困難なボス
7. 複雑な複合評価を使用する高難易度ボス
8. ヘルパーメソッドを活用した簡潔な記述
9. ループとリピートを活用したパターン攻撃
10. デコレーターを活用した制御

---

## ?? ベストプラクティス

### 1. Evaluatorの再利用

同じ評価を複数箇所で使う場合は変数に保存

```cpp
auto distanceEval = MakeDistanceEvaluator(
    1.0f, 0.0f, &boss.pos, &player.pos, 5.0f, 15.0f
);

// 複数のアクションで同じ評価を使用
builder.WeightedAction<Attack1>(distanceEval->Evaluate())
       .WeightedAction<Attack2>(distanceEval->Evaluate() * 0.8f);
```

### 2. 複合Evaluatorの階層化

複雑な評価は段階的に構築

```cpp
auto masterEval = MakeCompositeEvaluator(
    CompositeEvaluator::CombineMode::WeightedSum
);

masterEval->AddEvaluator(distanceEval, 0.4f);  // 40%
masterEval->AddEvaluator(hpEval, 0.3f);        // 30%
masterEval->AddEvaluator(timeEval, 0.3f);      // 30%
```

### 3. デバッグのヒント

Evaluator値をログ出力してバランス調整

```cpp
auto debugEval = std::make_unique<LambdaEvaluator>([eval = std::move(masterEval)]() {
    float value = eval->Evaluate();
    // デバッグ出力
    std::cout << "Eval: " << value << std::endl;
    return value;
});
```

---

## ?? パフォーマンス最適化

### キャッシング

頻繁に評価される値はキャッシュして計算を削減

```cpp
class CachedEvaluator : public IEvaluator {
    mutable float cachedValue_ = 0.0f;
    mutable int frameCount_ = 0;
    
    float Evaluate() const override {
        if (frameCount_++ % 5 == 0) {  // 5フレームごとに更新
            cachedValue_ = CalculateValue();
        }
        return cachedValue_;
    }
};
```

### 条件の早期リターン

重い計算は条件が満たされた場合のみ実行

```cpp
builder.Sequence()
    // 軽い条件を先に評価
    .Condition([&boss]() { return boss.IsAlive(); })
    .Condition([&boss]() { return boss.GetStamina() > 50.0f; })
    // 重い計算はここで実行
    .Action<ExpensiveAction>()
    .End();
```

---

## ?? まとめ

このシステムにより以下が可能になりました：

? **柔軟な評価システム**: 6種類の基本Evaluator + 3種類の複合Evaluator  
? **簡潔な記述**: 10種類以上のヘルパーメソッド  
? **再利用性**: テンプレートと関数型プログラミング  
? **拡張性**: 新しいEvaluatorやヘルパーの追加が容易  
? **可読性**: 意図が明確なAPI設計  

これらのツールを組み合わせることで、複雑なボスAIを効率的に構築できます。

---

## ?? 関連ファイル

- `Evaluator.h/cpp`: Evaluatorクラス定義
- `BehaviorTreeBuilder.h/cpp`: ビルダークラス定義
- `BehaviorTreeUsageExamples.md`: 詳細な使用例
- `BossAIExamples.h`: 実装サンプル集
