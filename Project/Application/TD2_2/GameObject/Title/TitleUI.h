#pragma once
#include "Engine/ObjectCommon/SpriteObject.h"
#include "../../Utility/StateMachine.h"
#include "Engine/Utility/Timer/GameTimer.h"
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

	/// @brief 決定ボタンが押された時のアニメーション開始
	void OnConfirm();

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

	/// @brief 決定時のアニメーションを更新
	void UpdateConfirmAnimation();

	/// @brief 登場アニメーションを更新
	void UpdateIntroAnimation();

	/// @brief 待機アニメーションを更新
	void UpdateIdleAnimation();

	/// @brief 矢印の揺れアニメーションを更新
	void UpdateArrowAnimation();

private:

	// UI要素のポインタ（所有権はgameObjects_が持つ）
	SpriteObject* titleLogo_ = nullptr;
	SpriteObject* startButtonUI_ = nullptr;
	SpriteObject* quitButtonUI_ = nullptr;
	SpriteObject* arrowLeftUI_ = nullptr;   // 左側の矢印
	SpriteObject* arrowRightUI_ = nullptr;  // 右側の矢印

	// 選択状態
	SelectionState selectionState_ = SelectionState::Start;

	// ステートマシーン
	StateMachine stateMachine_;

	// 決定アニメーション用
	GameTimer confirmAnimationTimer_;
	static constexpr float kConfirmAnimationDuration = 0.3f;  // アニメーション時間
	static constexpr float kButtonScaleMax = 1.1f;            // ボタンの最大スケール（1.15→1.1に減少）
	static constexpr float kArrowBlinkSpeed = 15.0f;          // 矢印の点滅速度

	// 登場アニメーション用
	GameTimer introAnimationTimer_;
	static constexpr float kIntroAnimationDuration = 1.5f;     // 登場アニメーション時間
	static constexpr float kTitleBounceHeight = 100.0f;        // タイトルのバウンス高さ
	static constexpr float kButtonDelayOffset = 0.3f;          // ボタンの遅延オフセット

	// 待機アニメーション用
	GameTimer idleAnimationTimer_;
	static constexpr float kIdleFloatSpeed = 3.0f;            // フロートアニメーションの速度
	static constexpr float kIdleFloatAmount = 8.0f;           // フロートアニメーションの上下移動量
	static constexpr float kTitleScaleAmount = 0.03f;         // タイトルロゴの拡縮量

	// 矢印アニメーション用
	GameTimer arrowAnimationTimer_;
	static constexpr float kArrowSwingSpeed = 4.0f;           // 矢印の揺れ速度
	static constexpr float kArrowSwingAmount = 10.0f;         // 矢印の揺れ幅（15→10に削減）

	// 矢印の位置遷移用
	GameTimer arrowTransitionTimer_;
	static constexpr float kArrowTransitionDuration = 0.25f;  // 矢印移動のアニメーション時間
	Vector3 arrowLeftStartPos_ = {};                          // 遷移開始時の左矢印位置
	Vector3 arrowRightStartPos_ = {};                         // 遷移開始時の右矢印位置
	Vector3 arrowLeftTargetPos_ = {};                         // 遷移先の左矢印位置
	Vector3 arrowRightTargetPos_ = {};                        // 遷移先の右矢印位置

	// 初期位置の保存
	Vector3 titleLogoInitialPos_ = {};
	Vector3 startButtonInitialPos_ = {};
	Vector3 quitButtonInitialPos_ = {};
	Vector3 arrowLeftInitialPos_ = {};
	Vector3 arrowRightInitialPos_ = {};

	// 矢印の位置（UIフォントの両端に配置）
	static constexpr float kArrowOffsetX_Start_Left = -200.0f;   // スタートボタン左側の矢印
	static constexpr float kArrowOffsetX_Start_Right = 200.0f;   // スタートボタン右側の矢印
	static constexpr float kArrowOffsetX_Quit_Left = -140.0f;    // Quitボタン左側の矢印（より外側に）
	static constexpr float kArrowOffsetX_Quit_Right = 140.0f;    // Quitボタン右側の矢印（より外側に）
	
	// ボタンの位置（間隔を広げて調整）
	static constexpr float kStartButtonY = -80.0f;               // スタートボタンのY座標
	static constexpr float kQuitButtonY = -230.0f;               // QuitボタンのY座標
	static constexpr float kButtonSpacing = 120.0f;              // ボタン間の間隔（100→120に拡大）
	static constexpr float kQuitToBottomSpacing = 120.0f;        // やめると画面下端の間隔
};
