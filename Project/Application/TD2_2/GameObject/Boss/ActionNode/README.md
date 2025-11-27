# ボスアクションノード設計ガイド

## 概要

このシステムは**ビヘイビアツリー**と**ステートマシン**を組み合わせた設計を採用しています。

### 設計思想

```
ビヘイビアツリー (高レベル意思決定)
    ↓
アクションノード (個別の行動)
    ↓
ステートマシン (行動内部の状態管理)
```

- **ビヘイビアツリー**: どの行動を選択するか（戦略的判断）
- **アクションノード**: 具体的な行動の実装（突進、攻撃、防御など）
- **ステートマシン**: 行動内部の遷移（準備→実行→終了）

---

## アーキテクチャ

### BossActionNode (基底クラス)

```cpp
class BossActionNode : public ActionNode {
public:
    enum class ActionState {
        Idle,       // 待機（未実行）
        Enter,      // 開始処理
        Execute,    // 実行中
        Exit,       // 終了処理
        Completed   // 完了
    };

protected:
    Boss* boss_;                              // ボスへの参照
    ActionState currentState_;                // 現在の状態
    std::unique_ptr<StateMachine> stateMachine_;  // 内部ステートマシン

    // 派生クラスで実装する仮想関数
    virtual void OnEnter() {}                 // 開始時
    virtual NodeState OnExecute() = 0;        // 実行中（必須実装）
    virtual void OnExit() {}                  // 終了時
};
```

### 状態遷移フロー

```
Idle → Enter → Execute → Exit → Completed
         ↓       ↓         ↓
      OnEnter  OnExecute  OnExit
```

---

## ChargeToPlayerAction の実装例

### 特徴

1. **準備フェーズ**: プレイヤーの方向を計算し、溜めモーション
2. **実行フェーズ**: 高速で突進
3. **完了フェーズ**: パラメータをリセット

### コード構造

```cpp
class ChargeToPlayerAction : public BossActionNode {
private:
    Player* player_;
    float chargeSpeed_;
    float chargeDuration_;
    GameTimer chargeTimer_;
    GameTimer preparationTimer_;
    Vector3 chargeDirection_;
    bool isPreparationComplete_;

protected:
    void OnEnter() override {
        // 準備開始
        preparationTimer_.Start(preparationTime_);
        chargeDirection_ = boss_->GetDirectionToPlayer();
    }

    NodeState OnExecute() override {
        // 準備フェーズ
        if (!isPreparationComplete_) {
            PrepareCharge();
            if (preparationTimer_.IsFinished()) {
                isPreparationComplete_ = true;
                chargeTimer_.Start(chargeDuration_);
                boss_->SetMaxSpeed(chargeMaxSpeed_);
                boss_->SetDamping(chargeDamping_);
            }
            return BossActionHelper::Running();
        }
        
        // 突進フェーズ
        if (!chargeTimer_.IsFinished()) {
            ExecuteCharge();
            return BossActionHelper::Running();
        }
        
        return BossActionHelper::Success();
    }

    void OnExit() override {
        boss_->ResetMovementParameters();
    }
};
```

---

## 使用方法

### 1. Bossの初期化

```cpp
// GameSceneなどで
boss_ = std::make_unique<Boss>();
boss_->Initialize(model, texture);
boss_->SetPlayer(player_.get());

// ビヘイビアツリーの構築
auto bossTree = BehaviorTreeBuilder()
    .Selector()
        // 近距離: 突進攻撃
        .DistanceBasedAction<ChargeToPlayerAction>(
            &boss_->GetWorldPosition(),
            &player_->GetWorldPosition(),
            0.0f, 10.0f,  // 10m以内で実行
            boss_.get(), player_.get()
        )
        
        // 遠距離: 追跡
        // .Action<ChaseAction>(boss_.get(), player_.get())
    .End()
    .Build();

boss_->SetBehaviorTree(std::move(bossTree));
```

### 2. 更新処理

```cpp
void Boss::Update() {
    // ビヘイビアツリーが自動的に適切なアクションを選択・実行
    if (behaviorTree_) {
        behaviorTree_->Tick();
    }
    
    // 物理演算などの基本処理
    UpdateMovement();
}
```

---

## 新しいアクションの追加方法

### ステップ1: ヘッダーファイル作成

```cpp
// MeleeAttackAction.h
#pragma once
#include "ActionNode.h"

class MeleeAttackAction : public BossActionNode {
public:
    MeleeAttackAction(Boss* boss, Player* player, float damage);
    
    void Reset() override;

protected:
    void OnEnter() override;
    NodeState OnExecute() override;
    void OnExit() override;

private:
    Player* player_;
    float damage_;
    GameTimer attackTimer_;
    bool hasHit_;
};
```

### ステップ2: 実装ファイル作成

```cpp
// MeleeAttackAction.cpp
#include "MeleeAttackAction.h"

MeleeAttackAction::MeleeAttackAction(Boss* boss, Player* player, float damage)
    : BossActionNode(boss, "MeleeAttack"),
      player_(player),
      damage_(damage),
      hasHit_(false) {}

void MeleeAttackAction::OnEnter() {
    attackTimer_.Start(0.5f);  // 攻撃モーション時間
    hasHit_ = false;
}

NodeState MeleeAttackAction::OnExecute() {
    // 攻撃判定
    if (!hasHit_ && boss_->GetDistanceToPlayer() < 2.0f) {
        // プレイヤーにダメージ
        // player_->TakeDamage(damage_);
        hasHit_ = true;
    }
    
    if (attackTimer_.IsFinished()) {
        return BossActionHelper::Success();
    }
    
    return BossActionHelper::Running();
}

void MeleeAttackAction::OnExit() {
    // クリーンアップ処理
}
```

### ステップ3: ビヘイビアツリーに追加

```cpp
auto bossTree = BehaviorTreeBuilder()
    .Selector()
        // 超近距離: 近接攻撃
        .DistanceBasedAction<MeleeAttackAction>(
            &boss_->pos, &player_->pos,
            0.0f, 3.0f,
            boss_.get(), player_.get(), 10.0f  // damage
        )
        
        // 近距離: 突進
        .DistanceBasedAction<ChargeToPlayerAction>(
            &boss_->pos, &player_->pos,
            3.0f, 10.0f,
            boss_.get(), player_.get()
        )
        
        // 遠距離: 追跡
        // .Action<ChaseAction>()
    .End()
    .Build();
```

---

## 複雑なアクション例

### コンボ攻撃アクション

```cpp
class ComboAttackAction : public BossActionNode {
protected:
    void SetupStateMachine() override {
        // 基底クラスのセットアップを呼び出し
        BossActionNode::SetupStateMachine();
        
        // 追加の状態を定義
        stateMachine_->AddState("Attack1",
            [this]() { StartAttack1(); },
            [this]() { 
                if (attack1Timer_.IsFinished()) {
                    stateMachine_->RequestState("Attack2", 1);
                }
            }
        );
        
        stateMachine_->AddState("Attack2",
            [this]() { StartAttack2(); },
            [this]() { 
                if (attack2Timer_.IsFinished()) {
                    stateMachine_->RequestState("Attack3", 1);
                }
            }
        );
        
        stateMachine_->AddState("Attack3",
            [this]() { StartAttack3(); },
            [this]() { 
                if (attack3Timer_.IsFinished()) {
                    stateMachine_->RequestState("Exit", 1);
                }
            }
        );
        
        // Executeから最初の攻撃へ遷移
        stateMachine_->AddTransitionRule("Execute", {"Attack1"});
    }
    
    NodeState OnExecute() override {
        // ステートマシンが状態を管理
        const std::string& state = stateMachine_->GetCurrentState();
        
        if (state == "Attack3" && attack3Timer_.IsFinished()) {
            return BossActionHelper::Success();
        }
        
        return BossActionHelper::Running();
    }
};
```

---

## ベストプラクティス

### 1. アクションの粒度

? **良い例**: 単一責任の原則
```cpp
ChargeToPlayerAction    // 突進のみ
MeleeAttackAction       // 近接攻撃のみ
RangedAttackAction      // 遠距離攻撃のみ
```

? **悪い例**: 複数の責任
```cpp
AttackPlayerAction      // 突進も近接も遠距離も全部
```

### 2. パラメータの外部化

```cpp
// コンストラクタでパラメータを受け取る
ChargeToPlayerAction(Boss* boss, Player* player,
                     float chargeSpeed,      // 調整可能
                     float chargeDuration);  // 調整可能
```

### 3. エラーハンドリング

```cpp
NodeState OnExecute() override {
    if (!boss_ || !player_) {
        return BossActionHelper::Failure();  // nullチェック
    }
    
    // 通常処理
}
```

### 4. デバッグ情報

```cpp
void OnEnter() override {
#ifdef _DEBUG
    std::cout << "[" << actionName_ << "] Started" << std::endl;
#endif
}
```

---

## パフォーマンス最適化

### 1. 不要な計算の削減

```cpp
// 悪い例: 毎フレーム計算
NodeState OnExecute() override {
    Vector3 direction = boss_->GetDirectionToPlayer();  // 重い計算
    // ...
}

// 良い例: OnEnterで1回だけ計算
void OnEnter() override {
    chargeDirection_ = boss_->GetDirectionToPlayer();  // 1回だけ
}

NodeState OnExecute() override {
    // 保存した値を使用
    boss_->AddAcceleration(chargeDirection_ * chargeSpeed_);
}
```

### 2. タイマーの再利用

```cpp
void Reset() override {
    BossActionNode::Reset();
    chargeTimer_.Reset();      // タイマーをリセット
    preparationTimer_.Reset();
}
```

---

## トラブルシューティング

### アクションが実行されない

1. ビヘイビアツリーが設定されているか確認
2. Playerへの参照が設定されているか確認
3. 条件（距離など）が満たされているか確認

### アクションが終了しない

1. `OnExecute()`が`Success`または`Failure`を返しているか確認
2. タイマーが正しく設定・更新されているか確認
3. 無限ループになっていないか確認

### 意図しない動作

1. `Reset()`メソッドで状態が正しくリセットされているか確認
2. ステートマシンの遷移ルールを確認
3. デバッグ出力を追加して状態遷移を追跡

---

## まとめ

### この設計の利点

? **関心の分離**: ビヘイビアツリー（戦略）とアクションノード（実装）の分離  
? **再利用性**: アクションノードは独立しており、異なるツリーで再利用可能  
? **拡張性**: 新しいアクションを簡単に追加可能  
? **デバッグ性**: 各アクションが独立しているため、問題の特定が容易  
? **状態管理**: ステートマシンによる明確な状態遷移  

### 推奨される使用パターン

```
複雑なボスAI
├── ビヘイビアツリー
│   ├── フェーズ1 (HP 100-66%)
│   │   ├── 突進攻撃
│   │   ├── 近接攻撃
│   │   └── 追跡
│   ├── フェーズ2 (HP 66-33%)
│   │   ├── コンボ攻撃
│   │   ├── 範囲攻撃
│   │   └── 回避
│   └── フェーズ3 (HP 33-0%)
│       ├── 必殺技
│       ├── 連続突進
│       └── 狂暴化
└── 各アクションは内部でステートマシンを使用
```
