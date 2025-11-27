# ボスAIシステム 完全設計ドキュメント

## ??? アーキテクチャ概要

このシステムは**3層構造**で設計されています：

```
┌─────────────────────────────────────────────────────────┐
│                    GameScene（使用層）                   │
│  - ボスとプレイヤーの初期化                              │
│  - BehaviorTreeの構築と設定                             │
│  - ゲームループの管理                                    │
└───────────────────────┬─────────────────────────────────┘
                        │
                        ↓
┌─────────────────────────────────────────────────────────┐
│              BehaviorTree（管理層）                      │
│  - ツリーのライフサイクル管理                            │
│  - ルートノードの実行                                    │
│  - デバッグ情報の追跡                                    │
└───────────────────────┬─────────────────────────────────┘
                        │
                        ↓
┌─────────────────────────────────────────────────────────┐
│           BehaviorTreeBuilder（構築層）                  │
│  - Selector, Sequence, Parallelなどの組み立て           │
│  - 条件とアクションの接続                                │
│  - ヘルパーメソッドによる簡潔な記述                      │
└───────────────────────┬─────────────────────────────────┘
                        │
                        ↓
┌─────────────────────────────────────────────────────────┐
│          CompositeNode / DecoratorNode（論理層）         │
│  - Selector: OR論理（いずれか成功）                      │
│  - Sequence: AND論理（すべて成功）                       │
│  - Parallel: 並列実行                                    │
│  - Inverter, Repeater, Retry など                       │
└───────────────────────┬─────────────────────────────────┘
                        │
                        ↓
┌─────────────────────────────────────────────────────────┐
│            BossActionNode（実装層）                      │
│  - 具体的な行動の実装                                    │
│  - Boss/Playerとのインタラクション                       │
│  - 内部でStateMachineを使用                             │
└───────────────────────┬─────────────────────────────────┘
                        │
                        ↓
┌─────────────────────────────────────────────────────────┐
│              StateMachine（状態管理層）                  │
│  - Idle → Enter → Execute → Exit → Completed           │
│  - アクション内部の細かい状態遷移                        │
└─────────────────────────────────────────────────────────┘
```

---

## ?? クラス構成

### 1. BehaviorTree（管理クラス）

**役割**: ビヘイビアツリーのライフサイクルを管理

```cpp
class BehaviorTree {
public:
    void SetRoot(std::unique_ptr<BaseNode> root);
    NodeState Tick();
    void Reset();
    bool HasRoot() const;
    uint32_t GetTickCount() const;
    void SetName(const std::string& name);
    const std::string& GetName() const;
};
```

**特徴**:
- ? ルートノードの所有権を管理
- ? 実行回数を追跡（デバッグ用）
- ? ツリー名による識別

### 2. BehaviorTreeFactory（ファクトリクラス）

**役割**: BehaviorTreeの構築を簡潔にする

```cpp
class BehaviorTreeFactory {
public:
    static BehaviorTreeBuilder CreateBuilder();
    static std::unique_ptr<BehaviorTree> Build(
        BehaviorTreeBuilder& builder, 
        const std::string& name = "BehaviorTree"
    );
    
    template<typename BuildFunc>
    static std::unique_ptr<BehaviorTree> Create(
        BuildFunc buildFunc, 
        const std::string& name = "BehaviorTree"
    );
};
```

**使用例**:
```cpp
// ラムダ式で直接構築
auto tree = BehaviorTreeFactory::Create(
    [](BehaviorTreeBuilder& builder) {
        builder.Selector()
            // ...
        .End();
    },
    "BossAI"
);
```

### 3. Boss（ゲームオブジェクト）

**役割**: ボスエンティティとBehaviorTreeの橋渡し

```cpp
class Boss : public GameObject {
public:
    // BehaviorTree関連
    void SetBehaviorTree(std::unique_ptr<BehaviorTree> tree);
    BehaviorTree* GetBehaviorTree() const;
    
    // プレイヤー関連
    void SetPlayer(Player* player);
    Player* GetPlayer() const;
    
    // アクションノードから呼ばれるAPI
    void AddAcceleration(const Vector2& accel);
    void SetVelocity(const Vector2& vel);
    Vector2 GetVelocity() const;
    void SetMaxSpeed(float maxSpeed);
    void SetDamping(float damping);
    void ResetMovementParameters();
    
    // プレイヤー関連のユーティリティ
    float GetDistanceToPlayer() const;
    Vector3 GetDirectionToPlayer() const;
    float GetAngleToPlayer() const;
    
private:
    std::unique_ptr<BehaviorTree> behaviorTree_;
    Player* player_ = nullptr;
};
```

### 4. BossActionNode（アクション基底クラス）

**役割**: 具体的な行動の基底クラス

```cpp
class BossActionNode : public ActionNode {
public:
    enum class ActionState {
        Idle, Enter, Execute, Exit, Completed
    };
    
protected:
    Boss* boss_;
    std::unique_ptr<StateMachine> stateMachine_;
    
    virtual void OnEnter() {}
    virtual NodeState OnExecute() = 0;  // 必須実装
    virtual void OnExit() {}
};
```

---

## ?? データフロー

### 初期化フロー

```
GameScene::Initialize()
    ↓
1. Boss/Playerの生成
    ↓
2. boss_->SetPlayer(player_.get())
    ↓
3. BehaviorTreeFactory::Create()
    ├── BehaviorTreeBuilder::Selector()
    ├── BehaviorTreeBuilder::Sequence()
    └── BehaviorTreeBuilder::Action<T>()
    ↓
4. boss_->SetBehaviorTree(tree)
```

### 実行フロー（毎フレーム）

```
GameScene::Update()
    ↓
Boss::Update()
    ↓
BehaviorTree::Tick()
    ↓
BaseNode::Tick() (ルートノード)
    ↓
SelectorNode::Tick() / SequenceNode::Tick()
    ↓
BossActionNode::Tick()
    ↓
StateMachine::Update()
    ├── OnEnter()
    ├── OnExecute()
    └── OnExit()
    ↓
Boss::AddAcceleration() / Boss::SetMaxSpeed() など
    ↓
Boss::UpdateMovement()
```

---

## ?? 実装パターン

### パターン1: シンプルな距離ベースAI

```cpp
auto tree = BehaviorTreeFactory::Create(
    [boss, player](BehaviorTreeBuilder& builder) {
        builder.Selector()
            // 近距離: 突進
            .Sequence()
                .Condition([boss]() {
                    return boss->GetDistanceToPlayer() <= 10.0f;
                })
                .Action<ChargeToPlayerAction>(boss, player)
            .End()
            
            // 遠距離: 追跡
            .Action<ChaseAction>(boss, player)
        .End();
    },
    "SimpleBossAI"
);
```

### パターン2: フェーズシステム

```cpp
auto tree = BehaviorTreeFactory::Create(
    [boss, player, getBossHp](BehaviorTreeBuilder& builder) {
        builder.Selector()
            // フェーズ3
            .ConditionalSequence([getBossHp]() { 
                return getBossHp() <= 0.33f; 
            })
                // 狂暴化行動
            .End()
            
            // フェーズ2
            .ConditionalSequence([getBossHp]() { 
                float hp = getBossHp();
                return hp > 0.33f && hp <= 0.66f; 
            })
                // 戦術的行動
            .End()
            
            // フェーズ1
            // 通常行動
        .End();
    },
    "PhasedBossAI"
);
```

### パターン3: 動的ツリー切り替え

```cpp
class Boss {
private:
    int currentPhase_ = 1;
    
public:
    void Update() override {
        UpdatePhase();
        GameObject::Update();
    }
    
private:
    void UpdatePhase() {
        int newPhase = CalculatePhaseFromHP();
        if (newPhase != currentPhase_) {
            currentPhase_ = newPhase;
            SwitchBehaviorTree(newPhase);
        }
    }
    
    void SwitchBehaviorTree(int phase) {
        switch (phase) {
            case 1: SetBehaviorTree(CreatePhase1AI()); break;
            case 2: SetBehaviorTree(CreatePhase2AI()); break;
            case 3: SetBehaviorTree(CreatePhase3AI()); break;
        }
    }
};
```

---

## ?? ベストプラクティス

### 1. ツリーの構築は初期化時に1回

```cpp
// ? 良い例
void GameScene::Initialize() {
    boss_->SetBehaviorTree(CreateBossBehaviorTree());
}

// ? 悪い例
void GameScene::Update() {
    boss_->SetBehaviorTree(CreateBossBehaviorTree());  // 毎フレーム作成しない
}
```

### 2. 明確な命名規則

```cpp
// ツリー名
"Phase1BossAI"
"AggressiveBossAI"
"DefensiveBossAI"

// アクション名
"ChargeToPlayer"
"MeleeAttack"
"RangedAttack"
```

### 3. ツリーの分割と再利用

```cpp
// 攻撃パターンを別関数に
std::unique_ptr<BaseNode> CreateAttackPattern(Boss* boss, Player* player) {
    return BehaviorTreeBuilder()
        .Sequence()
            .Action<Attack1>()
            .Wait(0.5f)
            .Action<Attack2>()
        .End()
        .Build();
}

// メインツリーで使用
auto tree = BehaviorTreeFactory::Create(
    [boss, player](BehaviorTreeBuilder& builder) {
        builder.Selector()
            .WeightedNode(CreateAttackPattern(boss, player), 0.7f)
            .Action<DefenseAction>()
        .End();
    }
);
```

### 4. デバッグ情報の活用

```cpp
// ImGuiでツリー情報を表示
if (behaviorTree_) {
    ImGui::Text("AI: %s", behaviorTree_->GetName().c_str());
    ImGui::Text("Tick: %u", behaviorTree_->GetTickCount());
}
```

---

## ?? 拡張ポイント

### 新しいアクションの追加

```cpp
// 1. アクションクラスを作成
class NewAttackAction : public BossActionNode {
protected:
    void OnEnter() override { /* 準備処理 */ }
    NodeState OnExecute() override { /* 実行処理 */ }
    void OnExit() override { /* 後処理 */ }
};

// 2. ツリーに追加
builder.Action<NewAttackAction>(boss, player, params);
```

### 新しいCompositeNodeの追加

```cpp
// 例: ランダムセレクター
class RandomSelectorNode : public CompositeNode {
    NodeState Tick() override {
        // ランダムに子ノードを選択
    }
};
```

---

## ?? パフォーマンス考慮事項

### メモリ使用量

```
BehaviorTree: ~64 bytes
├── unique_ptr<BaseNode>: 8 bytes
├── uint32_t tickCount: 4 bytes
└── string name: ~32 bytes

Boss: ~200+ bytes
└── unique_ptr<BehaviorTree>: 8 bytes (+ BehaviorTreeのサイズ)
```

### CPU使用量

- **Tick()**: O(n) - nはツリーのノード数
- **推奨**: ノード数は50以下
- **最適化**: 条件の早期リターンを活用

---

## ?? トラブルシューティング

### 問題1: ツリーが実行されない

**チェックリスト**:
- [ ] `SetBehaviorTree()`を呼んだか？
- [ ] `SetPlayer()`を呼んだか？
- [ ] `Boss::Update()`内で`behaviorTree_->Tick()`を呼んでいるか？

### 問題2: アクションが実行されない

**デバッグ方法**:
```cpp
.Condition([boss]() {
    float dist = boss->GetDistanceToPlayer();
    std::cout << "Distance: " << dist << std::endl;  // デバッグ出力
    return dist <= 10.0f;
})
```

### 問題3: 意図しない動作

**確認項目**:
- ツリーの構造（SelectorとSequenceの配置）
- 条件の評価順序
- アクションの`OnExit()`でリセット処理

---

## ?? 関連ドキュメント

- **`Application/TD2_2/AI/README.md`**: Evaluatorとヘルパーメソッド
- **`Application/TD2_2/GameObject/Boss/ActionNode/README.md`**: アクションノード詳細
- **`Application/TD2_2/GameObject/Boss/BehaviorTree/README.md`**: BehaviorTree使用ガイド

---

## ? まとめ

### この設計の利点

| 項目 | 説明 |
|------|------|
| **カプセル化** | BehaviorTreeクラスでツリー管理を一元化 |
| **再利用性** | アクションノードは独立して再利用可能 |
| **拡張性** | 新しいアクションやノードを簡単に追加 |
| **デバッグ性** | ツリー名や実行回数で追跡可能 |
| **可読性** | Builderパターンで直感的な構築 |
| **柔軟性** | 動的なツリー切り替えが容易 |

### 推奨されるワークフロー

```
1. 設計
   ↓
2. アクションノードの実装
   ↓
3. BehaviorTreeの構築
   ↓
4. Boss::SetBehaviorTree()で設定
   ↓
5. デバッグとチューニング
```

---

**このシステムにより、複雑なボスAIを効率的かつ保守性高く実装できます！** ??
