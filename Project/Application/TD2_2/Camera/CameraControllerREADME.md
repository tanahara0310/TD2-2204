# カメラ演出システム - 使用方法

## 概要
このカメラ演出システムは、ゲームシーンの開始時やボスが死んだ時などに、映画のようなカメラワークを実現するためのシステムです。
JSONファイルまたはコードで演出を設定でき、ImGuiを使ってリアルタイムに調整することができます。
**新機能：演出プリセット管理とカット割りシーケンス機能**により、複数の画角を組み合わせた映画的な演出が可能です。

## カメラ演出の種類

### 1. FixedPosition（固定位置）
カメラが特定の位置に固定されます。
- **用途**: 静的なシーンの撮影、タイトル画面など
- **設定項目**: startPosition, startRotation

### 2. LookAt（注視）
カメラが特定の位置を注視し続けます。
- **用途**: キャラクターや重要オブジェクトを映す
- **設定項目**: startPosition, targetPosition

### 3. Dolly（移動演出）
カメラが開始位置から終了位置へ滑らかに移動します。
- **用途**: ゲーム開始時の演出、シーン遷移
- **設定項目**: startPosition, endPosition, startRotation, endRotation

### 4. Arc（円弧移動）
カメラが円弧を描いて移動します。
- **用途**: ダイナミックな演出、ステージ全体を見せる
- **設定項目**: startPosition, endPosition, orbitRadius, startRotation, endRotation

### 5. Orbit（回転）
カメラがターゲットの周りを回転します。
- **用途**: ボス登場演出、勝利演出、キャラクター紹介
- **設定項目**: targetPosition, orbitRadius, orbitSpeed, startPosition（高さオフセット）

## 使用方法

### 方法1: プリセット名で演出を開始（最も簡単・推奨）

デフォルトで以下のプリセットが登録されています：

```cpp
// 初期化時にデフォルトプリセットを登録
void GameScene::Initialize(EngineSystem* engine) {
    // ... 既存の初期化コード ...
    
    auto& presetManager = CinematicPresetManager::GetInstance();
    presetManager.RegisterDefaultPresets();
    
    // プリセット名で演出を開始
    if (cameraController_) {
        cameraController_->StartCinematicByName("Opening");
    }
}

// ボス死亡時
void GameScene::Update() {
    if (boss_->GetHP() <= 0 && !cameraController_->IsCinematicActive()) {
        cameraController_->StartCinematicByName("BossAppear");
    }
}
```

**利用可能なデフォルトプリセット：**
- `"Opening"` - オープニング演出：遠くから近づく
- `"BossAppear"` - ボス登場演出：ボスを中心に回転
- `"Victory"` - 勝利演出：プレイヤーを注視
- `"Defeat"` - 敗北演出：プレイヤーの上から見下ろす
- `"CloseUp"` - クローズアップ：キャラクターに近づく
- `"BirdsEye"` - 俯瞰：ステージ全体を見下ろす

### 方法2: カット割りシーケンスを使用（映画的演出）

複数のカットを組み合わせて、映画のようなカメラワークを実現：

```cpp
// シーケンスを読み込んで登録
void GameScene::Initialize(EngineSystem* engine) {
    // ... 既存の初期化コード ...
    
    auto& presetManager = CinematicPresetManager::GetInstance();
    
    // シーケンスをJSONから読み込んで登録
    presetManager.LoadSequenceFromJson("OpeningSequence", 
        "Resources/Data/CameraSequence_Opening.json");
    presetManager.LoadSequenceFromJson("BossDeathSequence", 
        "Resources/Data/CameraSequence_BossDeath.json");
    
    // シーケンスを開始
    if (cameraController_) {
        cameraController_->StartSequenceByName("OpeningSequence");
    }
}

// ボス死亡時のカット割り演出
void GameScene::Update() {
    if (boss_->GetHP() <= 0 && !cameraController_->IsSequenceActive()) {
        cameraController_->StartSequenceByName("BossDeathSequence");
    }
}
```

### 方法3: JSONファイルから読み込む

#### 1. JSONファイルの作成
`Resources/Data/CameraCinematic_Opening.json` を作成：
```json
{
  "type": "Dolly",
  "duration": 5.0,
  "startPosition": [0.0, 15.0, -70.0],
  "endPosition": [0.0, 5.0, -50.0],
  "targetPosition": [0.0, 0.0, 0.0],
  "startRotation": [0.15, 0.0, 0.0],
  "endRotation": [0.0, 0.0, 0.0],
  "orbitRadius": 10.0,
  "orbitSpeed": 1.0,
  "useEasing": true,
  "easingType": "EaseInOutQuad"
}
```

#### 2. GameScene.cppで演出を開始
```cpp
void GameScene::Initialize(EngineSystem* engine) {
    // ... 既存の初期化コード ...
    
    // カメラコントローラーの初期化後に演出を開始
    if (cameraController_) {
        cameraController_->StartCinematicFromJson("Resources/Data/CameraCinematic_Opening.json");
    }
}
```

### 方法4: コードで直接設定する

```cpp
void GameScene::Initialize(EngineSystem* engine) {
    // ... 既存の初期化コード ...
    
    if (cameraController_) {
        // 演出設定を作成
        CameraController::CinematicConfig config;
        config.type = CameraController::CinematicType::Dolly;
        config.duration = 5.0f;
        config.startPosition = {0.0f, 15.0f, -70.0f};
        config.endPosition = {0.0f, 5.0f, -50.0f};
        config.startRotation = {0.15f, 0.0f, 0.0f};
        config.endRotation = {0.0f, 0.0f, 0.0f};
        config.useEasing = true;
        config.easingType = "EaseInOutQuad";
        
        // 演出を開始
        cameraController_->StartCinematic(config);
    }
}
```

## カット割りシーケンスのJSONフォーマット

カット割り演出のJSONファイルの例：

```json
{
  "cuts": [
    {
      "name": "遠景ショット",
      "duration": 2.0,
      "type": "FixedPosition",
      "startPosition": [0.0, 20.0, -80.0],
      "endPosition": [0.0, 20.0, -80.0],
      "targetPosition": [0.0, 0.0, 0.0],
      "startRotation": [0.2, 0.0, 0.0],
      "endRotation": [0.2, 0.0, 0.0],
      "orbitRadius": 10.0,
      "orbitSpeed": 1.0,
      "useEasing": true,
      "easingType": "EaseInOutQuad"
    },
    {
      "name": "接近ショット",
      "duration": 2.5,
      "type": "Dolly",
      "startPosition": [0.0, 15.0, -70.0],
      "endPosition": [0.0, 5.0, -40.0],
      "targetPosition": [0.0, 0.0, 0.0],
      "startRotation": [0.15, 0.0, 0.0],
      "endRotation": [0.05, 0.0, 0.0],
      "orbitRadius": 10.0,
      "orbitSpeed": 1.0,
      "useEasing": true,
      "easingType": "EaseInOutQuad"
    }
  ]
}
```

## 実装例

### カスタムプリセットの登録

```cpp
void GameScene::Initialize(EngineSystem* engine) {
    // ... 既存の初期化コード ...
    
    auto& presetManager = CinematicPresetManager::GetInstance();
    
    // デフォルトプリセットを登録
    presetManager.RegisterDefaultPresets();
    
    // カスタムプリセットを追加
    CameraController::CinematicConfig customConfig;
    customConfig.type = CameraController::CinematicType::Orbit;
    customConfig.duration = 5.0f;
    customConfig.targetPosition = boss_->GetWorldPosition();
    customConfig.startPosition = {0.0f, 8.0f, 0.0f};
    customConfig.orbitRadius = 30.0f;
    customConfig.orbitSpeed = 0.8f;
    customConfig.useEasing = true;
    customConfig.easingType = "EaseInOutCubic";
    
    presetManager.RegisterPreset("CustomBossIntro", customConfig);
    
    // 登録したプリセットを使用
    cameraController_->StartCinematicByName("CustomBossIntro");
}
```

### シーケンスの動的生成

```cpp
void GameScene::CreateCustomSequence() {
    auto sequence = std::make_shared<CinematicSequence>();
    
    // カット1: 遠景
    CinematicCut cut1;
    cut1.name = "遠景";
    cut1.duration = 2.0f;
    cut1.config.type = CameraController::CinematicType::FixedPosition;
    cut1.config.startPosition = {0.0f, 20.0f, -80.0f};
    cut1.config.startRotation = {0.2f, 0.0f, 0.0f};
    sequence->AddCut(cut1);
    
    // カット2: 接近
    CinematicCut cut2;
    cut2.name = "接近";
    cut2.duration = 2.5f;
    cut2.config.type = CameraController::CinematicType::Dolly;
    cut2.config.startPosition = {0.0f, 15.0f, -70.0f};
    cut2.config.endPosition = {0.0f, 5.0f, -40.0f};
    cut2.config.useEasing = true;
    cut2.config.easingType = "EaseInOutQuad";
    sequence->AddCut(cut2);
    
    // シーケンスを登録
    auto& presetManager = CinematicPresetManager::GetInstance();
    presetManager.RegisterSequence("MyCustomSequence", sequence);
    
    // シーケンスを開始
    cameraController_->StartSequenceByName("MyCustomSequence");
}
```

### ボス死亡時のカット割り演出（完全版）

```cpp
void GameScene::Update() {
    // ... 既存の更新コード ...
    
    // ボスが死んだ時
    if (boss_->GetHP() <= 0) {
        // シーケンスが未実行の場合のみ開始
        static bool deathSequenceStarted = false;
        if (!deathSequenceStarted && !cameraController_->IsSequenceActive()) {
            cameraController_->StartSequenceByName("BossDeathSequence");
            deathSequenceStarted = true;
        }
        
        // シーケンスが終了したらシーン切り替え
        if (deathSequenceStarted && !cameraController_->IsSequenceActive()) {
            sceneManager_->ChangeScene("ResultScene");
        }
    }
}
```

### 演出の制御
```cpp
// 演出が実行中かチェック
if (cameraController_->IsCinematicActive()) {
    // 演出の進行度を取得（0.0～1.0）
    float progress = cameraController_->GetCinematicProgress();
    
    // 演出を強制停止（通常追従モードに戻る）
    if (input->IsKeyTriggered(DIK_ESCAPE)) {
        cameraController_->StopCinematic();
    }
}

// シーケンスが実行中かチェック
if (cameraController_->IsSequenceActive()) {
    // シーケンスを停止
    if (input->IsKeyTriggered(DIK_ESCAPE)) {
        cameraController_->StopSequence();
    }
}
```

## JSONファイルの設定項目

| 項目 | 型 | 説明 | 例 |
|------|-----|------|-----|
| type | string | 演出タイプ（"FixedPosition", "LookAt", "Dolly", "Arc", "Orbit"） | "Dolly" |
| duration | float | 継続時間（秒） | 5.0 |
| startPosition | array[3] | 開始位置 [x, y, z] | [0.0, 15.0, -70.0] |
| endPosition | array[3] | 終了位置 [x, y, z] | [0.0, 5.0, -50.0] |
| targetPosition | array[3] | 注視点 [x, y, z] | [0.0, 0.0, 0.0] |
| startRotation | array[3] | 開始回転 [pitch, yaw, roll] | [0.15, 0.0, 0.0] |
| endRotation | array[3] | 終了回転 [pitch, yaw, roll] | [0.0, 0.0, 0.0] |
| orbitRadius | float | 回転半径（Orbitモード時） | 30.0 |
| orbitSpeed | float | 回転速度（Orbitモード時） | 0.5 |
| useEasing | bool | イージングを使用するか | true |
| easingType | string | イージングの種類 | "EaseInOutQuad" |

## 利用可能なイージングタイプ

- `Linear` - 線形（等速）
- `EaseInQuad` - ゆっくり始まる（2次）
- `EaseOutQuad` - ゆっくり終わる（2次）
- `EaseInOutQuad` - ゆっくり始まってゆっくり終わる（2次）
- `EaseInCubic` - ゆっくり始まる（3次）
- `EaseOutCubic` - ゆっくり終わる（3次）
- `EaseInOutCubic` - ゆっくり始まってゆっくり終わる（3次）
- `EaseInQuart` - ゆっくり始まる（4次）
- `EaseOutQuart` - ゆっくり終わる（4次）
- `EaseInOutQuart` - ゆっくり始まってゆっくり終わる（4次）
- `EaseInQuint` - ゆっくり始まる（5次）
- `EaseOutQuint` - ゆっくり終わる（5次）
- `EaseInOutQuint` - ゆっくり始まってゆっくり終わる（5次）
- `EaseInBack` - 少し戻ってから始まる
- `EaseOutBack` - 少しオーバーシュートして終わる
- `EaseInOutBack` - 少し戻ってから始まり、少しオーバーシュートして終わる

## ImGuiエディター機能（デバッグビルドのみ）

デバッグビルドでは、ImGuiを使ってリアルタイムにカメラ演出を調整できます。

### エディターの使い方

1. **演出コントロール（Cinematic Control）**
   - 現在の演出状態を表示
   - 演出の手動開始/停止
   - 現在のカメラ位置の取得
   - JSONの読み込み/保存

2. **プリセット&シーケンス選択（Preset & Sequence Control）**
   - 登録されているプリセット演出の一覧表示と実行
   - 登録されているシーケンスの一覧表示と実行
   - ワンクリックで演出を試せる

3. **シーケンスエディター（Sequence Editor）** ★NEW★
   - カット割りシーケンスを視覚的に作成・編集
   - カットの追加、削除、並び替え
   - 各カットのパラメータをGUIで編集
   - シーケンスのプレビュー実行
   - JSONへの保存/読み込み
   - プリセットマネージャーへの登録

4. **カメラパラメータ（Camera Parameters）**
   - 最小/最大距離、距離スケール、マージン距離の調整
   - カメラの高さオフセットと俯角の調整
   - 画面パディングの調整
   - スムーズ補間速度の調整

5. **ステージ境界（Stage Bounds）**
   - ステージ境界の設定
   - 境界の有効/無効切り替え

6. **カメラシェイク（Camera Shake）**
   - プリセットシェイク（小・中・大）の実行
   - カスタムシェイクのパラメータ調整

7. **現在のステータス（Current Status）**
   - カメラの現在位置、注視点、距離の表示
   - シェイクオフセットの表示

### シーケンスエディターの使い方

1. **新規シーケンスの作成**
   ```
   a. 「シーケンス名」に名前を入力（例: "MyBossIntro"）
   b. 「新規カット追加」ボタンをクリック
   c. カット名をクリックして展開
   d. カットのタイプ、継続時間、位置、回転などを設定
   e. 「このカットをプレビュー」で確認
   f. 必要に応じてカットを追加
   g. 「↑」「↓」ボタンでカットの順序を変更
   h. 「JSONに保存」で保存
   i. 「プリセットに登録」でプリセットマネージャーに登録
   ```

2. **既存シーケンスの編集**
   ```
   a. 「Sequence JSON Path」にJSONファイルのパスを入力
   b. 「JSONから読み込み」をクリック
   c. カットリストが表示される
   d. カットをクリックして編集
   e. 編集後、「JSONに保存」で保存
   ```

3. **シーケンスのプレビュー**
   ```
   - 「シーケンスをプレビュー」ボタンでシーケンス全体を再生
   - 各カットの「このカットをプレビュー」で個別のカットを確認
   ```

4. **カットの編集項目**
   - **名前**: カットの識別名（例: "遠景ショット"）
   - **継続時間**: カットの長さ（秒）
   - **Type**: 演出タイプ（FixedPosition, LookAt, Dolly, Arc, Orbit）
   - **Start Position**: カメラの開始位置
   - **End Position**: カメラの終了位置（Dolly, Arcで使用）
   - **Target Position**: 注視点（LookAt, Orbitで使用）
   - **Start/End Rotation**: カメラの回転
   - **Orbit Radius**: 回転半径（Orbitで使用）
   - **Orbit Speed**: 回転速度（Orbitで使用）
   - **Use Easing**: イージングの有効/無効
   - **Easing Type**: イージングの種類

5. **便利な機能**
   - **削除**: カットを削除
   - **↑↓**: カットの順序を変更
   - **現在位置取得**: ゲーム中のカメラ位置をカットに設定
   - **合計時間表示**: シーケンス全体の再生時間を確認

### エディターを使った推奨ワークフロー

1. ゲームを実行してシーンをセットアップ
2. ImGuiエディターを開く
3. 「現在位置取得」でカメラ位置をコピー
4. シーケンスエディターで新規カットを追加
5. パラメータを調整
6. 「このカットをプレビュー」で確認
7. 複数のカットを作成
8. 「シーケンスをプレビュー」で全体を確認
9. JSONに保存
10. プリセットに登録して、ゲームコードから呼び出す

## サンプルJSONファイル

プロジェクトには以下のサンプルが含まれています：
- `Resources/Data/CameraCinematic_Opening.json` - ゲーム開始時のDolly演出
- `Resources/Data/CameraCinematic_BossDeath.json` - ボス死亡時のOrbit演出

これらをテンプレートとして使用し、自分の演出を作成してください。
