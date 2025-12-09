#pragma once

#include "Engine/Math/Vector/Vector4.h"
#include "Engine/Utility/Timer/GameTimer.h"

class TitleUI;
class LightningEffectManager;

/// @brief タイトルシーンの決定演出を管理するクラス
class TitleConfirmAnimationManager {
public:
	/// @brief 初期化
	/// @param titleUI タイトルUI
	/// @param lightningManager 雷エフェクトマネージャー
	void Initialize(TitleUI* titleUI, LightningEffectManager* lightningManager);

	/// @brief 決定演出を開始
	/// @param isStartSelected スタートが選択されているか
	/// @param frameEffectIds 枠エフェクトID配列
	void StartAnimation(bool isStartSelected, int frameEffectIds[4]);

	/// @brief 更新
	/// @param deltaTime デルタタイム
	void Update(float deltaTime);

	/// @brief 演出が終了したか
	/// @return 終了していればtrue
	bool IsFinished() const { return !isAnimating_; }

	/// @brief 演出中か
	/// @return 演出中ならtrue
	bool IsAnimating() const { return isAnimating_; }

	/// @brief 演出終了後、スタートが選択されていたか
	/// @return スタート選択ならtrue
	bool WasStartSelected() const { return wasStartSelected_; }

private:
	TitleUI* titleUI_ = nullptr;
	LightningEffectManager* lightningManager_ = nullptr;

	bool isAnimating_ = false;
	bool wasStartSelected_ = false;
	GameTimer animationTimer_;
	float selectedModelInitialScale_ = 1.0f;
	Vector4 unselectedModelOriginalColor_ = { 1.0f, 1.0f, 1.0f, 1.0f };

	int* frameEffectIds_ = nullptr;

	static constexpr float kAnimationDuration_ = 0.3f;
	static constexpr float kSelectedScaleMax_ = 1.5f;
};
