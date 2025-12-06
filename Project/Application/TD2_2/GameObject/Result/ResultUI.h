#pragma once
#include "../../GameObject/Result/ColonModel.h"
#include "../../GameObject/Result/NumbersModel.h"
#include "../../GameObject/Result/PeriodModel.h"
#include "../../GameObject/Result/ReStartModel.h"
#include "../../GameObject/Result/ResultModel.h"
#include "../../GameObject/Result/ToTitleModel.h"
#include "../../Utility/StateMachine.h"
#include "Engine/ObjectCommon/SpriteObject.h"
#include <memory>
#include <vector>

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

private:
	/// @brief 選択状態に応じてモデルのスケールを更新
	void UpdateModelScale();

	/// @brief ステートマシーンの初期化
	void InitializeStateMachine();

	// リザルトモデルを作成
	std::unique_ptr<ResultModel> CreateResultModel(EngineSystem* engine);

	// 数字モデルを作成
	std::unique_ptr<NumbersModel> CreateNumberModel(EngineSystem* engine, int num);

	// 「タイトルへ」モデルを作成
	std::unique_ptr<ToTitleModel> CreateToTitleModel(EngineSystem* engine);

	// 「リスタート」モデルを作成
	std::unique_ptr<ReStartModel> CreateReStartModel(EngineSystem* engine);

	// ピリオドモデルを作成
	std::unique_ptr<PeriodModel> CreatePeriodModel(EngineSystem* engine);

	// コロンモデル作成
	std::unique_ptr<ColonModel> CreateColonModel(EngineSystem* engine);

private:
	// モデルのポインタ（所有権はgameObjects_が持つ）
	ResultModel* resultModel_;
	ToTitleModel* toTitleModel_;
	ReStartModel* reStartModel_;
	PeriodModel* periodModel_;
	ColonModel* colonModel_;
	std::vector<NumbersModel*> rankModels_;        // 順位モデル
	std::vector<NumbersModel*> currentTimeModels_; // 今回のクリアタイムモデル
	std::vector<NumbersModel*> rankTimeModels_;    // 1位から3位までのクリアタイムモデル
	std::vector<PeriodModel*> periodModels_;       // 1位から3位までのピリオドモデル
	std::vector<ColonModel*> colonModels_;         // 1位から3位までのピリオドモデル

	// 選択状態
	SelectionState selectionState_ = SelectionState::ToTitle;

	// ステートマシーン
	StateMachine stateMachine_;

	// タイマー文字列
	std::array<int, 6> timerDigits_;
};
