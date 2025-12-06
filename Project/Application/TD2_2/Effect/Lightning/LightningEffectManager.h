#pragma once
#include "Lightning.h"
#include "Engine/Graphics/Model/ModelManager.h"
#include "Engine/Graphics/TextureManager.h"
#include <vector>
#include <memory>

class IDrawable;

/// @brief 雷エフェクトマネージャークラス（簡素化版）
class LightningEffectManager {
public:
	/// @brief エフェクト設定
	struct EffectConfig {
		Vector3 startOffset = { 0.0f, 0.0f, 0.0f };
		Vector3 endOffset = { 0.0f, 5.0f, 0.0f };
		Vector4 color = { 0.5f, 0.5f, 1.0f, 1.0f };
		float noiseScale = 0.8f;
		float noiseSpeed = 10.0f;
		int segmentCount = 20;
		Vector3 voxelScale = { 3.0f, 3.0f, 3.0f };
	};

	LightningEffectManager() = default;
	~LightningEffectManager() = default;

	/// @brief 初期化
	void Initialize(ModelManager* modelManager, TextureManager* textureManager);

	/// @brief 固定座標に雷エフェクトを作成
	/// @param position 雷の基準位置
	/// @param config エフェクト設定
	/// @param gameObjects Lightningを追加するリスト
	/// @return エフェクトID
	int CreateEffect(const Vector3& position, const EffectConfig& config,
		std::vector<std::unique_ptr<IDrawable>>& gameObjects);

	/// @brief 全エフェクトを更新
	void UpdateAllEffects();

	/// @brief エフェクトの位置を設定
	void SetEffectPosition(int effectId, const Vector3& position);

	/// @brief エフェクトのアクティブ状態を設定
	void SetEffectActive(int effectId, bool active);

	/// @brief エフェクトが有効かどうか
	bool IsEffectActive(int effectId) const;

private:
	struct EffectData {
		Lightning* lightning = nullptr;
		Vector3 position = { 0.0f, 0.0f, 0.0f };
		EffectConfig config;
		bool isActive = true;
	};

	std::vector<EffectData> effects_;
	ModelResource* voxelModelResource_ = nullptr;
	TextureManager::LoadedTexture voxelTexture_;
	int nextEffectId_ = 0;
};
