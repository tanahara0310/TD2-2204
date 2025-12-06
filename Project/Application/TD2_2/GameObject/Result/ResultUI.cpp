#include "ResultUI.h"

std::vector<std::unique_ptr<IDrawable>> ResultUI::Initialize([[maybe_unused]] EngineSystem* engine) {
	std::vector<std::unique_ptr<IDrawable>> sprites;

	// リザルトモデル作成
	{
		auto resultModel = CreateResultModel(engine);
		resultModel_ = resultModel.get();
		sprites.push_back(std::move(resultModel));
	}

	// 「タイトルへ」モデル作成
	{
		auto toTitleModel = CreateToTitleModel(engine);
		toTitleModel_ = toTitleModel.get();
		sprites.push_back(std::move(toTitleModel));
	}

	// 「リスタート」モデル作成
	{
		auto reStartModel = CreateReStartModel(engine);
		reStartModel_ = reStartModel.get();
		sprites.push_back(std::move(reStartModel));
	}

	// ピリオドモデルを作成
	{
		auto periodModel = CreatePeriodModel(engine);
		periodModel->GetTransform().translate = {6.5f, 5.0f, 0.0f};
		periodModel->GetTransform().scale = {8.0f, 8.0f, 8.0f};
		periodModel_ = periodModel.get();
		sprites.push_back(std::move(periodModel));
	}

	// コロンモデルを作成
	{
		auto colonModel = CreateColonModel(engine);
		colonModel->GetTransform().translate = {-2.0f, 1.6f, -47.0f};
		colonModel->GetTransform().scale = {2.0f, 2.0f, 2.0f};
		colonModel_ = colonModel.get();
		sprites.push_back(std::move(colonModel));
	}

	// 順位用のモデルを作成
	for (int i = 1; i <= 3; i++) {
		auto ranking = CreateNumberModel(engine, i);
		ranking->GetTransform().scale = {1.0f, 1.0f, 1.0f};
		ranking->GetTransform().translate = {-5.0f, i * -1.0f + 1.2f, -47.0f};
		rankModels_.push_back(ranking.get());
		sprites.push_back(std::move(ranking));
	}

	// タイマー用のモデルを作成
	for (int i = 0; i < 6; i++) {
		auto timer = CreateNumberModel(engine, i);

		// 基本の位置
		float x = i * 1.5f - 4.7f;

		// iが2の倍数のときに余分なオフセットを加える
		x += (i / 2) * 1.0f;

		timer->GetTransform().translate = {x, 1.5f, -47.0f};
		timer->GetTransform().scale = {2.5f, 2.5f, 2.5f};
		currentTimeModels_.push_back(timer.get());
		sprites.push_back(std::move(timer));
	}

	// タイマー用のモデル(ランキング)を作成
	for (int i = 1; i <= 3; i++) {
		for (int j = 0; j < 6; j++) {
			auto timer = CreateNumberModel(engine, j);

			// 基本の位置
			float x = j * 0.5f - 2.2f;
			float y = i * -1.0f + 1.2f;

			// iが2の倍数のときに余分なオフセットを加える
			x += (j / 2) * 1.0f;

			timer->GetTransform().translate = {x, y, -47.0f};
			timer->GetTransform().scale = {1.0f, 1.0f, 1.0f};
			rankTimeModels_.push_back(timer.get());
			sprites.push_back(std::move(timer));
		}
	}

	// ピリオドモデル(ランキング)を作成
	for (int i = 0; i < 3; i++) {
		auto periodModel = CreatePeriodModel(engine);
		periodModel->GetTransform().translate = {1.1f, i * 1.0f - 1.7f, -47.0f};
		periodModel->GetTransform().scale = {2.0f, 2.0f, 2.0f};
		periodModels_.push_back(periodModel.get());
		sprites.push_back(std::move(periodModel));
	}

	// コロンモデル(ランキング)を作成
	for (int i = 0; i < 3; i++) {
		auto colonModel = CreateColonModel(engine);
		colonModel->GetTransform().translate = {-1.0f, i * 1.0f - 1.7f, -47.0f};
		colonModel->GetTransform().scale = {0.7f, 0.7f, 0.7f};
		colonModels_.push_back(colonModel.get());
		sprites.push_back(std::move(colonModel));
	}

	// ステートマシーンの初期化
	InitializeStateMachine();

	// 初期スケールを設定
	UpdateModelScale();

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
		// モデルの切り替え
		currentTimeModels_[i]->ChangeModelResource("Resources/Models/" + std::to_string(timerDigits_[i]) + "/" + std::to_string(timerDigits_[i]) + ".obj");
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
		    UpdateModelScale();
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
		    UpdateModelScale();
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

std::unique_ptr<NumbersModel> ResultUI::CreateNumberModel(EngineSystem* engine, int num) {
	auto modelManager = engine->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();

	auto numModelResource = modelManager->CreateStaticModel("Resources/Models/" + std::to_string(num) + "/" + std::to_string(num) + ".obj");
	auto numTexture = textureManager.Load("Resources/SampleResources/white1x1.png");

	auto numModel = std::make_unique<NumbersModel>();
	numModel->Initialize(std::move(numModelResource), numTexture);

	return numModel;
}

std::unique_ptr<ResultModel> ResultUI::CreateResultModel(EngineSystem* engine) {
	auto modelManager = engine->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();

	auto resultModelResource = modelManager->CreateStaticModel("Resources/Models/GameClear/GameClear.obj");
	auto resultTexture = textureManager.Load("Resources/SampleResources/white1x1.png");

	auto resultModel = std::make_unique<ResultModel>();
	resultModel->Initialize(std::move(resultModelResource), resultTexture);

	return resultModel;
}

std::unique_ptr<ToTitleModel> ResultUI::CreateToTitleModel(EngineSystem* engine) {
	auto modelManager = engine->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();

	auto toTitleModelResource = modelManager->CreateStaticModel("Resources/Models/ToTitle/ToTitle.obj");
	auto toTitleTexture = textureManager.Load("Resources/SampleResources/white1x1.png");

	auto toTitleModel = std::make_unique<ToTitleModel>();
	toTitleModel->Initialize(std::move(toTitleModelResource), toTitleTexture);

	return toTitleModel;
}

std::unique_ptr<ReStartModel> ResultUI::CreateReStartModel(EngineSystem* engine) {
	auto modelManager = engine->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();

	auto reStartModelResource = modelManager->CreateStaticModel("Resources/Models/ReStart/ReStart.obj");
	auto reStartTexture = textureManager.Load("Resources/SampleResources/white1x1.png");

	auto reStartModel = std::make_unique<ReStartModel>();
	reStartModel->Initialize(std::move(reStartModelResource), reStartTexture);

	return reStartModel;
}

std::unique_ptr<PeriodModel> ResultUI::CreatePeriodModel(EngineSystem* engine) {
	auto modelManager = engine->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();

	auto periodModelResource = modelManager->CreateStaticModel("Resources/Models/Voxel/Voxel.obj");
	auto periodTexture = textureManager.Load("Resources/SampleResources/white1x1.png");

	auto periodModel = std::make_unique<PeriodModel>();
	periodModel->Initialize(std::move(periodModelResource), periodTexture);

	return periodModel;
}

std::unique_ptr<ColonModel> ResultUI::CreateColonModel(EngineSystem* engine) { 
	auto modelManager = engine->GetComponent<ModelManager>();
	auto& textureManager = TextureManager::GetInstance();

	auto colonModelResource = modelManager->CreateStaticModel("Resources/Models/Colon/Colon.obj");
	auto colonTexture = textureManager.Load("Resources/SampleResources/white1x1.png");

	auto colonModel = std::make_unique<ColonModel>();
	colonModel->Initialize(std::move(colonModelResource), colonTexture);

	return colonModel;
}

void ResultUI::UpdateModelScale() {
	// 選択状態に応じてモデルのスケールを更新
	switch (selectionState_) {
	case SelectionState::ToTitle:
		toTitleModel_->GetTransform().scale = {1.2f, 1.2f, 1.2f};
		reStartModel_->GetTransform().scale = {1.0f, 1.0f, 1.0f};
		break;
	case SelectionState::ReStart:
		toTitleModel_->GetTransform().scale = {1.0f, 1.0f, 1.0f};
		reStartModel_->GetTransform().scale = {1.2f, 1.2f, 1.2f};
		break;
	}
}
