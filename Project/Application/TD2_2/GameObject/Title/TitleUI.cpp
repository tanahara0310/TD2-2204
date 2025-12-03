#include "TitleUI.h"
#include "EngineSystem/EngineSystem.h"
#include "ObjectCommon/IDrawable.h"
#include "Input/KeyboardInput.h"

std::vector<std::unique_ptr<IDrawable>> TitleUI::Initialize(EngineSystem* engine) {
	(void)engine;

	std::vector<std::unique_ptr<IDrawable>> sprites;

	// タイトルロゴを作成
	auto titleLogo = CreateTitleLogo();
	titleLogo_ = titleLogo.get();
	sprites.push_back(std::move(titleLogo));

	// 開始ボタンUIを作成
	auto startButtonUI = CreateStartButtonUI();
	startButtonUI_ = startButtonUI.get();
	sprites.push_back(std::move(startButtonUI));

	// quitボタンUIを作成
	auto quitButtonUI = CreateQuitButtonUI();
	quitButtonUI_ = quitButtonUI.get();
	sprites.push_back(std::move(quitButtonUI));

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

void TitleUI::Update() {
	if (titleLogo_) {
		titleLogo_->Update();
	}

	// ステートマシーンの更新
	stateMachine_.Update();
}

void TitleUI::SetSelectionState(SelectionState state) {
	selectionState_ = state;

	// ステートマシーンにリクエスト
	switch (state) {
	case SelectionState::Start:
		stateMachine_.RequestState("Start", 100);
		break;
	case SelectionState::Quit:
		stateMachine_.RequestState("Quit", 100);
		break;
	}
}

void TitleUI::InitializeStateMachine() {
	// Start状態の登録
	stateMachine_.AddState("Start",
		[this]() {
			// Start状態への遷移時
			selectionState_ = SelectionState::Start;
			UpdateArrowPosition();
		},
		[this]() {
			// Start状態の更新処理
		}
	);

	// Quit状態の登録
	stateMachine_.AddState("Quit",
		[this]() {
			// Quit状態への遷移時
			selectionState_ = SelectionState::Quit;
			UpdateArrowPosition();
		},
		[this]() {
			// Quit状態の更新処理
		}
	);

	// 遷移ルールの設定
	stateMachine_.AddTransitionRule("Start", { "Quit" });
	stateMachine_.AddTransitionRule("Quit", { "Start" });

	// 初期状態をStartに設定
	stateMachine_.RequestState("Start", 100);
}

void TitleUI::UpdateArrowPosition() {
	if (!arrowUI_) {
		return;
	}

	// 選択状態に応じて矢印の位置を更新（スタートUIのX方向のサイズを考慮）
	switch (selectionState_) {
	case SelectionState::Start:
		arrowUI_->GetTransform().translate = { kArrowOffsetX_Start, kStartButtonY, 0.0f };
		break;
	case SelectionState::Quit:
		arrowUI_->GetTransform().translate = { kArrowOffsetX_Quit, kQuitButtonY, 0.0f };
		break;
	}
}

std::unique_ptr<SpriteObject> TitleUI::CreateTitleLogo() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Title/Title.png");
	// 2Dカメラは画面中央が原点(0,0)なので、中央に配置するには(0,0)を指定
	sprite->GetTransform().translate = { 10.0f, 180.0f, 0.0f };
	sprite->SetAnchor({ 0.5f, 0.5f });
	return sprite;
}

std::unique_ptr<SpriteObject> TitleUI::CreateStartButtonUI()
{
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Title/titleStart.png");
	sprite->GetTransform().translate = { 6.0f, kStartButtonY, 0.0f };
	sprite->SetAnchor({ 0.5f, 0.5f });

	return sprite;
}

std::unique_ptr<SpriteObject> TitleUI::CreateQuitButtonUI()
{
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Title/quit.png");
	sprite->GetTransform().translate = { 0.0f, kQuitButtonY, 0.0f };
	sprite->SetAnchor({ 0.5f, 0.5f });

	return sprite;
}

std::unique_ptr<SpriteObject> TitleUI::CreateArrowUI()
{
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Title/arrow.png");
	// 初期位置はStart用
	sprite->GetTransform().translate = { kArrowOffsetX_Start, kStartButtonY, 0.0f };
	sprite->SetAnchor({ 0.5f, 0.5f });

	return sprite;
}
