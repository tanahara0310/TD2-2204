#include "GameScene.h"
#include "EngineSystem/EngineSystem.h"
#include "Scene/SceneManager.h"

void GameScene::Initialize(EngineSystem* engine) {
	engine_ = engine;

#ifdef _DEBUG
	auto console = engine_->GetConsole();
	if (console) {
		console->LogInfo("GameScene: 初期化完了");
	}
#endif
}

void GameScene::Update() {
	// ゲームシーンの更新処理
}

void GameScene::Draw() {
	// ゲームシーンの描画処理
}

void GameScene::Finalize() {
#ifdef _DEBUG
	auto console = engine_->GetConsole();
	if (console) {
		console->LogInfo("GameScene: 終了処理完了");
	}
#endif

	engine_ = nullptr;
}
