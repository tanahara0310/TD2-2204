#pragma once

#include <cassert>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <numbers>
#include <random>
#include <vector>
#include <memory>

// エンジンコア
#include "Engine/Audio/SoundManager.h"
#include "Engine/Camera/CameraManager.h"
#include "Engine/Camera/Debug/DebugCamera.h"
#include "Engine/Camera/Release/Camera.h"
#include "MathCore.h"
#include "Utility/Logger/Logger.h"
#include "Graphics/TextureManager.h"
#include "Engine/Graphics/Light/LightData.h"

// シーン関連
#include "Scene/IScene.h"
#include "EngineSystem/EngineSystem.h"

// GameObjectのインクルード
#include "Object3d.h"
#include "TestGameObject/SphereObject.h"
#include "TestGameObject/FenceObject.h"
#include "TestGameObject/TerrainObject.h"
#include "TestGameObject/AnimatedCubeObject.h"
#include "TestGameObject/SkeletonModelObject.h"
#include "TestGameObject/WalkModelObject.h"
#include "TestGameObject/SneakWalkModelObject.h"
#include "TestGameObject/SkyBoxObject.h"
#include "TestGameObject/SpriteObject.h"

// パーティクルシステム
#include "Engine/Particle/ParticleSystem.h"

using namespace Microsoft::WRL;


/// @brief テストシーンクラス
class TestScene : public IScene {
public:
	/// @brief 初期化
	void Initialize(EngineSystem* engine) override;

	/// @brief 更新
	void Update() override;

	/// @brief 描画処理
	void Draw() override;

	/// @brief 解放
	void Finalize() override;

private: // メンバ変数

	Logger& logger = Logger::GetInstance();

	EngineSystem* engine_ = nullptr; // エンジンシステムへのポインタ

	// カメラマネージャー
	std::unique_ptr<CameraManager> cameraManager_ = std::make_unique<CameraManager>();

	// ===== ゲームオブジェクト =====
	std::vector<std::unique_ptr<Object3d>> gameObjects_;  // 全3Dオブジェクト
	std::vector<std::unique_ptr<SpriteObject>> spriteObjects_;  // 全2Dオブジェクト

	// ===== パーティクルシステム =====
	std::unique_ptr<ParticleSystem> particleSystem_;  // パーティクルシステム

	// ===== テクスチャ =====
	TextureManager::LoadedTexture textureChecker_;
	TextureManager::LoadedTexture textureCircle_;

	// ===== サウンドリソース =====
	Sound mp3Resource_;  // 自動管理されるMP3リソース

	// サウンド制御用のUI変数
	float masterVolume_ = 1.0f;
	float mp3Volume_ = 1.0f;
	bool soundLoaded_ = false;
	
	// このシーン専用のディレクショナルライト
	DirectionalLightData* directionalLight_ = nullptr;
};