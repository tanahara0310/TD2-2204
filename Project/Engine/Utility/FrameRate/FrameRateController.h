#pragma once

#include <chrono>

/// @brief フレームレート管理クラス（60FPS固定）
/// @details エンジン全体で60FPSに固定されており、フレーム時間の管理とFPS計測を行う
class FrameRateController {
public:
    /// @brief 初期化
    void Initialize();

    /// @brief フレーム開始時の処理
    /// @details 前フレームからの経過時間を計算し、FPS計測を更新
    void BeginFrame();

    /// @brief フレーム終了時の処理
    /// @details 60FPS維持のための待機処理を実行
    void EndFrame();

    /// @brief フレーム間の経過時間を取得（秒）
    /// @return deltaTime（秒） - 60FPS固定のため約0.0167秒
    float GetDeltaTime() const { return deltaTime_; }

    /// @brief 現在のFPSを取得
    /// @return 実測FPS値（60サンプルの移動平均）
    float GetCurrentFPS() const { return currentFPS_; }

    /// @brief 目標FPSを取得
    /// @return 60.0f固定
    float GetTargetFPS() const { return kTargetFPS; }

    /// @brief 最小FPSを取得（診断用）
    /// @return 最近のフレームでの最小FPS
    float GetMinFPS() const { return minFPS_; }

    /// @brief 最大FPSを取得（診断用）
    /// @return 最近のフレームでの最大FPS
    float GetMaxFPS() const { return maxFPS_; }

    /// @brief 実際のフレーム時間を取得（ミリ秒）
    /// @return 前フレームの実際の経過時間（ms）
    float GetActualFrameTimeMs() const { return actualFrameTimeMs_; }

    /// @brief 待機時間を取得（ミリ秒）
    /// @return 前フレームでの待機時間（ms）
    float GetWaitTimeMs() const { return waitTimeMs_; }

    /// @brief 処理時間を取得（ミリ秒）
    /// @return 前フレームでの実際の処理時間（ms）
    float GetProcessTimeMs() const { return processTimeMs_; }

    /// @brief FPSドロップの回数を取得
    /// @return 目標FPSを下回ったフレームの累計
    int GetDroppedFrameCount() const { return droppedFrameCount_; }

private:
    /// @brief FPS計測値を更新
    void UpdateFPSCalculation();

    /// @brief 目標フレーム時間まで待機
    /// @param frameStartTime フレーム開始時刻
    void WaitForTargetFrameTime(const std::chrono::high_resolution_clock::time_point& frameStartTime);

    /// @brief 最小・最大FPSを更新
    void UpdateMinMaxFPS();

private:

    // 固定値
    static constexpr float kTargetFPS = 60.0f;                    // 目標FPS
    static constexpr float kFixedDeltaTime = 1.0f / kTargetFPS;   // 固定デルタタイム
    static constexpr float kTargetFrameTime = kFixedDeltaTime;    // 目標フレーム時間
    static constexpr int kFPSSampleCount = 60;                    // FPS計測用サンプル数
    static constexpr int kMinMaxSampleCount = 120;                // 最小・最大FPSの計測期間（2秒分）
    
    // 時間管理
    std::chrono::high_resolution_clock::time_point lastFrameTime_;  // 前フレームの開始時刻
    std::chrono::high_resolution_clock::time_point frameStartTime_; // 現フレームの開始時刻
    float deltaTime_ = kFixedDeltaTime;                            // フレーム間経過時間
    
    // FPS計測
    float fpsSamples_[kFPSSampleCount] = {};    // FPS計測用サンプル配列
    int fpsSampleIndex_ = 0;                    // 現在のサンプルインデックス
    int validSampleCount_ = 0;                  // 有効なサンプル数（初期化中は<60）
    float currentFPS_ = kTargetFPS;             // 現在のFPS（移動平均）
    
    // 診断情報
    float minFPS_ = kTargetFPS;                 // 最小FPS
    float maxFPS_ = kTargetFPS;                 // 最大FPS
    int minMaxFrameCounter_ = 0;                // 最小・最大FPS更新用カウンター
    float actualFrameTimeMs_ = 0.0f;            // 実際のフレーム時間（ms）
    float waitTimeMs_ = 0.0f;                   // 待機時間（ms）
    float processTimeMs_ = 0.0f;                // 処理時間（ms）
    int droppedFrameCount_ = 0;                 // FPSドロップ回数
};