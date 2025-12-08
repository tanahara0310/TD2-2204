#pragma once

#include <memory>
#include <vector>
#include "Scene/BaseScene.h"
#include "../../GameObject/Recommended/ControllerModel.h"
#include "../../GameObject/Recommended/RecommendedModel.h"
#include "../../GameObject/Voxel/Voxel.h"

class EngineSystem;
class CameraManager;
class Camera;

/// @brief コントローラー推奨シーンクラス
class RecommendedScene : public BaseScene {
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
	// 黒い背景ボクセル
	Voxel* blackBackground_ = nullptr;
	
	// コントローラーモデル
	ControllerModel* controllerModel_ = nullptr;
	
	// Recommendedモデル（複数の文字）
	std::vector<RecommendedModel*> textModels_;
	
	// ボタン入力待ちフラグ
	bool waitingForInput_ = false;
	
	// 待機時間タイマー
	float waitTimer_ = 0.0f;
	static constexpr float kWaitDuration = 1.0f; // 1秒待機してから入力受付開始
	
	// テキストアニメーション用
	static constexpr float kTextAppearDelay = 0.1f; // 各文字の出現遅延時間（秒）
};
