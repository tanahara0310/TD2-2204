#include "TitleUI.h"
#include "EngineSystem/EngineSystem.h"
#include "ObjectCommon/IDrawable.h"
#include "Graphics/Model/ModelManager.h"
#include "Graphics/TextureManager.h"
#include "Graphics/Common/DirectXCommon.h"

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
	
	// yameruモデルを作成
	auto yameruModel = CreateYameruModel(engine);
	yameruModel_ = yameruModel.get();
	sprites.push_back(std::move(yameruModel));
	
	// startモデルを作成
	auto startModel = CreateStartModel(engine);
	startModel_ = startModel.get();
	sprites.push_back(std::move(startModel));

	// ステートマシーンの初期化
	InitializeStateMachine();

	return sprites;
}

void TitleUI::Update() {
	// ステートマシーンの更新
	stateMachine_.Update();
}

void TitleUI::SetSelectionState(SelectionState state) {
	// 前の状態と同じ場合は何もしない
	if (selectionState_ == state) {
		return;
	}
	
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
			selectionState_ = SelectionState::Start;
		},
		[this]() {
		}
	);

	// Quit状態の登録
	stateMachine_.AddState("Quit",
		[this]() {
			selectionState_ = SelectionState::Quit;
		},
		[this]() {
		}
	);

	// 遷移ルールの設定
	stateMachine_.AddTransitionRule("Start", { "Quit" });
	stateMachine_.AddTransitionRule("Quit", { "Start" });

	// 初期状態をStartに設定
	stateMachine_.RequestState("Start", 100);
}

std::unique_ptr<SpriteObject> TitleUI::CreateTitleLogo() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Title/Title.png");
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

std::unique_ptr<YameruModel> TitleUI::CreateYameruModel(EngineSystem* engine)
{
	auto modelManager = engine->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();
	
	auto yameruModelResource = modelManager->CreateStaticModel("Resources/GameResources/Title/Yameru/yameru.obj");
	auto yameruTexture = textureManager.Load("Resources/SampleResources/white1x1.png");
	
	auto yameru = std::make_unique<YameruModel>();
	yameru->Initialize(std::move(yameruModelResource), yameruTexture);

	
	return yameru;
}

std::unique_ptr<StartModel> TitleUI::CreateStartModel(EngineSystem* engine)
{
	auto modelManager = engine->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();
	
	auto startModelResource = modelManager->CreateStaticModel("Resources/GameResources/Title/Start/start.obj");
	auto startTexture = textureManager.Load("Resources/SampleResources/white1x1.png");
	
	auto startModel = std::make_unique<StartModel>();
	startModel->Initialize(std::move(startModelResource), startTexture);
	
	
	return startModel;
}
