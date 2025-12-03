#pragma once
#include <memory>
#include <vector>
#include "../../Utility/StateMachine.h"
#include "Engine/ObjectCommon/SpriteObject.h"

class EngineSystem;
class IDrawable;

class ResultUI {
public:
	enum class SelectionState {
		ToTitle, // ToTitleボタンを選択
		ReStart, // リスタートボタンを選択
	};

	ResultUI() = default;
	~ResultUI() = default;

	/// @brief 初期化（スプライトを作成してvectorで返す）
	/// @param engine エンジンシステム
	/// @return 作成したスプライトのunique_ptrのvector
	std::vector<std::unique_ptr<IDrawable>> Initialize(EngineSystem* engine);

	/// @brief 更新
	void Update();

	/// @brief タイマーの文字列を受け取る
	void SetTimerString(std::array<int, 6> timerString);

	/// @brief 現在の選択状態を取得
	/// @return 選択状態
	SelectionState GetSelectionState() const { return selectionState_; }

	/// @brief 選択状態を設定（ステートマシーンを使用）
	/// @param state 新しい選択状態
	void SetSelectionState(SelectionState state);

	/// @brief ステートマシーンを取得
	StateMachine& GetStateMachine() { return stateMachine_; }

	/// @brief リザルトテクスチャのセッター
	/// @param filePath リザルトテクスチャのファイルパス
	void SetResult(std::string filePath) { result_->SetTexture(filePath); }

private:
	// リザルトを作成
	std::unique_ptr<SpriteObject> CreateResult();

	// 「タイトルへ」ui
	std::unique_ptr<SpriteObject> CreateToTitleUI();

	// 「リスタート」ui
	std::unique_ptr<SpriteObject> CreateRestartUI();

	// 順位用ui
	std::unique_ptr<SpriteObject> CreateRankingUI(int num);

	// 矢印UI
	std::unique_ptr<SpriteObject> CreateArrowUI();

	// タイマーui
	std::unique_ptr<SpriteObject> CreateTimerUI();

	// コロンui
	std::unique_ptr<SpriteObject> CreateColonUI();

	/// @brief 選択状態に応じて矢印の位置を更新
	void UpdateArrowPosition();

	/// @brief ステートマシーンの初期化
	void InitializeStateMachine();

private:
	// UI要素のポインタ（所有権はgameObjects_が持つ）
	SpriteObject* result_ = nullptr;
	SpriteObject* toTitleUI_ = nullptr;
	SpriteObject* restartUI_ = nullptr;
	SpriteObject* rankingUI_ = nullptr;
	SpriteObject* arrowUI_ = nullptr;
	std::vector<SpriteObject*> timerUI_;
	std::vector<SpriteObject*> timerUIRank1_;
	std::vector<SpriteObject*> timerUIRank2_;
	std::vector<SpriteObject*> timerUIRank3_;
	SpriteObject* colonUI_ = nullptr;
	SpriteObject* colonUIRank1_ = nullptr;
	SpriteObject* colonUIRank2_ = nullptr;
	SpriteObject* colonUIRank3_ = nullptr;

	// 選択状態
	SelectionState selectionState_ = SelectionState::ToTitle;

	// ステートマシーン
	StateMachine stateMachine_;

	// タイマー文字列
	std::array<int, 6> timerDigits_;

	// 矢印の位置（スタートUIのサイズを考慮）
	static constexpr float kArrowOffsetX_ToTitle = 320.0f;   // リスタートボタン用（X方向に大きいため左寄り）
	static constexpr float kArrowOffsetX_ReStart = -340.0f;  // 「タイトルへ」ボタン用（標準サイズ）
	static constexpr float kToTitleButtonY = -310.0f;        // 「タイトルへ」ボタンのY座標
	static constexpr float kReStartButtonY = -310.0f;        // リスタートボタンのY座標
};
