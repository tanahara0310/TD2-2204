#pragma once

#include "Scene/BaseScene.h"

class EngineSystem;
class Camera;
class SpriteObject;

/// @brief Tipsシーンクラス
class TipsScene : public BaseScene {
public:
	/// @brief 初期化
	void Initialize(EngineSystem* engine) override;

	/// @brief 更新
	void Update() override;

	/// @brief 描画処理
	void Draw() override;

	/// @brief 解放
	void Finalize() override;

protected:
	/// @brief リリースカメラの初期設定をカスタマイズ
	void SetupReleaseCameraParameters(Camera* camera) override;

private:
	// 背景スプライト
	std::unique_ptr<SpriteObject> backgroundSprite_;
};
