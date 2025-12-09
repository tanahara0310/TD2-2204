# ボスAI システム仕様書（改訂版）

## 概要
このドキュメントでは、ゲームシーンにおけるボスのビヘイビアツリーAIの実装について説明します。
HPに応じて5つのフェーズに分かれ、段階的に難易度が上昇する戦略的なAIシステムです。

**改訂ポイント**:
- HP3では突進とショットのみを使用（段階的な行動公開）
- HP2では突進とスパークのみを使用（段階的な行動公開）
- スパーク後は必ず突進で押し込むコンボ
- 新アクション「ChargeAwayFromPlayerAction」追加（フェイント・逃げ）
- ショットパターンを大幅強化（連射で押し出し）

---

## ゲームの基本仕様
- **ステージ構造**: 中央エリアと外周に配置されたダメージウォール
- **勝利条件**: 相手を突進で吹き飛ばし、ダメージウォールに衝突させてHPを削る
- **戦略的重要性**: 中央を陣取ることで有利なポジションを確保できる
- **スタン機能**: エネルギーを溜めてスタンさせることで隙を作る
- **ショットの特性**: 球で押し出すことができる強力な攻撃

---

## フェーズ別AI戦略

### 【HP=5 フェーズ】: 初期段階
**戦略**: 逃げるのみ（プレイヤーに慣れる時間を与える）

**実装**:
```
Sequence:
  - Condition: HP == 5
  - FleeFromPlayerAction
```

**特徴**:
- プレイヤーから常に距離を取る
- 攻撃を一切行わない
- ゲーム序盤でプレイヤーが操作に慣れる猶予を提供

---

### 【HP=4 フェーズ】: 基本攻撃導入
**戦略**: 逃げる → 突進 → ランダムセレクター（追撃 or 中央移動）

**実装**:
```
Sequence:
  - Condition: HP == 4
  - FleeFromPlayerAction
  - ChargeToPlayerAction
  - WeightedSelector:
      - ChargeToPlayerAction (0.6)
      - MoveToCenterAction (0.4)
```

**特徴**:
- 基本的な攻撃パターンの導入
- 60%の確率で連続突進、40%で中央への移動
- 位置取りの概念を学習させる

---

### 【HP=3 フェーズ】: 突進とショットのみ ??
**戦略**: 突進とショットのみを使用し、段階的に行動を公開

**実装**:
```
Sequence:
  - Condition: HP == 3
  - FleeFromPlayerAction
  - WeightedSelector:
      - [中央近] Shoot (near_center_evaluator)
      - [中央遠] Charge (far_center_evaluator)
      - Shoot×2 連射 (0.3)
```

**使用アクション**:
- ? ChargeToPlayerAction（突進）
- ? ShootEightWayAction（ショット）
- ? SparkNode（まだ使わない）
- ? ChargeAwayFromPlayerAction（まだ使わない）

**戦略的意図**:
- **中央近**: ショット攻撃で有利な位置をキープ
- **中央遠**: 突進でプレイヤーを壁に押し込む
- **ショット連射**: 強力な押し出し攻撃パターン

---

### 【HP=2 フェーズ】: 突進とスパークのみ ??
**戦略**: スパークでスタンを狙い、必ず突進で押し込む

**実装**:
```
Sequence:
  - Condition: HP == 2
  - WeightedSelector:
      - Flee → Spark → Charge (0.6) ★スパーク後押し込み
      - Charge → Charge (stun_bias_evaluator) ★スタン追撃
      - Charge (0.3)
```

**使用アクション**:
- ? ChargeToPlayerAction（突進）
- ? SparkNode（スパーク）
- ? FleeFromPlayerAction（逃げ）
- ? ShootEightWayAction（まだ使わない）
- ? ChargeAwayFromPlayerAction（まだ使わない）

**重要な変更**:
- **スパーク→突進コンボ**: スパークで範囲スタンさせた後、必ず突進で押し込む
- **スタン追撃**: プレイヤーがスタン中は連続突進で追撃

---

### 【HP=1 フェーズ】: 最強AI - 全技解放 ??
**戦略**: 全アクションを駆使し、8つの戦術を動的に選択

#### 戦術1: 中央確保
```
MoveToCenterAction
Evaluator: far_center × 1.5
```
壁から遠い時は中央確保を最優先

#### 戦術2: スパーク→突進コンボ
```
Spark → Charge ★押し込み
Evaluator: far_center × 0.6 + non_stun × 0.8
```
非スタン時かつ遠距離で選ばれやすい、スパーク後は必ず押し込み

#### 戦術3: ショット連射→離脱 ??
```
Shoot → Shoot → Flee
Evaluator: medium_range × 0.8 + near_center × 0.6
```
**強化**: ショット2連射で強力な押し出し攻撃

#### 戦術4: 連続突進
```
Charge → Charge → Charge
Evaluator: close_range × 1.0 + near_center × 0.8
```
近距離かつ中央近くで壁に押し込む

#### 戦術5: スタン追撃コンボ
```
Charge → Charge → Spark → Charge ★押し込み
Evaluator: stun_bias × 2.0
```
プレイヤースタン時に最優先、スパーク後は押し込み

#### 戦術6: フェイント→ショット ??
```
ChargeAway → Shoot
Weight: 0.35
```
**新アクション**: プレイヤーから離れる方向に突進（フェイント）してからショット

#### 戦術7: ショット→突進 ??
```
Shoot → Charge
Weight: 0.3
```
ショットで押し出してから突進で追撃

#### 戦術8: 逃げ→ショット3連射 ??
```
ChargeAway → Shoot × 3
Weight: 0.25
```
**最強パターン**: 距離を取ってからショット3連射で強力な押し出し

---

## 新規追加アクション

### ChargeAwayFromPlayerAction ??

**概要**: プレイヤーから離れる方向に突進するアクション

**用途**:
1. **フェイント**: プレイヤーの予想と逆に動く
2. **逃げ**: ダメージウォールへの自爆を防ぐ
3. **距離確保**: ショット攻撃の準備

**実装ファイル**:
- `ChargeAwayFromPlayerAction.h`
- `ChargeAwayFromPlayerAction.cpp`

**実装詳細**:
```cpp
Vector3 CalculateDirectionAwayFromPlayer() const {
   Vector3 bossPos = boss_->GetWorldPosition();
   Vector3 playerPos = player_->GetWorldPosition();
   // プレイヤーから離れる方向 = ボスから見てプレイヤーと反対方向
   Vector3 direction = {
      bossPos.x - playerPos.x,  // 反転
      bossPos.y - playerPos.y,  // 反転
      0.0f
   };
   return MathCore::Vector::Normalize(direction);
}
```

**ChargeToPlayerActionとの違い**:
| 項目 | ChargeToPlayerAction | ChargeAwayFromPlayerAction |
|------|---------------------|---------------------------|
| 方向 | プレイヤーへ | プレイヤーから離れる |
| 用途 | 攻撃・押し込み | フェイント・逃げ |
| HP使用フェーズ | HP4～HP1 | HP1のみ |

---

## アクション使用制限（段階的公開）

### 各フェーズで使用可能なアクション

| アクション | HP5 | HP4 | HP3 | HP2 | HP1 |
|-----------|-----|-----|-----|-----|-----|
| FleeFromPlayerAction | ? | ? | ? | ? | ? |
| ChargeToPlayerAction | ? | ? | ? | ? | ? |
| MoveToCenterAction | ? | ? | ? | ? | ? |
| ShootEightWayAction | ? | ? | ? | ? | ? |
| SparkNode | ? | ? | ? | ? | ? |
| ChargeAwayFromPlayerAction | ? | ? | ? | ? | ? |

**設計意図**:
- プレイヤーが段階的にボスの行動を学習できる
- HP3でショットの強さを体験
- HP2でスパーク→突進コンボを学ぶ
- HP1で全ての技を組み合わせた戦略と対峙

---

## スパーク後押し込みコンボ ??

### コンボの重要性

スパークは**範囲スタン攻撃**であり、プレイヤーを行動不能にします。
その後すぐに突進することで、**確実にプレイヤーを壁に押し込む**ことができます。

### 実装パターン

#### HP2フェーズ:
```
Flee → Spark → Charge
```
1. 距離を取る
2. スパークでスタン
3. 突進で押し込む

#### HP1フェーズ:
```
Spark → Charge
```
より直接的なコンボ

```
Charge → Charge → Spark → Charge
```
スタン追撃コンボ（スタン中のプレイヤーに対して）

### コンボの効果
- ? スタン中のプレイヤーは回避不可
- ? 確実にダメージウォールに押し込める
- ? プレイヤーに大きなプレッシャー

---

## ショット強化パターン ??

### ショットの特性
- **押し出し効果**: 球がプレイヤーに当たると押し出される
- **連射**: 複数回撃つことで強力な押し出しが可能
- **中央キープ**: 中央から撃つことで有利な位置を維持

### 強化されたショットパターン

#### HP3フェーズ:
```
Shoot × 2 (連射)
```
ショットの強さをプレイヤーに学習させる

#### HP1フェーズ:
```
Shoot → Shoot → Flee (戦術3)
```
連射してから離脱

```
Shoot → Charge (戦術7)
```
ショットで崩してから突進

```
ChargeAway → Shoot × 3 (戦術8)
```
**最強**: 距離を取ってから3連射

### ショットの戦略的価値
1. **安全な攻撃**: 突進と違い壁に自爆しない
2. **強力な押し出し**: 連射で確実にプレイヤーを動かせる
3. **中央キープ**: 有利な位置を維持しながら攻撃

---

## 自爆防止設計 ??

### 問題点
従来の「突進しまくるAI」は自らダメージウォールに突っ込む問題があった

### 解決策

#### 1. ChargeAwayFromPlayerAction導入
```cpp
// プレイヤーから離れる方向に突進
Vector3 direction = bossPos - playerPos;
```
壁から離れる方向に逃げることで自爆を防ぐ

#### 2. 中央確保の優先度向上
```cpp
MoveToCenterAction
Evaluator: far_center × 1.5 // 壁から遠い時に強く選ばれる
```

#### 3. ショット攻撃の増加
- 突進の頻度を下げ、ショット攻撃を増やすことで自爆リスク減少

#### 4. 評価関数による制御
```cpp
// 中央近くにいる時は突進を避ける
create_near_center_evaluator() // ショットを優先
create_far_center_evaluator()  // 突進を優先
```

---

## 評価関数システム

### 基本評価関数

#### 距離ベース
- **near_center**: `1.0 - (distance / max_range)` - 中央に近いほど高評価
- **far_center**: `distance / max_range` - 中央から遠いほど高評価
- **close_range**: 近距離（20.0f以下）で1.0、遠距離（40.0f以上）で0.0
- **medium_range**: 15.0f～25.0fの適度な距離で1.0

#### 状態ベース
- **stun_bias**: プレイヤースタン時に1.0、非スタン時に0.1
- **non_stun_bias**: プレイヤー非スタン時に1.0、スタン時に0.0

### 複合評価関数
`CompositeEvaluator`を使用して複数の評価を組み合わせ:
- **CombineMode::Product**: 評価値を掛け合わせて総合評価
- **Weight**: 各評価の重要度を調整

---

## 技術的実装詳細

### ビヘイビアツリー構造
```
Selector (Root)
├─ Sequence [HP=5] : Flee only
├─ Sequence [HP=4] : Flee + Charge + (Charge/MoveCenter)
├─ Sequence [HP=3] : Flee + (Shoot/Charge/Shoot×2) ??
├─ Sequence [HP=2] : (Spark→Charge / Charge×2 / Charge) ??
├─ Sequence [HP=1] : 8 strategies with all actions ??
└─ Sequence [Fallback]
```

### 新規ファイル ??
- `Application/TD2_2/GameObject/Boss/ActionNode/ChargeAwayFromPlayerAction.h`
- `Application/TD2_2/GameObject/Boss/ActionNode/ChargeAwayFromPlayerAction.cpp`

### アクションノード一覧
- **FleeFromPlayerAction**: プレイヤーから逃げる
- **ChargeToPlayerAction**: プレイヤーに突進
- **ChargeAwayFromPlayerAction**: プレイヤーから離れる方向に突進 ??
- **MoveToCenterAction**: 中央に移動
- **ShootEightWayAction**: 8方向に弾を発射
- **SparkNode**: スパーク攻撃（範囲スタン）

---

## 戦略的バランス設計

### HP=5→4の変化
逃げるだけ → 基本攻撃導入
**学習曲線**: プレイヤーに攻撃パターンを理解させる

### HP=4→3の変化 ??
ランダム選択 → **突進とショットのみ**
**段階公開**: ショットの強さを学ばせる

### HP=3→2の変化 ??
突進＋ショット → **突進とスパークのみ**
**新要素**: スパーク→突進コンボを導入

### HP=2→1の変化 ??
基本戦術 → **全技解放＋8つの戦術**
**最高難易度**: ChargeAway・ショット強化・複雑な組み合わせ

---

## パフォーマンス考慮事項

### 評価関数の最適化
- ラムダ関数によるキャプチャで動的評価
- 必要最小限の計算（距離計算は1回のみ）

### メモリ効率
- `std::unique_ptr`による所有権管理
- 不要なコピーを避ける（move semantics）

### 実行効率
- Conditionによる早期リターン
- HPチェックは最初のConditionで即座に判定

---

## デバッグとチューニング

### 調整可能なパラメータ

#### 距離評価
```cpp
const float max_range = 30.0f; // 中央距離の最大評価範囲
```

#### ウェイト調整（HP3）??
```cpp
.WeightedNode(..., create_near_center_evaluator()) // ショット優先
.WeightedNode(..., create_far_center_evaluator())  // 突進優先
.WeightedNode(..., 0.3f) // ショット連射
```

#### ウェイト調整（HP2）??
```cpp
.WeightedNode(..., 0.6f) // スパーク→突進コンボ
.WeightedNode(..., create_stun_bias_evaluator()) // スタン追撃
.WeightedNode(..., 0.3f) // 突進のみ
```

#### ウェイト調整（HP1）??
```cpp
// フェイント: 0.35
// ショット→突進: 0.3
// 逃げ→ショット3連: 0.25
```

### ビヘイビアツリー名
```cpp
"BossAdvancedAI"
```
デバッグ時にツリーの識別に使用

---

## 今後の拡張可能性

### 追加可能な要素
1. **時間ベース評価**: 戦闘時間に応じた難易度調整
2. **学習型AI**: プレイヤーの行動パターンを学習
3. **ランダム性**: 予測不可能性の追加
4. **フェーズ遷移エフェクト**: HPが減った時の演出

### 調整推奨項目
- 各フェーズの攻撃頻度
- スパーク攻撃のクールダウン
- ショット連射の回数（2連 or 3連）
- ChargeAwayの使用頻度
- 評価関数の閾値
- 複合評価のウェイトバランス

---

## まとめ

このAIシステムは、以下の特徴を持ちます:

? **段階的難易度**: HP減少に応じて難易度が上昇  
? **段階的行動公開**: HP3で突進＋ショット、HP2で突進＋スパーク ??  
? **スパーク→突進コンボ**: 確実に押し込む強力なコンボ ??  
? **ショット強化**: 連射パターンで押し出し攻撃 ??  
? **自爆防止**: ChargeAwayで壁への自爆を回避 ??  
? **戦略的多様性**: 状況に応じた行動選択（HP1で8戦術）??  
? **位置取り重視**: ゲームの核心（中央確保）を理解  
? **拡張性**: 新しい行動や評価関数を簡単に追加可能  

プレイヤーは、このAIと戦うことで自然にゲームの深い戦略を学ぶことができます。

## 改訂履歴

**v2.0** (最新):
- ChargeAwayFromPlayerAction追加
- HP3/HP2のアクション制限実装
- スパーク→突進コンボ必須化
- ショット強化パターン追加（連射×3）
- 自爆防止設計の強化

**v1.0**:
- 初版リリース
