#include "TitleScene.h"
#include "EngineSystem/EngineSystem.h"
#include "Scene/SceneManager.h"

void TitleScene::Initialize(EngineSystem* engine) {
	engine_ = engine;

#ifdef _DEBUG
	auto console = engine_->GetConsole();
	if (console) {
		console->LogInfo("TitleScene: 初期化完了");
	}
#endif
}

void TitleScene::Update() {
	// タイトルシーンの更新処理
}

void TitleScene::Draw() {
	// タイトルシーンの描画処理
}

void TitleScene::Finalize() {
#ifdef _DEBUG
	auto console = engine_->GetConsole();
	if (console) {
		console->LogInfo("TitleScene: 終了処理完了");
	}
#endif

	engine_ = nullptr;
}
