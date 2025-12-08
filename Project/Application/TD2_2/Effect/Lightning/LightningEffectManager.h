#pragma once
#include "Lightning.h"
#include "Engine/Graphics/Model/ModelManager.h"
#include "Engine/Graphics/TextureManager.h"
#include "Engine/Utility/Timer/GameTimer.h"
#include <vector>
#include <memory>
#include <chrono>

class IDrawable;

/// @brief 雷エフェクトマネージャークラス（簡素化版）
class LightningEffectManager {
public:
   /// @brief エフェクト設定
   struct EffectConfig {
	  Vector3 startOffset = { 0.0f, 0.0f, 0.0f };
	  Vector3 endOffset = { 0.0f, 5.0f, 0.0f };
	  Vector4 color = { 0.5f, 0.5f, 1.0f, 1.0f };
	  Vector4 hiddenColor = { 0.0f, 0.0f, 0.0f, 0.0f }; // 非表示時の色（完全透明）
	  float noiseScale = 0.8f;
	  float noiseSpeed = 10.0f;
	  int segmentCount = 20;
	  Vector3 voxelScale = { 3.0f, 3.0f, 3.0f };

	  // 球面配置設定
	  bool useSphereDistribution = false; // 球面配置を使用するか
	  float sphereRadius = 2.0f;          // 球面の半径
	  int lightningCount = 8;             // 雷の本数
	  float sphereStartRadiusRatio = 0.5f; // 開始位置の半径比率（0.0f～1.0f）
	  float randomOffsetRange = 0.3f;     // 位置のランダムオフセット範囲

	  // アニメーション設定
	  float fadeInDuration = 0.2f;  // フェードイン時間
	  float fadeOutDuration = 0.3f; // フェードアウト時間
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

   /// @brief エフェクトの表示状態を設定（アニメーション付き）
   void SetEffectVisible(int effectId, bool visible);

   /// @brief エフェクトが表示中かどうか
   bool IsEffectVisible(int effectId) const;

   /// @brief エフェクトの色を設定（演出中に変更可能）
   /// @param effectId エフェクトID
   /// @param color 新しい色（RGBA）
   void SetEffectColor(int effectId, const Vector4& color);

   /// @brief エフェクトの色を取得
   /// @param effectId エフェクトID
   /// @return 現在の色（RGBA）、エフェクトが存在しない場合は白色を返す
   Vector4 GetEffectColor(int effectId) const;

private:
   std::chrono::time_point<std::chrono::steady_clock> startTime_;

   /// @brief 球面上の点を計算（均等分布）
   std::vector<Vector3> GenerateSpherePoints(int count, float radius);

   /// @brief アニメーション状態
   enum class AnimationState {
	  Hidden,    // 完全非表示
	  FadingIn,  // フェードイン中
	  Visible,   // 完全表示
	  FadingOut  // フェードアウト中
   };

   struct LightningData {
	  Lightning* lightning = nullptr;
	  Vector3 originalStartPoint;
	  Vector3 originalEndPoint;
	  float delay = 0.0f; // ランダムな遅延
	  float noiseSeedOffset = 0.0f;
   };

   struct EffectData {
	  std::vector<LightningData> lightnings; // 複数の雷（球面配置用）
	  Vector3 position = { 0.0f, 0.0f, 0.0f };
	  EffectConfig config;
	  AnimationState state = AnimationState::Hidden;
	  GameTimer animationTimer;

	  float startEndNoiseOffset = 0.0f;
   };

   std::vector<EffectData> effects_;
   ModelResource* voxelModelResource_ = nullptr;
   TextureManager::LoadedTexture voxelTexture_;
   int nextEffectId_ = 0;
};
