#include "TitleUI.h"
#include "EngineSystem/EngineSystem.h"
#include "ObjectCommon/IDrawable.h"
#include "Graphics/Model/ModelManager.h"
#include "Graphics/TextureManager.h"
#include "Graphics/Common/DirectXCommon.h"
#include "../../Utility/GameUtils.h"
#include <cmath>

std::vector<std::unique_ptr<IDrawable>> TitleUI::Initialize(EngineSystem* engine) {
	(void)engine;

	std::vector<std::unique_ptr<IDrawable>> sprites;

	// タイトルロゴを作成
	auto titleLogo = CreateTitleLogo();
	titleLogo_ = titleLogo.get();
	sprites.push_back(std::move(titleLogo));
	
	// yameruモデルを作成
	auto yameruModel = CreateYameruModel(engine);
	yameruModel_ = yameruModel.get();
	sprites.push_back(std::move(yameruModel));
	
	// startモデルを作成
	auto startModel = CreateStartModel(engine);
	startModel_ = startModel.get();
	sprites.push_back(std::move(startModel));
	
	// titleモデルを作成
	auto titleModel = CreateTitleModel(engine);
	titleModel_ = titleModel.get();
	sprites.push_back(std::move(titleModel));

	// ステートマシーンの初期化
	InitializeStateMachine();
	
	// 初期状態の選択状態を設定（Startが選択された状態）
	if (startModel_) {
		startModel_->SetSelected(true);
	}
	if (yameruModel_) {
		yameruModel_->SetSelected(false);
	}

	return sprites;
}

void TitleUI::Update() {
	// ステートマシーンの更新
	stateMachine_.Update();
	
	// 選択演出の更新
	UpdateSelectionEffect();
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

void TitleUI::UpdateSelectionEffect() {
	// 選択状態に応じてモデルの選択フラグを設定
	bool isStartSelected = (selectionState_ == SelectionState::Start);
	bool isQuitSelected = (selectionState_ == SelectionState::Quit);
	
	// StartModelの選択状態を設定
	if (startModel_) {
		startModel_->SetSelected(isStartSelected);
	}
	
	// YameruModelの選択状態を設定
	if (yameruModel_) {
		yameruModel_->SetSelected(isQuitSelected);
	}
}

std::unique_ptr<SpriteObject> TitleUI::CreateTitleLogo() {
	auto sprite = std::make_unique<SpriteObject>();
	sprite->Initialize("Resources/GameResources/Title/Title.png");
	sprite->GetTransform().translate = { 10.0f, 180.0f, 0.0f };
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

std::unique_ptr<TitleModel> TitleUI::CreateTitleModel(EngineSystem* engine)
{
	auto modelManager = engine->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();
	
	auto titleModelResource = modelManager->CreateStaticModel("Resources/GameResources/Title/Title/title.obj");
	auto titleTexture = textureManager.Load("Resources/SampleResources/white1x1.png");
	
	auto titleModel = std::make_unique<TitleModel>();
	titleModel->Initialize(std::move(titleModelResource), titleTexture);
	
	return titleModel;
}
