#pragma once
#include "Engine/ObjectCommon/SpriteObject.h"
#include "YameruModel.h"
#include "StartModel.h"
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
	
	//yameruモデルを作成
	std::unique_ptr<YameruModel> CreateYameruModel(EngineSystem* engine);
	
	//startモデルを作成
	std::unique_ptr<StartModel> CreateStartModel(EngineSystem* engine);

	/// @brief ステートマシーンの初期化
	void InitializeStateMachine();

private:

	// UI要素のポインタ（所有権はgameObjects_が持つ）
	SpriteObject* titleLogo_ = nullptr;
	SpriteObject* startButtonUI_ = nullptr;
	SpriteObject* quitButtonUI_ = nullptr;
	YameruModel* yameruModel_ = nullptr;
	StartModel* startModel_ = nullptr; // 新たに追加されたメンバー変数

	// 選択状態
	SelectionState selectionState_ = SelectionState::Start;

	// ステートマシーン
	StateMachine stateMachine_;
	
	// ボタンの位置
	static constexpr float kStartButtonY = -80.0f;
	static constexpr float kQuitButtonY = -230.0f;
	
	// やめるボタンui
	SpriteObject* yameruButtonUI_ = nullptr;
};
