#include "LightningEffectManager.h"
#include "Application/TD2_2/GameObject/Voxel/Voxel.h"
#include "Application/TD2_2/Utility/GameUtils.h"
#include "Engine/Utility/Random/RandomGenerator.h"
#include "Engine/Math/Easing/EasingUtil.h"
#include <cmath>
#include <numbers>
#include <algorithm>
#include "../../Utility/GameUtils.h"

// Avoid Windows min/max macro collisions
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

void LightningEffectManager::Initialize(ModelManager* modelManager, TextureManager* textureManager)
{
   modelManager->LoadModelResource("Resources/Models/Voxel/", "voxel.obj");
   voxelModelResource_ = modelManager->GetModelResource("Resources/Models/Voxel/voxel.obj");
   voxelTexture_ = textureManager->Load("Resources/SampleResources/white1x1.png");
   startTime_ = std::chrono::steady_clock::now();
}

std::vector<Vector3> LightningEffectManager::GenerateSpherePoints(int count, float radius)
{
   std::vector<Vector3> points;
   auto& random = RandomGenerator::GetInstance();

   // 特定の少数カウントでは正多面体の頂点を使用（均等性が保証される）
   if (count == 4) {
	  // 正四面体の頂点
	  const float a = 1.0f / std::sqrt(3.0f);
	  points = {
		 { a,  a,  a},
		 { a, -a, -a},
		 {-a,  a, -a},
		 {-a, -a,  a}
	  };
   } else if (count == 6) {
	  // 正八面体の頂点（軸方向）
	  points = {
		 { 1.0f,  0.0f,  0.0f},
		 {-1.0f,  0.0f,  0.0f},
		 { 0.0f,  1.0f,  0.0f},
		 { 0.0f, -1.0f,  0.0f},
		 { 0.0f,  0.0f,  1.0f},
		 { 0.0f,  0.0f, -1.0f}
	  };
   } else if (count == 8) {
	  // 立方体の頂点
	  const float a = 1.0f / std::sqrt(3.0f);
	  points = {
		 { a,  a,  a},
		 { a,  a, -a},
		 { a, -a,  a},
		 { a, -a, -a},
		 {-a,  a,  a},
		 {-a,  a, -a},
		 {-a, -a,  a},
		 {-a, -a, -a}
	  };
   } else if (count == 12) {
	  // 正二十面体の頂点
	  const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f; // 黄金比
	  const float a = 1.0f / std::sqrt(1.0f + phi * phi);
	  const float b = phi * a;
	  points = {
		 { 0,  a,  b}, { 0,  a, -b}, { 0, -a,  b}, { 0, -a, -b},
		 { a,  b,  0}, { a, -b,  0}, {-a,  b,  0}, {-a, -b,  0},
		 { b,  0,  a}, { b,  0, -a}, {-b,  0,  a}, {-b,  0, -a}
	  };
   } else if (count == 20) {
	  // 正十二面体の頂点
	  const float phi = (1.0f + std::sqrt(5.0f)) / 2.0f;
	  const float invPhi = 1.0f / phi;
	  const float a = 1.0f / std::sqrt(3.0f);
	  points = {
		 // 立方体の頂点
		 { a,  a,  a}, { a,  a, -a}, { a, -a,  a}, { a, -a, -a},
		 {-a,  a,  a}, {-a,  a, -a}, {-a, -a,  a}, {-a, -a, -a},
		 // 長方形の頂点（3つの直交する長方形）
		 { 0,  invPhi * a,  phi * a}, { 0,  invPhi * a, -phi * a},
		 { 0, -invPhi * a,  phi * a}, { 0, -invPhi * a, -phi * a},
		 { invPhi * a,  phi * a,  0}, { invPhi * a, -phi * a,  0},
		 {-invPhi * a,  phi * a,  0}, {-invPhi * a, -phi * a,  0},
		 { phi * a,  0,  invPhi * a}, { phi * a,  0, -invPhi * a},
		 {-phi * a,  0,  invPhi * a}, {-phi * a,  0, -invPhi * a}
	  };
   } else {
	  // その他のカウント：改良版フィボナッチスフィア
	  const float goldenRatio = (1.0f + std::sqrt(5.0f)) / 2.0f;
	  const float angleIncrement = 2.0f * std::numbers::pi_v<float> / (goldenRatio * goldenRatio);

	  for (int i = 0; i < count; ++i) {
		 // インデックスを0.5オフセットして端点の偏りを軽減
		 float t = (static_cast<float>(i) + 0.5f) / static_cast<float>(count);
		 float z = 1.0f - 2.0f * t; // Z軸上の位置 (1.0f ～ -1.0f)
		 float r = std::sqrt(std::max(0.0f, 1.0f - z * z));

		 float theta = angleIncrement * i;

		 float x = r * std::cos(theta);
		 float y = r * std::sin(theta);

		 points.push_back({x, y, z});
	  }
   }

   // 正規化して半径を適用し、わずかなランダム性を追加
   for (auto& point : points) {
	  // 正規化
	  float length = std::sqrt(point.x * point.x + point.y * point.y + point.z * point.z);
	  if (length > 0.0f) {
		 point.x /= length;
		 point.y /= length;
		 point.z /= length;
	  }

	  // 半径とわずかなランダム性を適用
	  float radiusVariation = random.GetFloat(0.95f, 1.05f);
	  point.x *= radius * radiusVariation;
	  point.y *= radius * radiusVariation;
	  point.z *= radius * radiusVariation;
   }

   // シャッフルは行わない（均等配置を維持するため）

   return points;
}

int LightningEffectManager::CreateEffect(const Vector3& position, const EffectConfig& config,
   std::vector<std::unique_ptr<IDrawable>>& gameObjects)
{
   if (!voxelModelResource_) {
      return -1;
   }

   EffectData effectData;
   effectData.position = position;
   effectData.config = config;
   effectData.state = AnimationState::Hidden;
   effectData.startEndNoiseOffset = GameUtils::RandomFloat(0.0f, 1000.0f);

   if (config.useSphereDistribution) {
	  auto& random = RandomGenerator::GetInstance();

	  // 球面配置モード：複数の雷を球面上にバランスよく配置
	  auto spherePoints = GenerateSpherePoints(config.lightningCount, config.sphereRadius);

	  for (const auto& point : spherePoints) {
		 // 開始点と終点を計算（内側から外側へ）
		 Vector3 direction = point;
		 float length = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
		 if (length > 0.0f) {
			direction.x /= length;
			direction.y /= length;
			direction.z /= length;
		 }

		 // 開始点：球面の内側から（ランダム性を抑えて均等に）
		 float startRadiusVariation = random.GetFloat(0.9f, 1.1f);
		 Vector3 startPoint = {
			 direction.x * config.sphereRadius * config.sphereStartRadiusRatio * startRadiusVariation,
			 direction.y * config.sphereRadius * config.sphereStartRadiusRatio * startRadiusVariation,
			 direction.z * config.sphereRadius * config.sphereStartRadiusRatio * startRadiusVariation
		 };

		 // 終点：球面上に小さなランダムオフセット
		 float endRadiusVariation = random.GetFloat(0.95f, 1.05f);
		 Vector3 endPoint = {
		     point.x * endRadiusVariation,
			 point.y * endRadiusVariation,
			 point.z * endRadiusVariation
		 };

		 Lightning::Config lightningConfig;
		 // 初期状態は始点のみ（線の長さ0）
		 lightningConfig.startPoint = startPoint;
		 lightningConfig.endPoint = startPoint;
		 lightningConfig.segmentCount = config.segmentCount;
		 lightningConfig.noiseScale = config.noiseScale;
		 lightningConfig.noiseSpeed = config.noiseSpeed * random.GetFloat(0.8f, 1.2f);
		 lightningConfig.enableAnimation = true;
		 lightningConfig.color = config.hiddenColor; // 初期状態は非表示
		 lightningConfig.pathType = Lightning::PathType::Linear;
		 lightningConfig.voxelScale = config.voxelScale;

		 auto lightning = std::make_unique<Lightning>();
		 lightning->Initialize(voxelModelResource_, voxelTexture_, lightningConfig,
			"Lightning_" + std::to_string(nextEffectId_) + "_" + std::to_string(effectData.lightnings.size()));

		 lightning->SetActive(true);
		 lightning->GetTransform().translate = position;

		 LightningData lightningData;
		 lightningData.lightning = lightning.get();
		 lightningData.originalStartPoint = startPoint;
		 lightningData.originalEndPoint = endPoint;
		 lightningData.delay = random.GetFloat(0.0f, 0.05f); // 各ライトニングに小さな遅延
		 lightningData.noiseSeedOffset = random.GetFloat(0.0f, 100.0f);

		 effectData.lightnings.push_back(lightningData);
		 gameObjects.push_back(std::move(lightning));
	  }
   } else {
	  // 従来モード：単一の雷（初期表示は設定に従う）
	  Lightning::Config lightningConfig;
	  lightningConfig.startPoint = config.startOffset;
	  lightningConfig.endPoint = config.endOffset;
	  lightningConfig.segmentCount = config.segmentCount;
	  lightningConfig.noiseScale = config.noiseScale;
	  lightningConfig.noiseSpeed = config.noiseSpeed;
	  lightningConfig.enableAnimation = true;
	  lightningConfig.color = config.initialVisible ? config.color : config.hiddenColor;
	  lightningConfig.pathType = Lightning::PathType::Linear;
	  lightningConfig.voxelScale = config.voxelScale;

	  auto lightning = std::make_unique<Lightning>();
	  lightning->Initialize(voxelModelResource_, voxelTexture_, lightningConfig,
		 "Lightning_" + std::to_string(nextEffectId_));

	  lightning->SetActive(true);
	  lightning->GetTransform().translate = position;

	  LightningData lightningData;
	  lightningData.lightning = lightning.get();
	  lightningData.originalStartPoint = config.startOffset;
	  lightningData.originalEndPoint = config.endOffset;
	  lightningData.delay = 0.0f;

	  effectData.lightnings.push_back(lightningData);
	  gameObjects.push_back(std::move(lightning));

	  effectData.state = config.initialVisible ? AnimationState::Visible : AnimationState::Hidden;
   }

   int effectId = nextEffectId_++;
   effects_.push_back(std::move(effectData));

   return effectId;
}

void LightningEffectManager::UpdateAllEffects()
{
   float deltaTime = GameUtils::GetDeltaTime();
   auto now = std::chrono::steady_clock::now();
   float currentTime = std::chrono::duration<float>(now - startTime_).count();

   for (auto& effect : effects_) {
	  // 位置を更新
	  for (auto& lightningData : effect.lightnings) {
		 if (lightningData.lightning) {
			lightningData.lightning->GetTransform().translate = effect.position;
		 }
	  }

	  if (effect.state == AnimationState::Visible) {

		 // ノイズの基準時間（currentTime + ランダムオフセット）
		 float time = currentTime + effect.startEndNoiseOffset; // 5.0f はノイズ速度の例
		 float noiseRange = effect.config.randomOffsetRange; // 既存のランダムオフセット範囲を使用

		 for (auto& lightningData : effect.lightnings) {
			if (!lightningData.lightning) continue;

			// ノイズ値の計算。lightningData.delayとnoiseSeedOffsetを使用してユニークに揺らす
			float base = time + lightningData.delay * 10.0f + lightningData.noiseSeedOffset;

			// 始点・終点に揺らぎを与えるノイズ値を計算（Perlin Noiseの代用としてSin/Cosを使用）
			float startNoiseX = std::sin(base * 1.0f) * noiseRange;
			float startNoiseY = std::cos(base * 0.8f) * noiseRange;
			float startNoiseZ = std::sin(base * 1.2f) * noiseRange;

			float endNoiseX = std::cos(base * 1.1f + 100.0f) * noiseRange;
			float endNoiseY = std::sin(base * 0.9f + 100.0f) * noiseRange;
			float endNoiseZ = std::cos(base * 1.3f + 100.0f) * noiseRange;

			auto& config = lightningData.lightning->GetConfig();

			// 始点・終点を再配置
			config.startPoint = {
				lightningData.originalStartPoint.x + startNoiseX,
				lightningData.originalStartPoint.y + startNoiseY,
				lightningData.originalStartPoint.z + startNoiseZ
			};
			config.endPoint = {
				lightningData.originalEndPoint.x + endNoiseX,
				lightningData.originalEndPoint.y + endNoiseY,
				lightningData.originalEndPoint.z + endNoiseZ
			};

			// 色はAnimationState::FadingIn/Outで設定されているため、ここでは変更しない

			lightningData.lightning->ApplyConfigChanges();
		 }
	  }

	  // アニメーション更新
	  if (effect.state == AnimationState::FadingIn) {
		 effect.animationTimer.Update(deltaTime);
		 float progress = effect.animationTimer.GetProgress();

		 // 各ライトニングを遅延付きで更新
		 for (auto& lightningData : effect.lightnings) {
			if (!lightningData.lightning) continue;

			// 遅延を考慮した進行度
			float delayedProgress = std::max(0.0f, (progress - lightningData.delay) / (1.0f - lightningData.delay));
			delayedProgress = std::clamp(delayedProgress, 0.0f, 1.0f);
			float delayedEasedProgress = EasingUtil::Apply(delayedProgress, EasingUtil::Type::EaseOutCubic);

			// 始点から終点まで徐々に伸びる
			auto& config = lightningData.lightning->GetConfig();
			config.startPoint = lightningData.originalStartPoint;
			config.endPoint = {
				lightningData.originalStartPoint.x + (lightningData.originalEndPoint.x - lightningData.originalStartPoint.x) * delayedEasedProgress,
				lightningData.originalStartPoint.y + (lightningData.originalEndPoint.y - lightningData.originalStartPoint.y) * delayedEasedProgress,
				lightningData.originalStartPoint.z + (lightningData.originalEndPoint.z - lightningData.originalStartPoint.z) * delayedEasedProgress
			};

			// 色もフェードイン
			config.color = {
				effect.config.color.x,
				effect.config.color.y,
				effect.config.color.z,
				effect.config.color.w * delayedEasedProgress
			};

			lightningData.lightning->ApplyConfigChanges();
		 }

		 if (effect.animationTimer.IsFinished()) {
			effect.state = AnimationState::Visible;
		 }
	  } else if (effect.state == AnimationState::FadingOut) {
		 effect.animationTimer.Update(deltaTime);
		 float progress = effect.animationTimer.GetProgress();

		 // 各ライトニングを遅延付きで更新
		 for (auto& lightningData : effect.lightnings) {
			if (!lightningData.lightning) continue;

			// 遅延を考慮した進行度
			float delayedProgress = std::max(0.0f, (progress - lightningData.delay) / (1.0f - lightningData.delay));
			delayedProgress = std::clamp(delayedProgress, 0.0f, 1.0f);
			float delayedEasedProgress = EasingUtil::Apply(delayedProgress, EasingUtil::Type::EaseInCubic);

			// 始点から徐々に縮んでいく
			auto& config = lightningData.lightning->GetConfig();
			float remainingLength = 1.0f - delayedEasedProgress;
			config.startPoint = {
				lightningData.originalStartPoint.x + (lightningData.originalEndPoint.x - lightningData.originalStartPoint.x) * delayedEasedProgress,
				lightningData.originalStartPoint.y + (lightningData.originalEndPoint.y - lightningData.originalStartPoint.y) * delayedEasedProgress,
				lightningData.originalStartPoint.z + (lightningData.originalEndPoint.z - lightningData.originalStartPoint.z) * delayedEasedProgress
			};
			config.endPoint = lightningData.originalEndPoint;

			// 色もフェードアウト
			config.color = {
				effect.config.color.x,
				effect.config.color.y,
				effect.config.color.z,
				effect.config.color.w * remainingLength
			};

			lightningData.lightning->ApplyConfigChanges();
		 }

		 if (effect.animationTimer.IsFinished()) {
			effect.state = AnimationState::Hidden;
			// 完全に非表示
			for (auto& lightningData : effect.lightnings) {
			   if (lightningData.lightning) {
				  auto& config = lightningData.lightning->GetConfig();
				  config.color = effect.config.hiddenColor;
				  lightningData.lightning->ApplyConfigChanges();
			   }
			}

			effect.startEndNoiseOffset = GameUtils::RandomFloat(0.0f, 1000.0f);
		 }
	  }
   }
}

void LightningEffectManager::SetEffectPosition(int effectId, const Vector3& position)
{
   if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
	  return;
   }
   effects_[effectId].position = position;
}

void LightningEffectManager::SetEffectVisible(int effectId, bool visible)
{
   if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
	  return;
   }

   auto& effect = effects_[effectId];

   if (visible && (effect.state == AnimationState::Hidden || effect.state == AnimationState::FadingOut)) {
	  // フェードイン開始
	  effect.state = AnimationState::FadingIn;
	  effect.animationTimer.Start(effect.config.fadeInDuration, false);
   } else if (!visible && (effect.state == AnimationState::Visible || effect.state == AnimationState::FadingIn)) {
	  // フェードアウト開始
	  effect.state = AnimationState::FadingOut;
	  effect.animationTimer.Start(effect.config.fadeOutDuration, false);
   }
}

void LightningEffectManager::SetEffectVisibleImmediate(int effectId, bool visible)
{
   if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
      return;
   }
   auto& effect = effects_[effectId];
   effect.state = visible ? AnimationState::Visible : AnimationState::Hidden;
   for (auto& lightningData : effect.lightnings) {
      if (!lightningData.lightning) continue;
      auto& cfg = lightningData.lightning->GetConfig();
      cfg.color = visible ? effect.config.color : effect.config.hiddenColor;
      lightningData.lightning->ApplyConfigChanges();
   }
}

bool LightningEffectManager::IsEffectVisible(int effectId) const
{
   if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
	  return false;
   }
   return effects_[effectId].state == AnimationState::Visible ||
	  effects_[effectId].state == AnimationState::FadingIn;
}

void LightningEffectManager::SetEffectColor(int effectId, const Vector4& color)
{
   if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
	  return;
   }

   auto& effect = effects_[effectId];
   effect.config.color = color;

   // 全てのライトニングの色を更新
   for (auto& lightningData : effect.lightnings) {
	  if (lightningData.lightning) {
		 lightningData.lightning->SetColor(color);
	  }
   }
}

Vector4 LightningEffectManager::GetEffectColor(int effectId) const
{
   if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
	  return { 1.0f, 1.0f, 1.0f, 1.0f }; // デフォルトは白色
   }

   return effects_[effectId].config.color;
}
