#pragma once
#include "Lightning.h"
#include "Application/TD2_2/GameObject/GameObject.h"
#include "Application/TD2_2/GameObject/Voxel/Voxel.h"
#include "Engine/Graphics/Model/ModelManager.h"
#include "Engine/Graphics/TextureManager.h"
#include <vector>
#include <memory>

/// @brief 雷エフェクトマネージャークラス
/// GameObjectの周囲に球面配置された雷エフェクトを管理
class LightningEffectManager {
public:
	/// @brief 球面エフェクトの設定
	struct SphericalEffectConfig {
		float radius = 2.5f;              // 球の半径
		float arcLength = 0.5f;           // 各雷の円弧の長さ（ラジアン）
		int lightningCount = 5;           // 雷の数（球面上の配置数）
		int visibleCount = 5;             // 表示する雷の数（StartEffect時に常に表示）
		Vector4 color = { 1.0f, 0.3f, 0.3f, 1.0f }; // 雷の色
		float noiseScale = 0.6f;          // ノイズの振り幅
		float noiseSpeed = 15.0f;         // アニメーション速度
		int segmentCount = 15;            // セグメント数
		float effectDuration = 0.5f;      // エフェクトの継続時間（秒）
		float staggerDelay = 0.05f;       // 各雷の出現間隔（秒）
		bool enableStagger = true;        // 時間差出現を有効にする
	};

	LightningEffectManager() = default;
	~LightningEffectManager() = default;

	/// @brief 初期化
	void Initialize(ModelManager* modelManager, TextureManager* textureManager);

	/// @brief GameObjectの周りに球面雷エフェクトを作成
	/// @param target 追従対象のGameObject
	/// @param config エフェクト設定
	/// @param gameObjects Lightningを追加するリスト
	/// @return エフェクトID
	int CreateCircularEffect(GameObject* target, const SphericalEffectConfig& config,
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
	SphericalEffectConfig& GetEffectConfig(int effectId);

	/// @brief デバッグUI表示
	void DrawDebugUI(int effectId, const char* windowName = "Lightning Effect");

private:
	/// @brief エフェクトデータ
	struct EffectData {
		GameObject* target = nullptr;
		std::vector<Lightning*> lightnings;
		std::vector<bool> visibility;
		std::vector<Vector3> sphericalPositions; // 球面上の位置（始点用）
		std::vector<int> spawnOrder;             // 出現順序（ランダム化されたインデックス）
		std::vector<float> spawnTimes;           // 各雷の出現時刻
		SphericalEffectConfig config;
		bool isActive = false;
		float timer = 0.0f;
		int currentSpawnIndex = 0;               // 現在出現中のインデックス
	};

	/// @brief エフェクトの位置を更新
	void UpdateEffectPosition(EffectData& effect);

	/// @brief エフェクトのタイマーを更新
	void UpdateEffectTimer(EffectData& effect);

	/// @brief 時間差出現を更新
	void UpdateStaggeredSpawns(EffectData& effect);

	/// @brief 雷の色を更新
	void UpdateLightningColors(EffectData& effect);

	/// @brief 指定インデックスの雷の可視性を設定
	void SetLightningVisibility(EffectData& effect, int index, bool visible);

	/// @brief 全ての雷の可視性を設定
	void SetAllLightningsVisibility(EffectData& effect, bool visible);

	/// @brief 球面上の点を計算（フィボナッチ球面配置）
	std::vector<Vector3> GenerateSphericalPoints(int count, float radius);

	std::vector<EffectData> effects_;
	ModelResource* voxelModelResource_ = nullptr;
	TextureManager::LoadedTexture voxelTexture_;
	int nextEffectId_ = 0;
};
