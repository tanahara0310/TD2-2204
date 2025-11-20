#pragma once

#include <memory>
#include "Scene/IScene.h"

//ゲームオブジェクトのインクルード
#include "Object3d.h"
#include "../../GameObject/Title/Sphere.h"

class EngineSystem;
class CameraManager;

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

	std::vector<std::unique_ptr<Object3d>> gameObjects_;
};
