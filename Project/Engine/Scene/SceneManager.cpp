#include "SceneManager.h"
#include <EngineSystem.h>
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Light/LightManager.h"

void SceneManager::Initialize(EngineSystem* engine) {
	engine_ = engine;
}

void SceneManager::ChangeScene(std::string name) {
	// Update/Draw実行中のクラッシュを防ぐため次フレームで切り替え
	nextSceneName_ = std::move(name);
	isSceneChangeRequested_ = true;
}

void SceneManager::Update() {
	// 遅延シーン切り替えを最初に実行
	if (isSceneChangeRequested_) {
		DoChangeScene(nextSceneName_);
		isSceneChangeRequested_ = false;
	}

	if (currentScene_) {
		currentScene_->Update();
	}
}

void SceneManager::Draw() {
	if (currentScene_) {
		currentScene_->Draw();
	}
}

void SceneManager::Finalize() {
	// GPUの処理完了を待機してからシーンを解放
	auto dxCommon = engine_->GetComponent<DirectXCommon>();
	if (dxCommon) {
		dxCommon->WaitForPreviousFrame();
	}
	
	currentScene_.reset();
	currentSceneName_ = "None";
	sceneFactories_.clear();
}

bool SceneManager::HasScene(const std::string& name) const {
	return sceneFactories_.find(name) != sceneFactories_.end();
}

std::string SceneManager::GetCurrentSceneName() const {
	return currentSceneName_;
}

std::vector<std::string> SceneManager::GetAllSceneNames() const {
	std::vector<std::string> sceneNames;
	sceneNames.reserve(sceneFactories_.size());
	for (const auto& pair : sceneFactories_) {
		sceneNames.push_back(pair.first);
	}
	return sceneNames;
}

void SceneManager::DoChangeScene(const std::string& name) {
	auto it = sceneFactories_.find(name);
	if (it == sceneFactories_.end()) {
		return;
	}

	// GPUの処理完了を待機してから古いシーンを解放
	auto dxCommon = engine_->GetComponent<DirectXCommon>();
	if (dxCommon) {
		dxCommon->WaitForPreviousFrame();
	}

	// 古いシーンを解放
	if (currentScene_) {
		currentScene_->Finalize();
	}
	currentScene_.reset();
	
	// シーン切り替え時にライトをクリア
	auto lightManager = engine_->GetComponent<LightManager>();
	if (lightManager) {
		lightManager->ClearAllLights();
	}
	
	// 新しいシーンを作成・初期化
	currentScene_ = it->second();
	currentSceneName_ = name;
	currentScene_->SetSceneManager(this);
	currentScene_->Initialize(engine_);
}
