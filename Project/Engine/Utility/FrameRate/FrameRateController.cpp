#include "FrameRateController.h"
#include <algorithm>
#include <thread>

void FrameRateController::Initialize()
{
    // 時刻の初期化
    lastFrameTime_ = std::chrono::high_resolution_clock::now();
    frameStartTime_ = lastFrameTime_;
    
    // デルタタイムの初期化
    deltaTime_ = kFixedDeltaTime;
    
    // FPS計測用の初期化
    std::fill(fpsSamples_, fpsSamples_ + kFPSSampleCount, kTargetFPS);
    fpsSampleIndex_ = 0;
    validSampleCount_ = 0;
    currentFPS_ = kTargetFPS;
    
    // 診断情報の初期化
    minFPS_ = kTargetFPS;
    maxFPS_ = kTargetFPS;
    minMaxFrameCounter_ = 0;
    actualFrameTimeMs_ = 0.0f;
    waitTimeMs_ = 0.0f;
    processTimeMs_ = 0.0f;
    droppedFrameCount_ = 0;
}

void FrameRateController::BeginFrame()
{
    // フレーム開始時刻を記録
    frameStartTime_ = std::chrono::high_resolution_clock::now();
    
    // デルタタイムは60FPS固定値を使用
    deltaTime_ = kFixedDeltaTime;
    
    // FPS計測を更新
    UpdateFPSCalculation();
    
    // 最小・最大FPSの更新
    UpdateMinMaxFPS();
    
    // 次フレームのために現在時刻を保存
    lastFrameTime_ = frameStartTime_;
}

void FrameRateController::EndFrame()
{
    // フレーム処理時間を計測
    auto beforeWait = std::chrono::high_resolution_clock::now();
    auto processDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        beforeWait - frameStartTime_
    );
    processTimeMs_ = processDuration.count() / 1000.0f;
    
    // VSync有効時は待機処理をスキップ（GPUとディスプレイが同期を取る）
    // 実際のフレーム時間は次のBeginFrameで計測される
    
    // 実際のフレーム時間を計測（待機なしバージョン）
    auto afterProcess = std::chrono::high_resolution_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        afterProcess - frameStartTime_
    );
    actualFrameTimeMs_ = frameDuration.count() / 1000.0f;
    
    // 待機時間は0（VSyncがハンドルする）
    waitTimeMs_ = 0.0f;
}

void FrameRateController::UpdateFPSCalculation()
{
    // 前フレームからの実測経過時間を計算
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        frameStartTime_ - lastFrameTime_
    );
    float actualDeltaTime = duration.count() / 1000000.0f;
    
    // 実測FPSを計算（異常値のガード）
    float instantFPS = kTargetFPS;
    if (actualDeltaTime > 0.0001f) { // 0除算と異常に小さい値を防ぐ
        instantFPS = 1.0f / actualDeltaTime;
        instantFPS = std::clamp(instantFPS, 1.0f, 1000.0f); // 1〜1000FPSの範囲に制限
    }
    
    // FPSドロップを検出（目標の95%を下回った場合）
    if (instantFPS < kTargetFPS * 0.95f) {
        droppedFrameCount_++;
    }
    
    // サンプル配列を更新
    fpsSamples_[fpsSampleIndex_] = instantFPS;
    fpsSampleIndex_ = (fpsSampleIndex_ + 1) % kFPSSampleCount;
    
    // 有効なサンプル数を更新（初回60サンプルまで増加）
    if (validSampleCount_ < kFPSSampleCount) {
        validSampleCount_++;
    }
    
    // 移動平均を計算
    float totalFPS = 0.0f;
    for (int i = 0; i < validSampleCount_; ++i) {
        totalFPS += fpsSamples_[i];
    }
    currentFPS_ = totalFPS / validSampleCount_;
}

void FrameRateController::UpdateMinMaxFPS()
{
    minMaxFrameCounter_++;
    
    // 最小FPSを更新
    if (currentFPS_ < minFPS_) {
        minFPS_ = currentFPS_;
    }
    
    // 最大FPSを更新
    if (currentFPS_ > maxFPS_) {
        maxFPS_ = currentFPS_;
    }
    
    // 一定期間ごとに最小・最大をリセット（2秒分）
    if (minMaxFrameCounter_ >= kMinMaxSampleCount) {
        minFPS_ = currentFPS_;
        maxFPS_ = currentFPS_;
        minMaxFrameCounter_ = 0;
    }
}

void FrameRateController::WaitForTargetFrameTime(
    const std::chrono::high_resolution_clock::time_point& frameStartTime)
{
    // VSync有効時はこの関数は呼ばれない想定
    // Present(1, 0)で自動的に60Hzに同期される
    
    // 現在の経過時間を取得
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        currentTime - frameStartTime
    );
    float elapsedSeconds = elapsed.count() / 1000000.0f;
    
    // 残り待機時間を計算
    float remainingTime = kTargetFrameTime - elapsedSeconds;
    
    if (remainingTime <= 0.0f) {
        return; // すでに目標時間を超過している
    }
    
    // 高精度待機：sleep_forとyield_thisを組み合わせる
    // 1ms以上残っている場合はスリープで待機
    if (remainingTime > 0.001f) {
        auto sleepDuration = std::chrono::microseconds(
            static_cast<long long>((remainingTime - 0.0005f) * 1000000)
        );
        std::this_thread::sleep_for(sleepDuration);
    }
    
    // 残り時間を軽量なyield待機で消費（CPUを過度に使用しない）
    auto targetTime = frameStartTime + std::chrono::microseconds(
        static_cast<long long>(kTargetFrameTime * 1000000)
    );
    
    while (std::chrono::high_resolution_clock::now() < targetTime) {
        // スピンウェイトの代わりにyieldを使用してCPU使用率を抑える
        std::this_thread::yield();
    }
}