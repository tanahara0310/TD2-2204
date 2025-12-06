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
	
	// プレイヤープリセットモデルを作成（3つ縦に並べる）
	// Y座標の間隔を狭める
	const float kPresetSpacing = 2.0f; // 4.0から2.5に変更
	const float kPresetStartY = -3.0f; // 中央のY座標を下げる
	
	// 1. HiyokoAfro（上）
	auto hiyokoAfroModel = CreatePlayerPresetModel(engine, PresetType::HiyokoAfro, kPresetStartY + kPresetSpacing);
	presetModels_.push_back(hiyokoAfroModel.get());
	sprites.push_back(std::move(hiyokoAfroModel));
	
	// 2. Glass（中央）
	auto glassModel = CreatePlayerPresetModel(engine, PresetType::Glass, kPresetStartY);
	presetModels_.push_back(glassModel.get());
	sprites.push_back(std::move(glassModel));
	
	// 3. Student（下）
	auto studentModel = CreatePlayerPresetModel(engine, PresetType::Student, kPresetStartY - kPresetSpacing);
	presetModels_.push_back(studentModel.get());
	sprites.push_back(std::move(studentModel));

	// ステートマシーンの初期化
	InitializeStateMachine();
	
	// 初期状態の選択状態を設定（Startが選択された状態）
	if (startModel_) {
		startModel_->SetSelected(true);
	}
	if (yameruModel_) {
		yameruModel_->SetSelected(false);
	}
	
	// 初期プリセット選択（HiyokoAfro）
	selectedPresetIndex_ = 0;
	UpdatePresetSelectionEffect();

	return sprites;
}

void TitleUI::Update() {
	// ステートマシーンの更新
	stateMachine_.Update();
	
	// 選択演出の更新
	UpdateSelectionEffect();
	
	// プリセット選択演出の更新
	UpdatePresetSelectionEffect();
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

void TitleUI::SelectPreviousPreset() {
	selectedPresetIndex_--;
	if (selectedPresetIndex_ < 0) {
		selectedPresetIndex_ = static_cast<int>(presetModels_.size()) - 1;
	}
	UpdatePresetSelectionEffect();
}

void TitleUI::SelectNextPreset() {
	selectedPresetIndex_++;
	if (selectedPresetIndex_ >= static_cast<int>(presetModels_.size())) {
		selectedPresetIndex_ = 0;
	}
	UpdatePresetSelectionEffect();
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

void TitleUI::UpdatePresetSelectionEffect() {
	// 全てのプリセットモデルの選択状態を更新
	for (int i = 0; i < static_cast<int>(presetModels_.size()); ++i) {
		if (presetModels_[i]) {
			presetModels_[i]->SetSelected(i == selectedPresetIndex_);
		}
	}
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

std::unique_ptr<PlayerPresetModel> TitleUI::CreatePlayerPresetModel(EngineSystem* engine, PresetType presetType, float yPosition)
{
	auto modelManager = engine->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();
	
	std::string modelPath;
	std::string texturePath;
	
	// プリセットタイプに応じてパスを設定
	switch (presetType) {
	case PresetType::HiyokoAfro:
		modelPath = "Resources/Models/HiyokoAfroPropeller/HiyokoAfroPropeller.obj";
		texturePath = "Resources/Textures/HiyokoAfroPropeller.png";
		break;
	case PresetType::Glass:
		modelPath = "Resources/Models/HiyokoGlassPropeller/HiyokoGlassPropeller.obj";
		texturePath = "Resources/Textures/HiyokoGlassPropeller.png";
		break;
	case PresetType::Student:
		modelPath = "Resources/Models/HiyokoStudentPropeller/HiyokoStudentPropeller.obj";
		texturePath = "Resources/Textures/HiyokoStudentPropeller.png";
		break;
	}
	
	auto presetModelResource = modelManager->CreateStaticModel(modelPath);
	auto presetTexture = textureManager.Load(texturePath);
	
	auto presetModel = std::make_unique<PlayerPresetModel>();
	presetModel->Initialize(std::move(presetModelResource), presetTexture, presetType, yPosition);
	
	return presetModel;
}
