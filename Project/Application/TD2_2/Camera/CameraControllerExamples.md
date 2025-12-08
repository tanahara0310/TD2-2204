# カメラシステム使用例集

## 基本的な使い方

### 1. シンプルな演出開始

```cpp
// GameScene.cpp
void GameScene::Initialize(EngineSystem* engine) {
    // カメラコントローラーの初期化
    cameraController_->Initialize(camera_, player_, boss_);
    
    // デフォルトプリセットを登録
    CinematicPresetManager::GetInstance().RegisterDefaultPresets();
    
    // オープニング演出を開始
    cameraController_->StartCinematicByName("Opening");
}
```

### 2. ボス戦用のカット割り演出

```cpp
// GameScene.cpp
void GameScene::Initialize(EngineSystem* engine) {
    // ... 初期化コード ...
    
    // シーケンスを読み込み
    auto& presetManager = CinematicPresetManager::GetInstance();
    presetManager.LoadSequenceFromJson("BossIntro", 
        "Resources/Data/CameraSequence_BossIntro.json");
}

void GameScene::Update() {
    // ボス戦開始時
    if (bossPhase_ == BossPhase::Intro && !cinematicStarted_) {
        cameraController_->StartSequenceByName("BossIntro");
        cinematicStarted_ = true;
    }
    
    // 演出終了後にゲームプレイ開始
    if (cinematicStarted_ && !cameraController_->IsSequenceActive()) {
        bossPhase_ = BossPhase::Battle;
        cinematicStarted_ = false;
    }
}
```

## 高度な使用例

### 3. ダイナミックなターゲット位置を使用

```cpp
// ボス登場演出：ボスの位置を動的に更新
void GameScene::StartBossAppearance() {
    auto& presetManager = CinematicPresetManager::GetInstance();
    
    // プリセットを取得して改変
    auto config = *presetManager.GetPreset("BossAppear");
    config.targetPosition = boss_->GetWorldPosition(); // ボスの現在位置
    
    // カスタム演出として登録
    presetManager.RegisterPreset("BossAppear_Dynamic", config);
    
    // 演出開始
    cameraController_->StartCinematicByName("BossAppear_Dynamic");
}
```

### 4. 複数のシーケンスを連続再生

```cpp
class GameScene {
private:
    std::vector<std::string> sequenceQueue_;
    bool playingSequence_ = false;

public:
    void QueueSequences() {
        sequenceQueue_.push_back("Opening");
        sequenceQueue_.push_back("BossIntro");
        sequenceQueue_.push_back("BattleStart");
    }
    
    void Update() {
        // シーケンスキューの処理
        if (!sequenceQueue_.empty() && !cameraController_->IsSequenceActive()) {
            std::string nextSeq = sequenceQueue_.front();
            sequenceQueue_.erase(sequenceQueue_.begin());
            
            cameraController_->StartSequenceByName(nextSeq);
        }
    }
};
```

### 5. イベントトリガーによる演出切り替え

```cpp
void GameScene::Update() {
    // ボスのHPに応じた演出変更
    if (boss_->GetHP() <= boss_->GetMaxHP() * 0.5f && !phaseTransitionStarted_) {
        // 第2形態への移行演出
        auto sequence = std::make_shared<CinematicSequence>();
        
        // カット1: ボスに急接近
        CinematicCut cut1;
        cut1.duration = 1.0f;
        cut1.config.type = CameraController::CinematicType::Dolly;
        cut1.config.startPosition = cameraController_->GetCurrentCameraPos();
        cut1.config.endPosition = boss_->GetWorldPosition() + Vector3{0, 2, -10};
        cut1.config.useEasing = true;
        cut1.config.easingType = "EaseInQuad";
        sequence->AddCut(cut1);
        
        // カット2: ボスを中心に回転
        CinematicCut cut2;
        cut2.duration = 2.0f;
        cut2.config.type = CameraController::CinematicType::Orbit;
        cut2.config.targetPosition = boss_->GetWorldPosition();
        cut2.config.orbitRadius = 15.0f;
        cut2.config.orbitSpeed = 1.5f;
        sequence->AddCut(cut2);
        
        // カット3: 引きのショット
        CinematicCut cut3;
        cut3.duration = 1.5f;
        cut3.config.type = CameraController::CinematicType::Dolly;
        cut3.config.startPosition = boss_->GetWorldPosition() + Vector3{0, 5, -15};
        cut3.config.endPosition = {0, 10, -50};
        cut3.config.useEasing = true;
        cut3.config.easingType = "EaseOutQuad";
        sequence->AddCut(cut3);
        
        cameraController_->StartSequence(sequence);
        phaseTransitionStarted_ = true;
    }
}
```

### 6. カメラシェイクと演出の組み合わせ

```cpp
void GameScene::OnBossAttack() {
    // 攻撃時にカメラシェイク（演出中でも有効）
    cameraController_->StartShake(CameraController::ShakeIntensity::Medium);
}

void GameScene::OnBossDeath() {
    // 死亡時の演出
    if (!cameraController_->IsSequenceActive()) {
        // 大きなシェイクを開始
        cameraController_->StartShake(0.8f, 1.0f, 15.0f, 0.7f);
        
        // 0.5秒後に死亡演出シーケンスを開始
        deathSequenceTimer_.Start(0.5f, false);
    }
    
    if (deathSequenceTimer_.IsFinished()) {
        cameraController_->StartSequenceByName("BossDeathSequence");
    }
}
```

## JSON設定のテンプレート

### カット割りシーケンス - ドラマチックな登場

```json
{
  "cuts": [
    {
      "name": "静寂のショット",
      "duration": 1.5,
      "type": "FixedPosition",
      "startPosition": [0.0, 30.0, -100.0],
      "targetPosition": [0.0, 0.0, 0.0],
      "startRotation": [0.3, 0.0, 0.0],
      "useEasing": true,
      "easingType": "Linear"
    },
    {
      "name": "高速接近",
      "duration": 1.2,
      "type": "Dolly",
      "startPosition": [0.0, 25.0, -80.0],
      "endPosition": [0.0, 8.0, -35.0],
      "startRotation": [0.25, 0.0, 0.0],
      "endRotation": [0.1, 0.0, 0.0],
      "useEasing": true,
      "easingType": "EaseInCubic"
    },
    {
      "name": "ローアングル",
      "duration": 1.0,
      "type": "LookAt",
      "startPosition": [0.0, 1.0, -15.0],
      "targetPosition": [0.0, 5.0, 0.0],
      "useEasing": true,
      "easingType": "EaseOutQuad"
    },
    {
      "name": "サークル撮影",
      "duration": 2.5,
      "type": "Orbit",
      "targetPosition": [0.0, 3.0, 0.0],
      "startPosition": [0.0, 5.0, 0.0],
      "orbitRadius": 20.0,
      "orbitSpeed": 1.0,
      "useEasing": true,
      "easingType": "EaseInOutQuad"
    }
  ]
}
```

### カット割りシーケンス - アクションシーン

```json
{
  "cuts": [
    {
      "name": "ワイドショット",
      "duration": 0.8,
      "type": "FixedPosition",
      "startPosition": [-40.0, 10.0, -20.0],
      "targetPosition": [0.0, 2.0, 0.0],
      "useEasing": true,
      "easingType": "Linear"
    },
    {
      "name": "クイックカット1",
      "duration": 0.6,
      "type": "LookAt",
      "startPosition": [20.0, 5.0, -15.0],
      "targetPosition": [0.0, 3.0, 0.0],
      "useEasing": true,
      "easingType": "Linear"
    },
    {
      "name": "クイックカット2",
      "duration": 0.6,
      "type": "LookAt",
      "startPosition": [-15.0, 8.0, 20.0],
      "targetPosition": [0.0, 2.0, 0.0],
      "useEasing": true,
      "easingType": "Linear"
    },
    {
      "name": "スローモーション",
      "duration": 2.0,
      "type": "Dolly",
      "startPosition": [0.0, 3.0, -25.0],
      "endPosition": [0.0, 1.5, -12.0],
      "startRotation": [0.05, 0.0, 0.0],
      "endRotation": [0.15, 0.0, 0.0],
      "useEasing": true,
      "easingType": "EaseInOutQuint"
    }
  ]
}
```

## デバッグとチューニング

### ImGuiでのリアルタイム調整

```cpp
#ifdef _DEBUG
void GameScene::DrawImGui() {
    if (ImGui::Begin("演出制御")) {
        // プリセット一覧を表示
        auto& presetManager = CinematicPresetManager::GetInstance();
        auto presetNames = presetManager.GetPresetNames();
        
        static int selectedPreset = 0;
        if (ImGui::BeginCombo("プリセット", presetNames[selectedPreset].c_str())) {
            for (int i = 0; i < presetNames.size(); i++) {
                bool isSelected = (selectedPreset == i);
                if (ImGui::Selectable(presetNames[i].c_str(), isSelected)) {
                    selectedPreset = i;
                }
            }
            ImGui::EndCombo();
        }
        
        if (ImGui::Button("プリセット開始")) {
            cameraController_->StartCinematicByName(presetNames[selectedPreset]);
        }
        
        // シーケンス一覧を表示
        auto sequenceNames = presetManager.GetSequenceNames();
        static int selectedSequence = 0;
        if (ImGui::BeginCombo("シーケンス", 
            selectedSequence < sequenceNames.size() ? 
            sequenceNames[selectedSequence].c_str() : "なし")) {
            for (int i = 0; i < sequenceNames.size(); i++) {
                bool isSelected = (selectedSequence == i);
                if (ImGui::Selectable(sequenceNames[i].c_str(), isSelected)) {
                    selectedSequence = i;
                }
            }
            ImGui::EndCombo();
        }
        
        if (ImGui::Button("シーケンス開始")) {
            if (selectedSequence < sequenceNames.size()) {
                cameraController_->StartSequenceByName(sequenceNames[selectedSequence]);
            }
        }
    }
    ImGui::End();
}
#endif
```

### ImGuiシーケンスエディターの活用

```cpp
// シーケンスエディターを使った演出制作のワークフロー例

// 1. ゲームを実行し、適切なシーンを表示
// 2. ImGuiで「Camera Controller」ウィンドウを開く
// 3. 「Sequence Editor」セクションを開く
// 4. 以下の手順で演出を作成：

// ステップ1: シーケンス名を設定
// - 「シーケンス名」フィールドに "BossPhase2Transition" と入力

// ステップ2: 最初のカットを追加
// - 「新規カット追加」をクリック
// - カット名を "緊張の瞬間" に変更
// - Type: FixedPosition
// - Duration: 2.0
// - 「現在位置取得」でカメラ位置をコピー
// - 「このカットをプレビュー」で確認

// ステップ3: 2つ目のカットを追加（接近ショット）
// - 「新規カット追加」をクリック
// - カット名を "ボスへ接近" に変更
// - Type: Dolly
// - Duration: 1.5
// - Start Position: 現在のカメラ位置
// - End Position: ボスの近く
// - Easing Type: EaseInCubic
// - 「このカットをプレビュー」で確認

// ステップ4: 3つ目のカット（回転ショット）
// - 「新規カット追加」をクリック
// - カット名を "変身演出" に変更
// - Type: Orbit
// - Duration: 3.0
// - Target Position: ボスの位置
// - Orbit Radius: 20.0
// - Orbit Speed: 1.2
// - 「このカットをプレビュー」で確認

// ステップ5: シーケンス全体をプレビュー
// - 「シーケンスをプレビュー」をクリック
// - 全体の流れを確認

// ステップ6: 調整とリファイン
// - 各カットのタイミングや位置を微調整
// - カットの順序を「↑」「↓」ボタンで変更
// - 不要なカットは「削除」ボタンで削除

// ステップ7: 保存と登録
// - Sequence JSON Path: "Resources/Data/CameraSequence_BossPhase2.json"
// - 「JSONに保存」をクリック
// - 「プリセットに登録」をクリック

// ステップ8: コードから使用
void GameScene::OnBossPhase2Start() {
    cameraController_->StartSequenceByName("BossPhase2Transition");
}
```

### シーケンスエディターの便利な機能

```cpp
// 1. カット位置の素早い設定
// ゲーム中にカメラを手動で動かして理想的な位置を見つけたら、
// エディターの「現在位置取得」ボタンで即座にカットに反映

// 2. カットの複製と微調整
// JSONから読み込んだシーケンスをベースに、
// パラメータを少し変えて新しいバリエーションを作成

// 3. テンプレートカットの作成
// よく使うカット構成をテンプレートとして保存し、
// 必要に応じて読み込んで使用

// 実装例：テンプレートシーケンスの管理
class CameraTemplateManager {
public:
    static std::shared_ptr<CinematicSequence> CreateDramaticIntro() {
        auto seq = std::make_shared<CinematicSequence>();
        
        // カット1: 静寂
        CinematicCut cut1;
        cut1.name = "静寂";
        cut1.duration = 1.5f;
        cut1.config.type = CameraController::CinematicType::FixedPosition;
        // ... 設定 ...
        seq->AddCut(cut1);
        
        // カット2: 急接近
        CinematicCut cut2;
        cut2.name = "急接近";
        cut2.duration = 1.0f;
        cut2.config.type = CameraController::CinematicType::Dolly;
        // ... 設定 ...
        seq->AddCut(cut2);
        
        return seq;
    }
    
    static std::shared_ptr<CinematicSequence> CreateActionSequence() {
        // 複数のクイックカットを含むアクションシーケンス
        auto seq = std::make_shared<CinematicSequence>();
        // ... カットを追加 ...
        return seq;
    }
};

// 使用例
void GameScene::Initialize(EngineSystem* engine) {
    auto& presetManager = CinematicPresetManager::GetInstance();
    
    // テンプレートシーケンスを登録
    auto dramaticIntro = CameraTemplateManager::CreateDramaticIntro();
    presetManager.RegisterSequence("Template_DramaticIntro", dramaticIntro);
    
    auto actionSeq = CameraTemplateManager::CreateActionSequence();
    presetManager.RegisterSequence("Template_Action", actionSeq);
}
