#pragma once

#include <memory>
#include "Engine/Math/Vector/Vector3.h"

class TitlePlayerDemo;
class TitleEnemyDemo;

/// @brief タイトルシーンのデモ演出を管理するクラス
class TitleDemoManager {
public:
	/// @brief デモパターン
	enum class DemoPattern {
		EnemyChasePlayer,  // 敵が自機を追跡
		PlayerChaseEnemy   // 自機が敵を追跡
	};

	/// @brief 初期化
	/// @param player デモプレイヤー
	/// @param enemy デモエネミー
	void Initialize(TitlePlayerDemo* player, TitleEnemyDemo* enemy);

	/// @brief 更新
	/// @param deltaTime デルタタイム
	void Update(float deltaTime);

	/// @brief パターンを手動で切り替え
	void SwitchPattern();

#ifdef _DEBUG
	/// @brief ImGuiデバッグ表示
	void DrawImGui();
#endif

private:
	/// @brief プレイヤーが敵を追跡するパターンに切り替え
	/// @param direction 移動方向（1.0 = 右, -1.0 = 左）
	void SwitchToPlayerChaseEnemy(float direction);

	/// @brief 敵がプレイヤーを追跡するパターンに切り替え
	/// @param direction 移動方向（1.0 = 右, -1.0 = 左）
	void SwitchToEnemyChasePlayer(float direction);

	/// @brief デモキャラクターの位置を設定
	void SetDemoPositions();

private:
	TitlePlayerDemo* demoPlayer_ = nullptr;
	TitleEnemyDemo* demoEnemy_ = nullptr;

	DemoPattern currentDemoPattern_ = DemoPattern::EnemyChasePlayer;
	bool isMovingRight_ = true; // true = +X方向, false = -X方向

	// デモのZ位置管理
	bool isDemoBehindBackground_ = true; // true = 背景の後ろ, false = 背景の前
	int demoSwitchCounter_ = 0; // パターン切り替え回数カウンター
	static constexpr int kDemoBehindCount_ = 2; // 背景の後ろで移動する回数
	static constexpr int kDemoInFrontCount_ = 1; // 背景の前で移動する回数
	static constexpr int kDemoTotalCycle_ = kDemoBehindCount_ + kDemoInFrontCount_; // 合計サイクル（3回）
	static constexpr float kDemoZBehind_ = 10.0f; // 背景の後ろ
	static constexpr float kDemoZFront_ = -15.0f; // 背景の前（めり込み防止のためより手前に）

	// デモのY座標管理（3段階）
	static constexpr float kDemoYTop_ = 28.0f;    // 上段
	static constexpr float kDemoYMiddle_ = 24.0f; // 中段（デフォルト）
	static constexpr float kDemoYBottom_ = 20.0f; // 下段
	int lastDemoYIndex_ = 1; // 前回選択したY座標のインデックス（0:上, 1:中, 2:下）
};
