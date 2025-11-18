# GameTimer クラス - README

このフォルダには、`GameTimer` クラスの使い方とサンプルコードをまとめた README とサンプルファイルが含まれています。

## 目次
- 概要
- 主な機能
- API（メソッド一覧）
- よくある使い方（コード例）
  - 秒単位タイマーの基本
  - フレーム単位タイマーの使用
  - コールバックの追加
  - タイムスケールの利用
  - ImGui デバッグ表示（デバッグビルドのみ）
- 注意点 / 実装上のポイント
- サンプルファイル

---

## 概要
`GameTimer` は、ゲーム内の演出・シーン遷移・アニメーションなどのタイミング制御に使う汎用タイマーです。
- 秒単位（経過時間ベース）とフレーム単位の両方に対応
- ループ / ノンループ
- タイムスケール（時間の速さ）対応
- 任意の時刻でコールバックを登録可能
- デバッグビルドでは ImGui による視覚化が可能

---

## 主な機能
- Start / Stop / Reset / Pause / Resume
- StartFrames（フレーム指定で開始）
- GetProgress / GetEasedProgress（EasingUtil を適用可）
- SetTimeScale / GetTimeScale
- AddCallback / AddCallbackAtProgress / ClearCallbacks
- DrawImGui（_DEBUG 時）

---

## API（抜粋）
```cpp
GameTimer();
GameTimer(float duration, bool loop = false);
void Update(float deltaTime);
void Start(float duration, bool loop = false);
void Stop();
void Reset();
void Pause();
void Resume();
bool IsActive() const;
bool IsFinished() const;
float GetProgress() const; // 0.0 - 1.0
float GetEasedProgress(EasingUtil::Type easingType = EasingUtil::Type::Linear) const;
float GetRemainingTime() const;
float GetElapsedTime() const;
float GetDuration() const;
bool IsLoop() const;
bool HasLooped() const;
void SetDuration(float duration);
void SetLoop(bool loop);

void StartFrames(int frameCount, bool loop = false, float targetFPS = 60.0f);
int GetCurrentFrame() const;
int GetTotalFrames() const;

void SetTimeScale(float scale);
float GetTimeScale() const;

void AddCallback(float triggerTime, std::function<void()> callback);
void AddCallbackAtProgress(float progress, std::function<void()> callback);
void ClearCallbacks();

#ifdef _DEBUG
void DrawImGui(const char* label = "Timer");
#endif

void SetName(const char* name);
```

---

## よくある使い方（コード例）

### 秒単位タイマーの基本
```cpp
GameTimer timer;
timer.Start(3.0f, false); // 3秒、ループしない

// ゲームループ内
void Update(float deltaTime) {
    timer.Update(deltaTime);
    if (timer.IsFinished()) {
        // 3秒経過時の処理
    }
}
```

### ループ時の判定（HasLooped）
```cpp
GameTimer loopTimer(2.0f, true); // 2秒でループ

// ゲームループ内
void Update(float deltaTime) {
    loopTimer.Update(deltaTime);
    if (loopTimer.HasLooped()) {
        // ループ完了して再スタートした瞬間に呼ばれる
        // 例: 敵の攻撃サイクル、アニメーションループ時の処理
    }
}
```

### フレーム単位タイマー
```cpp
GameTimer frameTimer;
frameTimer.StartFrames(180, false, 60.0f); // 180フレーム, 60FPS想定 -> 3秒

// 更新は通常通り deltaTime を渡す
frameTimer.Update(deltaTime);
int currentFrame = frameTimer.GetCurrentFrame();
```

### コールバックの追加
```cpp
GameTimer cbTimer(5.0f); // 5秒
cbTimer.AddCallback(1.0f, [](){ /* 1秒時の処理 */ });
cbTimer.AddCallbackAtProgress(0.5f, [](){ /* 50%到達時の処理 */ });
cbTimer.Start(5.0f);
```

### タイムスケールの利用
```cpp
timer.SetTimeScale(0.5f); // 遅くする
timer.SetTimeScale(2.0f); // 速くする
```

### ImGui デバッグ（デバッグビルドで有効）
```cpp
#ifdef _DEBUG
timer.DrawImGui("MyTimer");
#endif
```

---

## 注意点 / 実装上のポイント
- `Update` に渡す `deltaTime` は「前フレームの経過秒数」です。フレームレート依存の挙動を避けるため正確な delta を渡してください。
- `StartFrames` は内部でフレーム数を秒に変換しているため、`targetFPS` を実際の動作フレームレートに合わせること。
- ループ時は `currentTime_` が 0 に戻り、コールバックの発火フラグもリセットされます（同じコールバックをループごとに再発火させたい場合に便利）。
- `SetDuration` を途中で呼ぶと、既に経過済みの場合は `finished_` フラグが更新されます。
- `timeScale_` は 0 未満に設定されないように保護されています（0 が設定されると時間が止まります）。
- Easing を使う場合は `EasingUtil` がリンクされていることを確認してください。

---

## サンプルファイル
- `sample_usage.cpp` : README に沿った簡単な使用例（このフォルダに同梱）

---

## ライセンス
内部利用や学習目的であれば自由に利用してください。外部配布時は既存のプロジェクトのライセンスに従ってください。

