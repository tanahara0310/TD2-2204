#pragma once

#include "Scene/IScene.h"

class EngineSystem;

/// @brief ゲームシーンクラス
class GameScene : public IScene {
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
};
