#pragma once

#include <memory>

class EngineSystem;
class PostEffectManager;
class FadeEffect;

/// @brief シーントランジション管理クラス
/// @details シーン遷移時のフェードイン・フェードアウトなどの演出を管理（ポストエフェクトベース）
class SceneTransition {
public:
    /// @brief トランジションタイプ
    enum class TransitionType {
        None,       // トランジションなし（即座に切り替え）
        Fade,       // フェード（デフォルト）
        Slide,      // スライド（未実装）
        Dissolve    // ディゾルブ（未実装）
    };

    /// @brief トランジションフェーズ
    enum class TransitionPhase {
        Idle,       // 待機中（トランジション無し）
        FadeOut,    // フェードアウト中
        Changing,   // シーン切り替え準備完了
        FadeIn      // フェードイン中
    };

public:
    SceneTransition() = default;
    ~SceneTransition() = default;

    /// @brief 初期化
    /// @param engine エンジンシステムへのポインタ
    void Initialize(EngineSystem* engine);

    /// @brief 更新処理
    /// @param deltaTime デルタタイム（秒）
    void Update(float deltaTime);

    /// @brief トランジション開始（SceneManagerから呼ばれる）
    /// @param type トランジションタイプ
    /// @param duration トランジションの持続時間（秒）
    void StartTransition(TransitionType type, float duration);

    /// @brief シーン切り替え準備が完了したか確認
    /// @return true: シーン切り替え可能, false: まだフェードアウト中
    bool IsReadyToChangeScene() const;

    /// @brief シーン切り替え完了通知（フェードイン開始）
    void OnSceneChanged();

    /// @brief トランジション中か確認
    /// @return true: トランジション中, false: 待機中
    bool IsTransitioning() const;

    /// @brief トランジションがシーンの更新をブロックするか
    /// @return true: ブロック中, false: 更新可能
    bool IsBlocking() const;

    /// @brief 現在のフェーズを取得
    /// @return 現在のトランジションフェーズ
    TransitionPhase GetCurrentPhase() const { return phase_; }

    /// @brief トランジションをスキップ（デバッグ用）
    void SkipTransition();

private:
    /// @brief フェードアルファ値を計算
    /// @return アルファ値（0.0 = 透明, 1.0 = 不透明）
    float CalculateFadeAlpha() const;

    /// @brief ポストエフェクトにフェード値を適用
    void ApplyFadeToPostEffect();

private:
    EngineSystem* engine_ = nullptr;
    PostEffectManager* postEffectManager_ = nullptr;
    FadeEffect* fadeEffect_ = nullptr;

    TransitionPhase phase_ = TransitionPhase::Idle;
    TransitionType type_ = TransitionType::None;

    float timer_ = 0.0f;        // 現在のタイマー
    float duration_ = 1.0f;     // トランジション時間（秒）
};
