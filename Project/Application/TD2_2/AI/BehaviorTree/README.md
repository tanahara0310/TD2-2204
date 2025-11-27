# BehaviorTree クラス使用ガイド

## 概要

`BehaviorTree`クラスは、ボスやエネミーのAIを管理するためのクラスです。`BehaviorTreeBuilder`を使用してツリーを構築し、ボスクラスに設定することで、複雑な行動パターンを簡単に実装できます。

---

## 基本的な使い方

### 1. GameSceneでの初期化

```cpp
// GameScene.h
class GameScene : public BaseScene {
private:
    std::unique_ptr<Boss> boss_;
    std::unique_ptr<Player> player_;
};

// GameScene.cpp
#include "Application/TD2_2/GameObject/Boss/BehaviorTree/BehaviorTree.h"
#include "Application/TD2_2/GameObject/Boss/ActionNode/ChargeToPlayerAction.h"

void GameScene::Initialize() {
    // 1. ボスとプレイヤーの初期化
    boss_ = std::make_unique<Boss>();
    boss_->Initialize(bossModel, bossTexture);
    
    player_ = std::make_unique<Player>();
    player_->Initialize(playerModel, playerTexture);
    
    // 2. プレイヤーへの参照を設定
    boss_->SetPlayer(player_.get());
    
    // 3. ビヘイビアツリーの構築
    auto bossTree = CreateBossBehaviorTree();
    
    // 4. ビヘイビアツリーを設定
    boss_->SetBehaviorTree(std::move(bossTree));
}

void GameScene::Update() {
    player_->Update();
    boss_->Update();  // 内部でbehaviorTree_->Tick()が実行される
}
```

---

## BehaviorTreeの構築方法

### 方法1: BehaviorTreeBuilderを直接使用

```cpp
std::unique_ptr<BehaviorTree> CreateBossBehaviorTree() {
    // 1. ビルダーを作成
    BehaviorTreeBuilder builder;
    
    // 2. ツリーを構築
    builder.Selector()
        // 近距離: 突進攻撃
        .Sequence()
            .Condition([this]() {
                return boss_->GetDistanceToPlayer() <= 10.0f;
            })
            // ChargeToPlayerActionを追加する場合は
            // アクションノードを継承した実装が必要
        .End()
        
        // 遠距離: 待機
        .Action<IdleAction>()
    .End();
    
    // 3. BehaviorTreeオブジェクトを作成
    auto tree = std::make_unique<BehaviorTree>();
    tree->SetRoot(builder.Build());
    tree->SetName("BossAI");
    
    return tree;
}
```

### 方法2: BehaviorTreeFactoryを使用（推奨）

```cpp
std::unique_ptr<BehaviorTree> CreateBossBehaviorTree() {
    return BehaviorTreeFactory::Create(
        [this](BehaviorTreeBuilder& builder) {
            builder.Selector()
                // 近距離: 突進攻撃
                .DistanceBasedAction<ChargeToPlayerAction>(
                    &boss_->GetWorldPosition(),
                    &player_->GetWorldPosition(),
                    0.0f, 10.0f,
                    boss_.get(), player_.get()
                )
                
                // 遠距離: 追跡
                .Action<ChaseAction>(boss_.get(), player_.get())
            .End();
        },
        "BossAI"  // ツリーの名前
    );
}
```

---

## 実践例

### 例1: シンプルな距離ベースAI

```cpp
std::unique_ptr<BehaviorTree> CreateSimpleBossAI(Boss* boss, Player* player) {
    auto builder = BehaviorTreeFactory::CreateBuilder();
    
    builder.Selector()
        // 超近距離（0-3m）: 近接攻撃
        .Sequence()
            .Condition([boss]() {
                return boss->GetDistanceToPlayer() <= 3.0f;
            })
            // .Action<MeleeAttackAction>(boss, player)
        .End()
        
        // 近距離（3-10m）: 突進攻撃
        .Sequence()
            .Condition([boss]() {
                float dist = boss->GetDistanceToPlayer();
                return dist > 3.0f && dist <= 10.0f;
            })
            // .Action<ChargeToPlayerAction>(boss, player)
        .End()
        
        // 遠距離（10m以上）: 追跡
        // .Action<ChaseAction>(boss, player)
    .End();
    
    return BehaviorTreeFactory::Build(builder, "SimpleBossAI");
}
```

### 例2: 3段階フェーズシステム

```cpp
std::unique_ptr<BehaviorTree> CreatePhasedBossAI(
    Boss* boss, 
    Player* player,
    std::function<float()> getBossHp) {
    
    return BehaviorTreeFactory::Create(
        [boss, player, getBossHp](BehaviorTreeBuilder& builder) {
            builder.Selector()
                // === フェーズ3: HP 0-33% - 狂暴化 ===
                .ConditionalSequence([getBossHp]() { 
                    return getBossHp() <= 0.33f; 
                })
                    .WeightedSelector()
                        // 連続突進
                        .Repeater(3)
                            // .Action<ChargeToPlayerAction>(boss, player)
                        .End()
                        // 範囲攻撃
                        // .Action<AOEAttackAction>(boss, player)
                    .End()
                .End()
                
                // === フェーズ2: HP 34-66% - 戦術的 ===
                .ConditionalSequence([getBossHp]() { 
                    float hp = getBossHp();
                    return hp > 0.33f && hp <= 0.66f; 
                })
                    .WeightedSelector()
                        // .Action<TacticalAttackAction>(boss, player)
                        // .Action<DefenseAction>(boss)
                    .End()
                .End()
                
                // === フェーズ1: HP 67-100% - 通常 ===
                .WeightedSelector()
                    // .Action<NormalAttackAction>(boss, player)
                    // .Action<PatrolAction>(boss)
                .End()
            .End();
        },
        "PhasedBossAI"
    );
}
```

### 例3: 複合Evaluatorを使用した動的AI

```cpp
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
```

---

## GameSceneでの完全な実装例

```cpp
// GameScene.h
#pragma once
#include "Engine/Scene/BaseScene.h"
#include "Application/TD2_2/GameObject/Boss/Boss.h"
#include "Application/TD2_2/GameObject/Player/Player.h"

class GameScene : public BaseScene {
public:
    void Initialize() override;
    void Update() override;
    void Draw() override;

private:
    std::unique_ptr<Boss> boss_;
    std::unique_ptr<Player> player_;
    
    /// @brief ボスのビヘイビアツリーを作成
    std::unique_ptr<BehaviorTree> CreateBossBehaviorTree();
};

// GameScene.cpp
#include "GameScene.h"
#include "Application/TD2_2/GameObject/Boss/BehaviorTree/BehaviorTree.h"
#include "Application/TD2_2/GameObject/Boss/ActionNode/ChargeToPlayerAction.h"

void GameScene::Initialize() {
    // ボスの初期化
    boss_ = std::make_unique<Boss>();
    boss_->Initialize(
        ModelManager::GetInstance().Load("boss.obj"),
        TextureManager::GetInstance().Load("boss.png")
    );
    boss_->SetWorldPosition({0.0f, 0.0f, 0.0f});
    
    // プレイヤーの初期化
    player_ = std::make_unique<Player>();
    player_->Initialize(
        ModelManager::GetInstance().Load("player.obj"),
        TextureManager::GetInstance().Load("player.png")
    );
    player_->SetWorldPosition({10.0f, 0.0f, 0.0f});
    
    // プレイヤーへの参照を設定
    boss_->SetPlayer(player_.get());
    
    // ビヘイビアツリーの構築と設定
    boss_->SetBehaviorTree(CreateBossBehaviorTree());
}

void GameScene::Update() {
    if (player_) {
        player_->Update();
    }
    
    if (boss_) {
        boss_->Update();  // ビヘイビアツリーが自動実行される
    }
}

void GameScene::Draw() {
    // カメラの取得
    auto camera = CameraManager::GetInstance().GetActiveCamera();
    
    // 描画
    if (player_) {
        player_->Draw(camera);
    }
    
    if (boss_) {
        boss_->Draw(camera);
    }
}

std::unique_ptr<BehaviorTree> GameScene::CreateBossBehaviorTree() {
    // BehaviorTreeFactoryを使用して構築
    return BehaviorTreeFactory::Create(
        [this](BehaviorTreeBuilder& builder) {
            builder.Selector()
                // 距離が10m以内なら突進
                .Sequence()
                    .Condition([this]() {
                        return boss_->GetDistanceToPlayer() <= 10.0f;
                    })
                    // 実際のアクションを追加
                    // .Action<ChargeToPlayerAction>(boss_.get(), player_.get())
                .End()
                
                // それ以外は待機
                // .Action<IdleAction>(boss_.get())
            .End();
        },
        "BossMainAI"
    );
}
```

---

## BehaviorTreeクラスのAPI

### 主要なメソッド

| メソッド | 説明 |
|---------|------|
| `SetRoot(std::unique_ptr<BaseNode>)` | ルートノードを設定 |
| `Tick()` | ツリーを1回実行（毎フレーム呼ぶ） |
| `Reset()` | ツリーをリセット |
| `HasRoot()` | ルートノードが設定されているか確認 |
| `GetTickCount()` | 実行回数を取得（デバッグ用） |
| `SetName(const std::string&)` | ツリー名を設定 |
| `GetName()` | ツリー名を取得 |

### BehaviorTreeFactoryのヘルパー

| メソッド | 説明 |
|---------|------|
| `CreateBuilder()` | 新しいビルダーを作成 |
| `Build(builder, name)` | ビルダーからBehaviorTreeを構築 |
| `Create(buildFunc, name)` | ラムダ式で直接構築（推奨） |

---

## デバッグとトラブルシューティング

### ImGuiでのデバッグ情報

BossクラスのImGuiに以下の情報が表示されます：

```cpp
// Boss.cpp DrawImGui()内
if (behaviorTree_) {
    ImGui::Text("ビヘイビアツリー: %s", behaviorTree_->GetName().c_str());
    ImGui::Text("実行回数: %u", behaviorTree_->GetTickCount());
}
```

### よくある問題と解決方法

#### 1. ビヘイビアツリーが実行されない

```cpp
// 原因: ツリーが設定されていない
// 解決: SetBehaviorTree()を呼び出す
boss_->SetBehaviorTree(CreateBossBehaviorTree());
```

#### 2. プレイヤーへの参照がnullptr

```cpp
// 原因: SetPlayer()を呼び忘れ
// 解決: 初期化時にSetPlayer()を呼ぶ
boss_->SetPlayer(player_.get());
```

#### 3. アクションが実行されない

```cpp
// 原因: 条件が満たされていない
// 解決: 条件をデバッグ出力で確認
.Condition([boss]() {
    float dist = boss->GetDistanceToPlayer();
    std::cout << "Distance: " << dist << std::endl;
    return dist <= 10.0f;
})
```

---

## ベストプラクティス

### 1. ツリーの構築は初期化時に1回だけ

```cpp
// 良い例
void GameScene::Initialize() {
    boss_->SetBehaviorTree(CreateBossBehaviorTree());
}

// 悪い例
void GameScene::Update() {
    boss_->SetBehaviorTree(CreateBossBehaviorTree());  // 毎フレーム作成しない！
}
```

### 2. ツリー名を設定してデバッグを容易に

```cpp
BehaviorTreeFactory::Create(
    [](BehaviorTreeBuilder& builder) { /* ... */ },
    "Phase1BossAI"  // 分かりやすい名前を付ける
);
```

### 3. 複雑なツリーは関数に分割

```cpp
std::unique_ptr<BehaviorTree> CreatePhase1AI(Boss* boss, Player* player);
std::unique_ptr<BehaviorTree> CreatePhase2AI(Boss* boss, Player* player);
std::unique_ptr<BehaviorTree> CreatePhase3AI(Boss* boss, Player* player);

// フェーズに応じて切り替え
void Boss::ChangePhase(int phase) {
    switch (phase) {
        case 1: SetBehaviorTree(CreatePhase1AI(this, player_)); break;
        case 2: SetBehaviorTree(CreatePhase2AI(this, player_)); break;
        case 3: SetBehaviorTree(CreatePhase3AI(this, player_)); break;
    }
}
```

---

## まとめ

### この設計の利点

? **カプセル化**: BehaviorTreeクラスでツリーの管理を一元化  
? **再利用性**: BehaviorTreeFactoryで簡単に構築可能  
? **デバッグ性**: ツリー名や実行回数を追跡可能  
? **柔軟性**: ツリーの動的な切り替えが容易  
? **可読性**: GameSceneでのコードがシンプルに  

### 推奨されるワークフロー

1. **設計**: ボスの行動パターンを紙に書く
2. **構築**: BehaviorTreeFactoryでツリーを構築
3. **設定**: Boss::SetBehaviorTree()で設定
4. **テスト**: ImGuiでデバッグ情報を確認
5. **調整**: Evaluatorやパラメータを調整
