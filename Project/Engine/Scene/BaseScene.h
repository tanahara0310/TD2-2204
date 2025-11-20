#pragma once

#include "IScene.h"
#include "Engine/Graphics/Light/LightData.h"
#include <memory>

class EngineSystem;
class CameraManager;
class DirectXCommon;

/// @brief シーンの基底クラス（共通処理を実装）
class BaseScene : public IScene {
public:

	virtual ~BaseScene() = default;

	/// @brief 初期化（共通処理 + 派生クラスの初期化）
	virtual void Initialize(EngineSystem* engine) override;

	/// @brief 更新（共通処理 + 派生クラスの更新）
	virtual void Update() override;

	/// @brief 描画処理（共通処理 + 派生クラスの描画）
	virtual void Draw() override;

	/// @brief 解放（共通処理 + 派生クラスの解放）
	virtual void Finalize() override;

private:


	void SetupCamera();

	void SetupLight();

protected:
	// 派生クラスからアクセス可能な共通メンバー
	EngineSystem* engine_ = nullptr;
	std::unique_ptr<CameraManager> cameraManager_;
	DirectionalLightData* directionalLight_ = nullptr;
};
