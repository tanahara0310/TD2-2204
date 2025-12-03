#include "ResultUI.h"

std::vector<std::unique_ptr<IDrawable>> ResultUI::Initialize([[maybe_unused]] EngineSystem* engine) {
	std::vector<std::unique_ptr<IDrawable>> sprites;

	// リザルトを作成
	auto result = CreateResult();
	result_ = result.get();
	sprites.push_back(std::move(result));

	// 「タイトルへ」UIを作成
	auto toTitleUI = CreateToTitleUI();
	toTitleUI_ = toTitleUI.get();
	sprites.push_back(std::move(toTitleUI));

	// 「リスタート」UIを作成
	auto restartUI = CreateRestartUI();
	restartUI_ = restartUI.get();
	sprites.push_back(std::move(restartUI));

	// 順位用のUIを作成
	for (int i = 1; i <= 3; i++) {
		auto ranking = CreateRankingUI(i);
		rankingUI_ = ranking.get();
		sprites.push_back(std::move(ranking));
	}

	// 矢印UIを作成
	auto arrowUI = CreateArrowUI();
	arrowUI_ = arrowUI.get();
	sprites.push_back(std::move(arrowUI));

	// ステートマシーンの初期化
	InitializeStateMachine();

	// 初期位置を設定
	UpdateArrowPosition();

	return sprites;
}

void ResultUI::Update() {
	// ステートマシーンの更新
	stateMachine_.Update();
}

void ResultUI::SetSelectionState(SelectionState state) {
	selectionState_ = state;

	// ステートマシーンにリクエスト
	switch (state) {
	case SelectionState::ToTitle:
		stateMachine_.RequestState("ToTitle", 100);
		break;
	case SelectionState::ReStart:
		stateMachine_.RequestState("ReStart", 100);
		break;
	}
}

void ResultUI::InitializeStateMachine() {
	// Start状態の登録
	stateMachine_.AddState(
	    "ToTitle",
	    [this]() {
		    // Start状態への遷移時
		    selectionState_ = SelectionState::ToTitle;
		    UpdateArrowPosition();
	    },
	    [this]() {
		    // Start状態の更新処理
	    });

	// Quit状態の登録
	stateMachine_.AddState(
	    "ReStart",
	    [this]() {
		    // Quit状態への遷移時
		    selectionState_ = SelectionState::ReStart;
		    UpdateArrowPosition();
	    },
	    [this]() {
		    // Quit状態の更新処理
	    });

	// 遷移ルールの設定
	stateMachine_.AddTransitionRule("ToTitle", {"ReStart"});
	stateMachine_.AddTransitionRule("ReStart", {"ToTitle"});

	// 初期状態をStartに設定
	stateMachine_.RequestState("ToTitle", 100);
}

void ResultUI::UpdateArrowPosition() {
	if (!arrowUI_) {
		return;
	}

	// 選択状態に応じて矢印の位置を更新（スタートUIのX方向のサイズを考慮）
	switch (selectionState_) {
	case SelectionState::ToTitle:
		arrowUI_->GetTransform().translate = {kArrowOffsetX_ToTitle, kToTitleButtonY, 0.0f};
		arrowUI_->SetTexture("Resources/GameResources/Title/arrow.png");
		break;
	case SelectionState::ReStart:
		arrowUI_->GetTransform().translate = {kArrowOffsetX_ReStart, kReStartButtonY, 0.0f};
		arrowUI_->SetTexture("Resources/GameResources/Result/arrowLeft.png");
		break;
	}
}

std::unique_ptr<SpriteObject> ResultUI::CreateResult() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Result/GameClear.png");
	sprite->GetTransform().translate = {0.0f, 290.0f, 0.0f};
	sprite->SetAnchor({0.5f, 0.5f});
	return sprite;
}

std::unique_ptr<SpriteObject> ResultUI::CreateToTitleUI() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Result/ToTitle.png");
	sprite->GetTransform().translate = {490.0f, -310.0f, 0.0f};
	sprite->SetAnchor({0.5f, 0.5f});

	return sprite;
}

std::unique_ptr<SpriteObject> ResultUI::CreateRestartUI() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Result/Restart.png");
	sprite->GetTransform().translate = {-500.0f, -300.0f, 0.0f};
	sprite->SetAnchor({0.5f, 0.5f});

	return sprite;
}

std::unique_ptr<SpriteObject> ResultUI::CreateRankingUI(int num) {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Result/Numbers/" + std::to_string(num) + ".png");
	sprite->GetTransform().translate = {-500.0f, 280.0f - (num * 140.0f), 0.0f};
	sprite->SetAnchor({0.5f, 0.5f});

	return sprite;
}

std::unique_ptr<SpriteObject> ResultUI::CreateArrowUI() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Result/arrowLeft.png");
	sprite->GetTransform().translate = { kArrowOffsetX_ReStart, kToTitleButtonY, 0.0f };
	sprite->SetAnchor({ 0.5f, 0.5f });

	return sprite;
}
