#pragma once

#include <memory>
#include "Scene/BaseScene.h"

//ゲームオブジェクトのインクルード
#include "Object3d.h"
#include "../../GameObject/Voxel/Voxel.h"
#include "../../GameObject/Lightning/Lightning.h"
#include "../../GameObject/Title/TitleUI.h"

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
	std::unique_ptr<TitleUI> titleUI_;
};
