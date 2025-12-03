#pragma once

#include <memory>
#include "Scene/BaseScene.h"

//ゲームオブジェクトのインクルード
#include "Object3d.h"
#include "../../GameObject/Voxel/Voxel.h"
#include "../../Effect/Lightning/LightningEffectManager.h"
#include "../../GameObject/Title/TitleUI.h"

// パーティクルシステム
#include "Engine/Particle/ParticleSystem.h"

class EngineSystem;
class CameraManager;

/// @brief タイトルシーンクラス
class TitleScene : public BaseScene {
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
	/// @brief 電気パーティクルエフェクトを作成
	void CreateElectricParticleEffect();

	/// @brief シーン遷移の処理
	void UpdateSceneTransition(float deltaTime);

private:
	std::unique_ptr<TitleUI> titleUI_;
	
	// パーティクルシステム
	ParticleSystem* electricParticle_ = nullptr;
	
	// 雷エフェクトマネージャー
	std::unique_ptr<LightningEffectManager> lightningManager_;
	
	// シーン遷移フラグとタイマー
	bool isTransitioning_ = false;
	float transitionTimer_ = 0.0f;
	static constexpr float kTransitionDuration = 1.0f; // 1秒で遷移
};
