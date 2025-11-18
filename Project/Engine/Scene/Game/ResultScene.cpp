#include "ResultScene.h"
#include "EngineSystem/EngineSystem.h"
#include "Scene/SceneManager.h"

void ResultScene::Initialize(EngineSystem* engine) {
	engine_ = engine;

#ifdef _DEBUG
	auto console = engine_->GetConsole();
	if (console) {
		console->LogInfo("ResultScene: 初期化完了");
	}
#endif
}

void ResultScene::Update() {
	// リザルトシーンの更新処理
}

void ResultScene::Draw() {
	// リザルトシーンの描画処理
}

void ResultScene::Finalize() {
#ifdef _DEBUG
	auto console = engine_->GetConsole();
	if (console) {
		console->LogInfo("ResultScene: 終了処理完了");
	}
#endif

	engine_ = nullptr;
}
