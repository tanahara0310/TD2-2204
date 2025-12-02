#pragma once

#include <memory>
#include "Scene/BaseScene.h"
#include "Engine/Graphics/Sprite/Sprite.h"

class EngineSystem;
class CameraManager;
struct DirectionalLightData;

/// @brief リザルトシーンクラス
class ResultScene : public BaseScene {
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
	// クリア時間　上位3つ
	float clearTimes_[3]{};

	// 今回のクリアタイム
	float currentClearTime_ = 0.0f;

	// リザルト画像
	std::unique_ptr<Sprite> resultSprite_;
	TextureManager::LoadedTexture resultTexture_;

	// リスタート画像
	std::unique_ptr<Sprite> restartSprite_;

	// 「タイトルへ」画像
	std::unique_ptr<Sprite> toTitleSprite_;

	// タイマー画像
	std::unique_ptr<Sprite> curretTimeSprite_;
};
