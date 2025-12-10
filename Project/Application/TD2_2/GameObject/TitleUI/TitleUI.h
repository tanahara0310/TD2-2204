#pragma once
#include "Engine/ObjectCommon/SpriteObject.h"
#include "YameruModel.h"
#include "StartModel.h"
#include "TitleModel.h"
#include "GekitotsuModel.h"
#include "PlayerPresetModel.h"
#include "../../Utility/StateMachine.h"
#include "Engine/Utility/Timer/GameTimer.h"
#include <memory>
#include <vector>
#include <functional>

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
	
	/// @brief 現在選択中のプリセットインデックスを取得
	/// @return プリセットインデックス（0:HiyokoAfro, 1:Glass, 2:Student）
	int GetSelectedPresetIndex() const { return selectedPresetIndex_; }
	
	/// @brief プリセット選択を上に移動
	void SelectPreviousPreset();
	
	/// @brief プリセット選択を下に移動
	void SelectNextPreset();

	/// @brief ステートマシーンを取得
	StateMachine& GetStateMachine() { return stateMachine_; }

	/// @brief StartModelを取得
	StartModel* GetStartModel() const { return startModel_; }
	
	/// @brief YameruModelを取得
	YameruModel* GetYameruModel() const { return yameruModel_; }
	
	/// @brief TitleModelを取得
	TitleModel* GetTitleModel() const { return titleModel_; }
	
	/// @brief GekitotsuModelを取得
	GekitotsuModel* GetGekitotsuModel() const { return gekitotsuModel_; }
	
	/// @brief イントロアニメーションを開始
	void StartIntroAnimation();
	
	/// @brief イントロアニメーションが完全に完了したか（雷演出開始タイミング）
	/// @return 完了していればtrue
	bool IsIntroAnimationCompleted() const { return introAnimationCompleted_; }
	
	/// @brief イントロアニメーションをスキップ（即座に完了）
	void SkipIntroAnimation();
	
	/// @brief 白フラッシュコールバックを設定
	/// @param callback フラッシュ発生時に呼ばれる関数
	void SetFlashCallback(std::function<void()> callback) { flashCallback_ = callback; }

private:
	
	//yameruモデルを作成
	std::unique_ptr<YameruModel> CreateYameruModel(EngineSystem* engine);
	
	//startモデルを作成
	std::unique_ptr<StartModel> CreateStartModel(EngineSystem* engine);
	
	//titleモデルを作成
	std::unique_ptr<TitleModel> CreateTitleModel(EngineSystem* engine);
	
	//gekitotsuモデルを作成
	std::unique_ptr<GekitotsuModel> CreateGekitotsuModel(EngineSystem* engine);
	
	/// @brief プレイヤープリセットモデルを作成
	std::unique_ptr<PlayerPresetModel> CreatePlayerPresetModel(EngineSystem* engine, PresetType presetType, float yPosition);

	/// @brief ステートマシーンの初期化
	void InitializeStateMachine();

	/// @brief 選択演出の更新
	void UpdateSelectionEffect();
	
	/// @brief プリセット選択演出の更新
	void UpdatePresetSelectionEffect();
	
	/// @brief イントロアニメーションシーケンスの更新
	void UpdateIntroAnimationSequence();

private:

	// UI要素のポインタ（所有権はgameObjects_が持つ）
	YameruModel* yameruModel_ = nullptr;
	StartModel* startModel_ = nullptr;
	TitleModel* titleModel_ = nullptr;
	GekitotsuModel* gekitotsuModel_ = nullptr;
	
	// プリセットモデル（3つ）
	std::vector<PlayerPresetModel*> presetModels_;
	
	// 選択中のプリセットインデックス（0:HiyokoAfro, 1:Glass, 2:Student）
	int selectedPresetIndex_ = 0;

	// 選択状態
	SelectionState selectionState_ = SelectionState::Start;

	// ステートマシーン
	StateMachine stateMachine_;
	
	// イントロアニメーション管理
	GameTimer introDelayTimer_;
	bool isIntroDelayActive_ = false;
	bool isGekitotsuAnimating_ = false;
	bool isTitleAnimating_ = false;
	bool isButtonsAnimating_ = false; // StartとYameruのアニメーション
	bool introAnimationCompleted_ = false; // 全てのイントロアニメーションが完了したか
	bool flashTriggered_ = false; // 白フラッシュが既にトリガーされたか
	static constexpr float kIntroStartDelay = 0.3f; // 開始遅延時間
	static constexpr float kFlashTriggerProgress = 0.85f; // フラッシュトリガー進行度（90%）
	
	// 白フラッシュコールバック
	std::function<void()> flashCallback_;
};
