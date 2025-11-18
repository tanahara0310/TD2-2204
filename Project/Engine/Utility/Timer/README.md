# GameTimer クラス 使用方法

`GameTimer` は、一定時間で完了する処理やアニメーション、演出などを管理するための汎用タイマーです。  
以下では **初めて使う人向けに最低限必要な使い方だけ** をまとめています。

---

## 1. 準備

### インクルード
```cpp
#include "Engine/Utility/Timer/GameTimer.h"
```

### インスタンス作成
```cpp
GameTimer timer;
```

---

## 2. タイマーの開始

### 秒指定で開始
```cpp
timer.Start(3.0f, false); // 3秒、ループなし
```

### フレーム数で開始
```cpp
timer.StartFrames(60, false, 60.0f); // 60フレーム、ループなし、FPS 60基準
```

---

## 3. 毎フレーム更新

必ず `Update(deltaTime)` を毎フレーム呼びます。

```cpp
timer.Update(deltaTime);
```

`deltaTime` はゲームループで計算された1フレームの経過時間(秒)です。

---

## 4. タイマーの状態取得

```cpp
timer.IsActive();        // 動作中か
timer.IsFinished();      // 終了したか
timer.GetElapsedTime();  // 経過時間(秒)
timer.GetRemainingTime();// 残り時間(秒)
timer.GetDuration();     // 総時間(秒)
timer.GetProgress();     // 進行度(0.0～1.0)
```

---

## 5. イージングを適用した進行度

```cpp
float t = timer.GetEasedProgress(EasingUtil::Type::EaseOutCubic);
```

---

## 6. 一時停止・再開・停止・リセット

```cpp
timer.Pause();
timer.Resume();
timer.Stop();
timer.Reset();
```

---

## 7. タイムスケール（スロー/加速）

```cpp
timer.SetTimeScale(0.5f); // 半分の速さ（スロー）
timer.SetTimeScale(2.0f); // 2倍速
```

---

## 8. コールバック（時間または進行度で一度だけ実行）

### 時間によるコールバック
```cpp
timer.AddCallback(1.5f, []() {
    // 1.5秒経過時に1回だけ実行される
});
```

### 進行度によるコールバック
```cpp
timer.AddCallbackAtProgress(0.5f, []() {
    // 進行度 50% の瞬間に1回実行
});
```

---

## 9. ループタイマーとループ検知

### ループ開始
```cpp
timer.Start(1.0f, true); // 1秒ループ
```

### このフレームでループしたか確認
```cpp
if (timer.HasLooped()) {
    // ループ境界に到達したフレームでのみ true
}
```

---

## 10. ImGui でのデバッグ表示（Debug ビルド）

```cpp
timer.DrawImGui("MyTimer");
```

タイマーの状態やボタン操作ができ、進行状況を可視化できます。

---

## まとめ
- `Start()`→`Update()`→`GetProgress()` が基本運用
- イージング・スロー・コールバック・ループなど幅広く利用可能
- Debug ビルドでは `DrawImGui()` による可視化も対応

