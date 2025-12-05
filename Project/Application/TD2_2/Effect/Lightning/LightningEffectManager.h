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

	/// @brief 直線エフェクトの設定
	struct LinearEffectConfig {
		Vector3 startOffset = { 0.0f, 0.0f, 0.0f };  // 始点のオフセット（ターゲット基準）
		Vector3 endOffset = { 0.0f, 5.0f, 0.0f };    // 終点のオフセット（ターゲット基準）
		Vector4 color = { 0.5f, 0.5f, 1.0f, 1.0f };  // 雷の色
		float noiseScale = 0.8f;                     // ノイズの振り幅
		float noiseSpeed = 10.0f;                    // アニメーション速度
		int segmentCount = 20;                       // セグメント数
		Lightning::PathType pathType = Lightning::PathType::Linear;  // パスタイプ
		bool enableAnimation = true;                 // アニメーション有効化
		Vector3 voxelScale = { 3.0f, 3.0f, 3.0f };   // ボクセルのスケール
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

	/// @brief GameObjectから直線状の雷エフェクトを作成
	/// @param target 追従対象のGameObject
	/// @param config エフェクト設定
	/// @param gameObjects Lightningを追加するリスト
	/// @return エフェクトID（雷オブジェクトへのアクセス用）
	int CreateLinearEffect(GameObject* target, const LinearEffectConfig& config,
		std::vector<std::unique_ptr<IDrawable>>& gameObjects);

	/// @brief 固定座標に直線状の雷エフェクトを作成（GameObject不要）
	/// @param position 雷の基準位置
	/// @param config エフェクト設定
	/// @param gameObjects Lightningを追加するリスト
	/// @return エフェクトID（雷オブジェクトへのアクセス用）
	int CreateLinearEffectAtPosition(const Vector3& position, const LinearEffectConfig& config,
		std::vector<std::unique_ptr<IDrawable>>& gameObjects);

	/// @brief 全エフェクトを更新
	void UpdateAllEffects();

	/// @brief エフェクトをランダムに表示開始（球面エフェクト用）
	void StartEffect(int effectId);

	/// @brief エフェクトを停止
	void StopEffect(int effectId);

	/// @brief エフェクトが有効かどうか
	bool IsEffectActive(int effectId) const;

	/// @brief 球面エフェクト設定を取得
	SphericalEffectConfig& GetEffectConfig(int effectId);

	/// @brief 直線エフェクト設定を取得
	LinearEffectConfig& GetLinearEffectConfig(int effectId);

	/// @brief エフェクトのスケールを設定
	/// @param effectId エフェクトID
	/// @param scale 新しいスケール
	void SetEffectScale(int effectId, const Vector3& scale);

	/// @brief デバッグUI表示
	void DrawDebugUI(int effectId, const char* windowName = "Lightning Effect");

private:
	/// @brief エフェクトタイプ
	enum class EffectType {
		Spherical,  // 球面配置
		Linear      // 直線配置
	};

	/// @brief エフェクトデータ
	struct EffectData {
		EffectType type = EffectType::Spherical;
		GameObject* target = nullptr;
		Vector3 fixedPosition = { 0.0f, 0.0f, 0.0f };  // 固定座標（target == nullptrの場合に使用）
		std::vector<Lightning*> lightnings;
		std::vector<bool> visibility;
		std::vector<Vector3> sphericalPositions; // 球面上の位置（始点用）
		std::vector<int> spawnOrder;             // 出現順序（ランダム化されたインデックス）
		std::vector<float> spawnTimes;           // 各雷の出現時刻
		SphericalEffectConfig config;
		LinearEffectConfig linearConfig;
		bool isActive = false;
		float timer = 0.0f;
		int currentSpawnIndex = 0;               // 現在出現中のインデックス
	};

	/// @brief エフェクトの位置を更新
	void UpdateEffectPosition(EffectData& effect);

	/// @brief 直線エフェクトの位置を更新
	void UpdateLinearEffectPosition(EffectData& effect);

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
