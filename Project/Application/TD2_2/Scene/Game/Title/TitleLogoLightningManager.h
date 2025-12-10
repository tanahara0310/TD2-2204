#pragma once

#include <memory>
#include <vector>
#include "Engine/Math/Vector/Vector3.h"
#include "Engine/Math/Vector/Vector4.h"
#include "Engine/Utility/Timer/GameTimer.h"

class LightningEffectManager;
class IDrawable;

/// @brief タイトルロゴの横一直線雷エフェクトを管理するクラス
class TitleLogoLightningManager {
public:
	/// @brief 初期化
	/// @param lightningManager 雷エフェクトマネージャー
	/// @param gameObjects ゲームオブジェクトリスト（エフェクト追加用）
	void Initialize(LightningEffectManager* lightningManager, std::vector<std::unique_ptr<IDrawable>>& gameObjects);

	/// @brief 更新（アニメーション処理）
	/// @param deltaTime デルタタイム
	void Update(float deltaTime);

	/// @brief 雷エフェクトを表示
	void ShowLightning();

	/// @brief 雷エフェクトを非表示
	void HideLightning();

private:
	/// @brief アニメーションの更新
	/// @param deltaTime デルタタイム
	void UpdateAnimation(float deltaTime);

private:
	LightningEffectManager* lightningManager_ = nullptr;

	// 横一直線の雷エフェクトID
	int lightningEffectId_ = -1;

	// アニメーション状態
	bool isAnimating_ = false;
	bool isMovingForward_ = true; // true: 左→右(伸びる), false: 右→左(縮む)
	bool isWaiting_ = false; // 待機中フラグ
	GameTimer animationTimer_;
	GameTimer waitTimer_; // 待機用タイマー
	
	// アニメーション設定
	static constexpr float kAnimationDuration = 0.2f; // 片道の移動時間（雷の一閃らしく超高速化）
	static constexpr float kWaitDuration = 5.0f; // 待機時間（5秒）
	static constexpr float kHalfWidth = 6.0f; // 横幅の半分（3.0fから6.0fに拡大）

	// タイトルロゴの座標（TitleModel.cppのtargetPosition_と完全一致）
	static constexpr float kLogoY = -3.5f;   // Y座標: タイトルロゴの中心
	static constexpr float kLogoZ = -60.9f;  // Z座標: カメラから見やすい位置
	
	// 雷の色
	static constexpr Vector4 kLightningColor_ = { 0.6f, 0.9f, 1.0f, 1.0f };
};
