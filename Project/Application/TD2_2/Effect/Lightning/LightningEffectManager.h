#pragma once
#include "Lightning.h"
#include "Application/TD2_2/GameObject/GameObject.h"
#include "Application/TD2_2/GameObject/Voxel/Voxel.h"
#include "Engine/Graphics/Model/ModelManager.h"
#include "Engine/Graphics/TextureManager.h"
#include <vector>
#include <memory>

/// @brief 雷エフェクトマネージャークラス
/// GameObjectの周囲に円形配置された雷エフェクトを管理
class LightningEffectManager {
public:
	/// @brief 円形エフェクトの設定
	struct CircularEffectConfig {
		float radius = 2.5f;              // 円の半径
		float arcLength = 0.5f;           // 各雷の円弧の長さ（ラジアン）
		int lightningCount = 5;           // 雷の数
		Vector4 color = { 1.0f, 0.3f, 0.3f, 1.0f }; // 雷の色
		float noiseScale = 0.6f;          // ノイズの振り幅
		float noiseSpeed = 15.0f;         // アニメーション速度
		int segmentCount = 15;            // セグメント数
		float displayProbability = 0.5f;  // 各雷の表示確率（StartEffect時）
		int minVisibleCount = 2;          // 最低表示数（StartEffect時）
		float effectDuration = 0.5f;      // エフェクトの継続時間（秒）
	};

	LightningEffectManager() = default;
	~LightningEffectManager() = default;

	/// @brief 初期化
	void Initialize(ModelManager* modelManager, TextureManager* textureManager);

	/// @brief GameObjectの周りに円形雷エフェクトを作成
	/// @param target 追従対象のGameObject
	/// @param config エフェクト設定
	/// @param gameObjects Lightningを追加するリスト
	/// @return エフェクトID
	int CreateCircularEffect(GameObject* target, const CircularEffectConfig& config,
		std::vector<std::unique_ptr<IDrawable>>& gameObjects);

	/// @brief 全エフェクトを更新
	void UpdateAllEffects();

	/// @brief エフェクトをランダムに表示開始
	void StartEffect(int effectId);

	/// @brief エフェクトを停止
	void StopEffect(int effectId);

	/// @brief エフェクトが有効かどうか
	bool IsEffectActive(int effectId) const;

	/// @brief エフェクト設定を取得
	CircularEffectConfig& GetEffectConfig(int effectId);

	/// @brief デバッグUI表示
	void DrawDebugUI(int effectId, const char* windowName = "Lightning Effect");

private:
	/// @brief エフェクトデータ
	struct EffectData {
		GameObject* target = nullptr;
		std::vector<Lightning*> lightnings;
		std::vector<bool> visibility;
		CircularEffectConfig config;
		bool isActive = false;
		float timer = 0.0f;
	};

	/// @brief エフェクトの位置を更新
	void UpdateEffectPosition(EffectData& effect);

	/// @brief エフェクトのタイマーを更新
	void UpdateEffectTimer(EffectData& effect);

	/// @brief 雷の色を更新
	void UpdateLightningColors(EffectData& effect);

	std::vector<EffectData> effects_;
	ModelResource* voxelModelResource_ = nullptr;
	TextureManager::LoadedTexture voxelTexture_;
	int nextEffectId_ = 0;
};
