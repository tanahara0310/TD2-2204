#pragma once

#include <memory>
#include "Scene/IScene.h"

class EngineSystem;
class CameraManager;
struct DirectionalLightData;

/// @brief タイトルシーンクラス
class TitleScene : public IScene {
public:
	/// @brief 初期化
	void Initialize(EngineSystem* engine) override;

	/// @brief 更新
	void Update() override;

	/// @brief 描画処理
	void Draw() override;

	/// @brief 解放
	void Finalize() override;

private:
	EngineSystem* engine_ = nullptr;
	
	// カメラマネージャー
	std::unique_ptr<CameraManager> cameraManager_ = std::make_unique<CameraManager>();
	
	// このシーン専用のディレクショナルライト
	DirectionalLightData* directionalLight_ = nullptr;
};
