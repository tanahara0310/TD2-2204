#pragma once

#include <memory>
#include <vector>
#include "Engine/Math/Vector/Vector3.h"
#include "Engine/Math/Vector/Vector4.h"

class LightningEffectManager;
class IDrawable;

/// @brief タイトルシーンの選択枠雷エフェクトを管理するクラス
class TitleLightningFrameManager {
public:
	/// @brief 初期化
	/// @param lightningManager 雷エフェクトマネージャー
	/// @param gameObjects ゲームオブジェクトリスト（エフェクト追加用）
	void Initialize(LightningEffectManager* lightningManager, std::vector<std::unique_ptr<IDrawable>>& gameObjects);

	/// @brief 更新（パルス演出を含む）
	/// @param deltaTime デルタタイム
	/// @param isConfirmAnimating 決定演出中かどうか
	void Update(float deltaTime, bool isConfirmAnimating);

	/// @brief 選択状態に応じて枠の位置と色を更新
	/// @param isStartSelected スタートが選択されているか
	void UpdateFramePosition(bool isStartSelected);

	/// @brief 決定時に全辺を即座に表示
	void ShowAllEdges();

	/// @brief 全辺を即座に非表示
	void HideAllEdges();

private:
	/// @brief パルス演出の更新
	/// @param deltaTime デルタタイム
	void UpdatePulseEffect(float deltaTime);

private:
	LightningEffectManager* lightningManager_ = nullptr;

	// 枠を構成する4本のエフェクトID（上,下,左,右）
	int frameEffectIds_[4] = { -1, -1, -1, -1 };

	// パルス演出用
	float pulseTimer_ = 0.0f;
	float visibleTimer_ = 0.0f;
	int currentEdgeIndexA_ = -1;
	int currentEdgeIndexB_ = -1;
	float flickerTimer_ = 0.0f;
	static constexpr float kPulseDuration_ = 0.25f;
	static constexpr float kFlickerInterval_ = 0.03f;

	// 選択状態キャッシュ
	bool lastIsStartSelected_ = true;

	// 色定義
	static constexpr Vector4 kStartColor_ = { 0.6f, 0.9f, 1.0f, 1.0f };
	static constexpr Vector4 kQuitColor_ = { 0.6f, 0.9f, 1.0f, 1.0f };

	// 位置定義
	static constexpr float kFrameYOffset_ = 0.3f;
	static constexpr Vector3 kStartCenter_ = { 0.0f, -5.5f + kFrameYOffset_, -60.4f };
	static constexpr Vector3 kQuitCenter_ = { 0.0f, -7.0f + kFrameYOffset_, -60.4f };
};
