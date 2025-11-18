#include "GameTimer.h"
#include <algorithm>
#include <cmath>

#ifdef _DEBUG
#include <imgui.h>
#endif

GameTimer::GameTimer(float duration, bool loop) 
    : duration_(duration), loop_(loop) {
}

void GameTimer::Update(float deltaTime) {
    if (!isActive_) return;
    
    // 前フレームのループフラグをリセット
    loopedThisFrame_ = false;
    
    // タイムスケールを適用
    float scaledDeltaTime = deltaTime * timeScale_;
    currentTime_ += scaledDeltaTime;
    
    // コールバックをチェック
    CheckAndExecuteCallbacks();
    
    if (currentTime_ >= duration_) {
        finished_ = true;
        
        if (loop_) {
            currentTime_ = 0.0f;
            finished_ = false;  // ループ時は完了フラグをリセット
            loopedThisFrame_ = true;  // ループ発生をマーク
            
            // ループ時はコールバックの発火状態もリセット
            for (auto& callback : callbacks_) {
                callback.triggered = false;
            }
        } else {
            isActive_ = false;
        }
    }
}

void GameTimer::Start(float duration, bool loop) {
    duration_ = duration;
    loop_ = loop;
    currentTime_ = 0.0f;
    isActive_ = true;
    finished_ = false;
    useFrameMode_ = false;
    loopedThisFrame_ = false;  // ループフラグもリセット
    
    // コールバックの発火状態をリセット
    for (auto& callback : callbacks_) {
        callback.triggered = false;
    }
}

void GameTimer::Stop() {
    isActive_ = false;
}

void GameTimer::Reset() {
    currentTime_ = 0.0f;
    isActive_ = false;
    finished_ = false;
    loopedThisFrame_ = false;  // ループフラグもリセット
    
    // コールバックの発火状態をリセット
    for (auto& callback : callbacks_) {
        callback.triggered = false;
    }
}

void GameTimer::Pause() {
    isActive_ = false;
}

void GameTimer::Resume() {
    if (currentTime_ < duration_) {
        isActive_ = true;
        finished_ = false;
    }
}

bool GameTimer::IsActive() const {
    return isActive_;
}

bool GameTimer::IsFinished() const {
    return finished_;
}

float GameTimer::GetProgress() const {
    if (duration_ <= 0.0f) return 1.0f;
    return (std::min)(1.0f, currentTime_ / duration_);
}

float GameTimer::GetEasedProgress(EasingUtil::Type easingType) const {
    float progress = GetProgress();
    return EasingUtil::Apply(progress, easingType);  // ★★★ EasingUtilを使用 ★★★
}

float GameTimer::GetRemainingTime() const {
    return (std::max)(0.0f, duration_ - currentTime_);
}

float GameTimer::GetElapsedTime() const {
    return currentTime_;
}

float GameTimer::GetDuration() const {
    return duration_;
}

bool GameTimer::IsLoop() const {
    return loop_;
}

bool GameTimer::HasLooped() const {
    return loopedThisFrame_;
}

void GameTimer::SetDuration(float duration) {
    duration_ = duration;
    // 現在時間が新しい継続時間を超えている場合の処理
    if (currentTime_ >= duration_ && isActive_) {
        finished_ = true;
        if (!loop_) {
            isActive_ = false;
        }
    }
}

void GameTimer::SetLoop(bool loop) {
    loop_ = loop;
}

// ★★★ 新機能：フレームカウンター ★★★

void GameTimer::StartFrames(int frameCount, bool loop, float targetFPS) {
    totalFrames_ = frameCount;
    targetFPS_ = targetFPS;
    duration_ = frameCount / targetFPS;  // フレーム数をタイムに変換
    loop_ = loop;
    currentTime_ = 0.0f;
    isActive_ = true;
    finished_ = false;
    useFrameMode_ = true;
    loopedThisFrame_ = false;  // ループフラグもリセット
    
    // コールバックの発火状態をリセット
    for (auto& callback : callbacks_) {
        callback.triggered = false;
    }
}

int GameTimer::GetCurrentFrame() const {
    if (!useFrameMode_) return 0;
    return static_cast<int>(currentTime_ * targetFPS_);
}

int GameTimer::GetTotalFrames() const {
    return totalFrames_;
}

// ★★★ 新機能：タイムスケール ★★★

void GameTimer::SetTimeScale(float scale) {
    timeScale_ = (std::max)(0.0f, scale);  // 負の値は防ぐ
}

float GameTimer::GetTimeScale() const {
    return timeScale_;
}

// ★★★ 新機能：コールバック ★★★

void GameTimer::AddCallback(float triggerTime, std::function<void()> callback) {
    TimerCallback timerCallback;
    timerCallback.triggerTime = triggerTime;
    timerCallback.callback = callback;
    timerCallback.triggered = false;
    callbacks_.push_back(timerCallback);
}

void GameTimer::AddCallbackAtProgress(float progress, std::function<void()> callback) {
    float triggerTime = duration_ * progress;
    AddCallback(triggerTime, callback);
}

void GameTimer::ClearCallbacks() {
    callbacks_.clear();
}

// ★★★ 新機能：デバッグ表示 ★★★
#ifdef _DEBUG
void GameTimer::DrawImGui(const char* label) {
    ImGui::PushID(this);  // 複数のタイマーがある場合のID衝突を防ぐ
    
    if (ImGui::CollapsingHeader(label)) {
        ImGui::Text("Name: %s", name_.c_str());
        ImGui::Text("Status: %s", isActive_ ? "ACTIVE" : (finished_ ? "FINISHED" : "STOPPED"));
        
        // 基本情報
        ImGui::Separator();
        ImGui::Text("Time: %.3f / %.3f sec", currentTime_, duration_);
        ImGui::Text("Progress: %.1f%%", GetProgress() * 100.0f);
        ImGui::Text("Remaining: %.3f sec", GetRemainingTime());
        
        // プログレスバー
        ImGui::ProgressBar(GetProgress(), ImVec2(-1.0f, 0.0f));
        
        // フレームモード情報
        if (useFrameMode_) {
            ImGui::Separator();
            ImGui::Text("Frame Mode: %d / %d frames", GetCurrentFrame(), totalFrames_);
            ImGui::Text("Target FPS: %.1f", targetFPS_);
        }
        
        // タイムスケール
        ImGui::Separator();
        ImGui::Text("Time Scale: %.2fx", timeScale_);
        if (ImGui::SliderFloat("##TimeScale", &timeScale_, 0.0f, 3.0f, "%.2fx")) {
            SetTimeScale(timeScale_);
        }
        
        // 制御ボタン
        ImGui::Separator();
        if (ImGui::Button("Start")) { Start(duration_, loop_); }
        ImGui::SameLine();
        if (ImGui::Button("Stop")) { Stop(); }
        ImGui::SameLine();
        if (ImGui::Button("Reset")) { Reset(); }
        
        if (isActive_) {
            if (ImGui::Button("Pause")) { Pause(); }
        } else if (currentTime_ < duration_) {
            if (ImGui::Button("Resume")) { Resume(); }
        }
        
        // ループ設定とループ状態表示
        ImGui::Checkbox("Loop", &loop_);
        if (loop_) {
            ImGui::SameLine();
            if (loopedThisFrame_) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "[LOOPED THIS FRAME]");
            } else {
                ImGui::Text("[Loop Enabled]");
            }
        }
        
        // コールバック情報
        if (!callbacks_.empty()) {
            ImGui::Separator();
            ImGui::Text("Callbacks: %zu", callbacks_.size());
            for (size_t i = 0; i < callbacks_.size(); ++i) {
                const auto& cb = callbacks_[i];
                ImGui::Text("  [%zu] %.3fs %s", i, cb.triggerTime, cb.triggered ? "(FIRED)" : "");
            }
        }
    }
    
    ImGui::PopID();
}
#endif

void GameTimer::SetName(const char* name) {
    name_ = name;
}

// ★★★ プライベートメソッド ★★★

void GameTimer::CheckAndExecuteCallbacks() {
    for (auto& callback : callbacks_) {
        if (!callback.triggered && currentTime_ >= callback.triggerTime) {
            callback.triggered = true;
            if (callback.callback) {
                callback.callback();
            }
        }
    }
}