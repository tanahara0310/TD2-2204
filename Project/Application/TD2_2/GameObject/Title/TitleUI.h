#pragma once
#include "Engine/ObjectCommon/SpriteObject.h"
#include "../../Utility/StateMachine.h"
#include <memory>
#include <vector>

class EngineSystem;
class IDrawable;

/// @brief タイトル画面のUI管理クラス
class TitleUI {
public:
    enum class SelectionState {
        Start,  // スタートボタンを選択
        Quit    // Quitボタンを選択
    };

    TitleUI() = default;
    ~TitleUI() = default;

    /// @brief 初期化（スプライトを作成してvectorで返す）
    /// @param engine エンジンシステム
    /// @return 作成したスプライトのunique_ptrのvector
    std::vector<std::unique_ptr<IDrawable>> Initialize(EngineSystem* engine);

    /// @brief 更新
    void Update();

    /// @brief 現在の選択状態を取得
    /// @return 選択状態
    SelectionState GetSelectionState() const { return selectionState_; }

    /// @brief 選択状態を設定（ステートマシーンを使用）
    /// @param state 新しい選択状態
    void SetSelectionState(SelectionState state);

    /// @brief ステートマシーンを取得
    StateMachine& GetStateMachine() { return stateMachine_; }

private:
    //タイトルロゴを作成
    std::unique_ptr<SpriteObject> CreateTitleLogo();

    //開始ボタンui
	std::unique_ptr<SpriteObject> CreateStartButtonUI();

	//quitボタンui
	std::unique_ptr<SpriteObject> CreateQuitButtonUI();

    //矢印UI
	std::unique_ptr<SpriteObject> CreateArrowUI();

    /// @brief 選択状態に応じて矢印の位置を更新
    void UpdateArrowPosition();

    /// @brief ステートマシーンの初期化
    void InitializeStateMachine();

    // UI要素のポインタ（所有権はgameObjects_が持つ）
    SpriteObject* titleLogo_ = nullptr;
	SpriteObject* startButtonUI_ = nullptr;
	SpriteObject* quitButtonUI_ = nullptr;
	SpriteObject* arrowUI_ = nullptr;

    // 選択状態
    SelectionState selectionState_ = SelectionState::Start;

    // ステートマシーン
    StateMachine stateMachine_;

    // 矢印の位置（スタートUIのサイズを考慮）
    static constexpr float kArrowOffsetX_Start = -270.0f;  // スタートボタン用（X方向に大きいため左寄り）
    static constexpr float kArrowOffsetX_Quit = -150.0f;   // Quitボタン用（標準サイズ）
    static constexpr float kStartButtonY = -160.0f;         // スタートボタンのY座標
    static constexpr float kQuitButtonY = -260.0f;          // QuitボタンのY座標
};
