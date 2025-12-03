#include "ResultUI.h"

std::vector<std::unique_ptr<IDrawable>> ResultUI::Initialize([[maybe_unused]] EngineSystem* engine) {
	std::vector<std::unique_ptr<IDrawable>> sprites;

	// リザルトを作成
	{
		auto result = CreateResult();
		result_ = result.get();
		sprites.push_back(std::move(result));
	}

	// 「タイトルへ」UIを作成
	{
		auto toTitleUI = CreateToTitleUI();
		toTitleUI_ = toTitleUI.get();
		sprites.push_back(std::move(toTitleUI));
	}

	// 「リスタート」UIを作成
	{
		auto restartUI = CreateRestartUI();
		restartUI_ = restartUI.get();
		sprites.push_back(std::move(restartUI));
	}

	// 順位用のUIを作成
	for (int i = 1; i <= 3; i++) {
		auto ranking = CreateRankingUI(i);
		ranking->GetTransform().scale = {0.5f, 0.5f, 0.5f};
		rankingUI_ = ranking.get();
		sprites.push_back(std::move(ranking));
	}

	// 矢印UIを作成
	{
		auto arrowUI = CreateArrowUI();
		arrowUI_ = arrowUI.get();
		sprites.push_back(std::move(arrowUI));
	}

	// タイマー用のUIを作成
	for (int i = 0; i < 6; i++) {
		auto timer = CreateTimerUI();

		// 基本の位置
		float x = i * 64.0f - 400.0f;

		// iが2の倍数のときに余分なオフセットを加える
		x += (i / 2) * 256.0f;

		timer->GetTransform().translate = {x, 140.0f, 0.0f};
		timerUI_.push_back(timer.get());
		sprites.push_back(std::move(timer));
	}

	// タイマー用(1位)のUIを作成
	for (int i = 0; i < 6; i++) {
		auto timer = CreateTimerUI();

		// 基本の位置
		float x = i * 32.0f - 200.0f;

		// iが2の倍数のときに余分なオフセットを加える
		x += (i / 2) * 128.0f;

		timer->GetTransform().translate = {x, -10.0f, 0.0f};
		timer->GetTransform().scale = {0.5f, 0.5f, 0.5f};
		timerUIRank1_.push_back(timer.get());
		sprites.push_back(std::move(timer));
	}

	// タイマー用(2位)のUIを作成
	for (int i = 0; i < 6; i++) {
		auto timer = CreateTimerUI();

		// 基本の位置
		float x = i * 32.0f - 200.0f;

		// iが2の倍数のときに余分なオフセットを加える
		x += (i / 2) * 128.0f;

		timer->GetTransform().translate = {x, -100.0f, 0.0f};
		timer->GetTransform().scale = {0.5f, 0.5f, 0.5f};
		timerUIRank1_.push_back(timer.get());
		sprites.push_back(std::move(timer));
	}

	// タイマー用(3位)のUIを作成
	for (int i = 0; i < 6; i++) {
		auto timer = CreateTimerUI();

		// 基本の位置
		float x = i * 32.0f - 200.0f;

		// iが2の倍数のときに余分なオフセットを加える
		x += (i / 2) * 128.0f;

		timer->GetTransform().translate = {x, -190.0f, 0.0f};
		timer->GetTransform().scale = {0.5f, 0.5f, 0.5f};
		timerUIRank1_.push_back(timer.get());
		sprites.push_back(std::move(timer));
	}

	// コロンUIを作成
	for (int i = 0; i < 2; i++) {
		auto colonUI = CreateColonUI();
		colonUI->GetTransform().translate = {i * 390.0f - 180.0f, 130.0f, 0.0f};
		colonUI_ = colonUI.get();
		sprites.push_back(std::move(colonUI));
	}

	// コロンUI(1位)を作成
	for (int i = 0; i < 2; i++) {
		auto colonUI = CreateColonUI();
		colonUI->GetTransform().translate = {i * 195.0f - 90.0f, -10.0f, 0.0f};
		colonUI->GetTransform().scale = {0.5f, 0.5f, 0.5f};
		colonUIRank1_ = colonUI.get();
		sprites.push_back(std::move(colonUI));
	}

	// コロンUI(2位)を作成
	for (int i = 0; i < 2; i++) {
		auto colonUI = CreateColonUI();
		colonUI->GetTransform().translate = {i * 195.0f - 90.0f, -100.0f, 0.0f};
		colonUI->GetTransform().scale = {0.5f, 0.5f, 0.5f};
		colonUIRank2_ = colonUI.get();
		sprites.push_back(std::move(colonUI));
	}

	// コロンUI(3位)を作成
	for (int i = 0; i < 2; i++) {
		auto colonUI = CreateColonUI();
		colonUI->GetTransform().translate = {i * 195.0f - 90.0f, -190.0f, 0.0f};
		colonUI->GetTransform().scale = {0.5f, 0.5f, 0.5f};
		colonUIRank3_ = colonUI.get();
		sprites.push_back(std::move(colonUI));
	}

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

void ResultUI::SetTimerString(std::array<int, 6> timerString) {
	timerDigits_ = timerString;

	// タイマー
	for (int i = 0; i < timerDigits_.size(); ++i) {
		timerUI_[i]->SetTexture("Resources/GameResources/Result/Numbers/" + std::to_string(timerDigits_[i]) + ".png");
	}
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
	sprite->GetTransform().translate = {-300.0f, 80.0f - (num * 90.0f), 0.0f};
	sprite->SetAnchor({0.5f, 0.5f});

	return sprite;
}

std::unique_ptr<SpriteObject> ResultUI::CreateArrowUI() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Result/arrowLeft.png");
	sprite->GetTransform().translate = {kArrowOffsetX_ReStart, kToTitleButtonY, 0.0f};
	sprite->SetAnchor({0.5f, 0.5f});

	return sprite;
}

std::unique_ptr<SpriteObject> ResultUI::CreateTimerUI() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Result/numbers/0.png");
	sprite->GetTransform().translate = {0.0f, 0.0f, 0.0f};
	sprite->SetAnchor({0.5f, 0.5f});

	return sprite;
}

std::unique_ptr<SpriteObject> ResultUI::CreateColonUI() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Result/Colon.png");
	sprite->SetAnchor({0.5f, 0.5f});

	return sprite;
}
