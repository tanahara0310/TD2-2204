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

namespace {
// ロドリゲスの回転公式を用いてベクトルvをaxis周りにangleラジアン回転させる
Vector3 RotateVector(const Vector3& v, const Vector3& axis, float angle) {
   float cosTheta = std::cos(angle);
   float sinTheta = std::sin(angle);

   // v * cos(theta)
   Vector3 term1 = v * cosTheta;

   // (axis x v) * sin(theta)
   Vector3 crossProd = {
	   axis.y * v.z - axis.z * v.y,
	   axis.z * v.x - axis.x * v.z,
	   axis.x * v.y - axis.y * v.x
   };
   Vector3 term2 = crossProd * sinTheta;

   // axis * (axis . v) * (1 - cos(theta))
   float dotProd = axis.x * v.x + axis.y * v.y + axis.z * v.z;
   Vector3 term3 = axis * (dotProd * (1.0f - cosTheta));

   return term1 + term2 + term3;
}
}

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

	  std::shuffle(spherePoints.begin(), spherePoints.end(), *random.GetEngine());

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

		 Vector3 randomAxis = {
			 random.GetFloat(-1.0f, 1.0f),
			 random.GetFloat(-1.0f, 1.0f),
			 random.GetFloat(-1.0f, 1.0f)
		 };
		 // 正規化（ゼロ除算対策含む）
		 float axisLen = std::sqrt(randomAxis.x * randomAxis.x + randomAxis.y * randomAxis.y + randomAxis.z * randomAxis.z);
		 if (axisLen > 0.001f) {
			lightningData.orbitAxis = { randomAxis.x / axisLen, randomAxis.y / axisLen, randomAxis.z / axisLen };
		 } else {
			lightningData.orbitAxis = { 0.0f, 1.0f, 0.0f };
		 }

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
	  lightningConfig.voxelSpacing = config.voxelSpacing; // 追加: ボクセル間隔を伝播

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
	  lightningData.orbitAxis = { 0.0f, 1.0f, 0.0f };

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

	  // =========================================================
	  // ▼ 修正ポイント: 回転角度の計算を条件付きにする
	  //    球面配置(useSphereDistribution)の場合のみ回転させる
	  // =========================================================
	  float orbitAngle = 0.0f;
	  if (effect.config.useSphereDistribution) {
		 orbitAngle = currentTime * effect.config.orbitSpeed;
	  }

	  if (effect.state == AnimationState::Visible) {

		 float time = currentTime + effect.startEndNoiseOffset;
		 float noiseRange = effect.config.randomOffsetRange;

		 for (auto& lightningData : effect.lightnings) {
			if (!lightningData.lightning) continue;

			// ▼ 修正済みの orbitAngle を使用 (単体モードなら0なので回転しない)
			Vector3 rotatedStart = RotateVector(lightningData.originalStartPoint, lightningData.orbitAxis, orbitAngle);
			Vector3 rotatedEnd = RotateVector(lightningData.originalEndPoint, lightningData.orbitAxis, orbitAngle);

			// 2. ノイズの計算 (変更なし)
			float base = time + lightningData.delay * 10.0f + lightningData.noiseSeedOffset;

			float startNoiseX = std::sin(base * 1.0f) * noiseRange;
			float startNoiseY = std::cos(base * 0.8f) * noiseRange;
			float startNoiseZ = std::sin(base * 1.2f) * noiseRange;

			float endNoiseX = std::cos(base * 1.1f + 100.0f) * noiseRange;
			float endNoiseY = std::sin(base * 0.9f + 100.0f) * noiseRange;
			float endNoiseZ = std::cos(base * 1.3f + 100.0f) * noiseRange;

			auto& config = lightningData.lightning->GetConfig();

			// 3. 座標設定
			config.startPoint = {
			   rotatedStart.x + startNoiseX,
			   rotatedStart.y + startNoiseY,
			   rotatedStart.z + startNoiseZ
			};
			config.endPoint = {
			   rotatedEnd.x + endNoiseX,
			   rotatedEnd.y + endNoiseY,
			   rotatedEnd.z + endNoiseZ
			};

			lightningData.lightning->ApplyConfigChanges();
		 }
	  } else if (effect.state == AnimationState::FadingIn) {
		 effect.animationTimer.Update(deltaTime);
		 float progress = effect.animationTimer.GetProgress();

		 // ▼ ここでは既に上で計算した orbitAngle が有効です
		 // float orbitAngle = currentTime * effect.config.orbitSpeed; // ← この行は削除か、上の共通変数を使う

		 for (auto& lightningData : effect.lightnings) {
			if (!lightningData.lightning) continue;

			float delayedProgress = std::max(0.0f, (progress - lightningData.delay) / (1.0f - lightningData.delay));
			delayedProgress = std::clamp(delayedProgress, 0.0f, 1.0f);
			float delayedEasedProgress = EasingUtil::Apply(delayedProgress, EasingUtil::Type::EaseOutCubic);

			// 回転後の座標を取得（単体モードなら元の座標と同じになる）
			Vector3 currentStart = RotateVector(lightningData.originalStartPoint, lightningData.orbitAxis, orbitAngle);
			Vector3 currentEnd = RotateVector(lightningData.originalEndPoint, lightningData.orbitAxis, orbitAngle);

			auto& config = lightningData.lightning->GetConfig();

			// 以下変更なし
			config.startPoint = currentStart;
			config.endPoint = {
			   currentStart.x + (currentEnd.x - currentStart.x) * delayedEasedProgress,
			   currentStart.y + (currentEnd.y - currentStart.y) * delayedEasedProgress,
			   currentStart.z + (currentEnd.z - currentStart.z) * delayedEasedProgress
			};

			float baseAlpha = config.color.w; // SetEffectIntensityで設定された現在のアルファ値(非表示なら0)

			config.color = {
			   effect.config.color.x, effect.config.color.y, effect.config.color.z,
			   baseAlpha * delayedEasedProgress
			};

			lightningData.lightning->ApplyConfigChanges();
		 }

		 if (effect.animationTimer.IsFinished()) {
			effect.state = AnimationState::Visible;
		 }
	  } else if (effect.state == AnimationState::FadingOut) {
		 effect.animationTimer.Update(deltaTime);
		 float progress = effect.animationTimer.GetProgress();

		 // ▼ 同様に修正
		 // float orbitAngle = currentTime * effect.config.orbitSpeed; // ← 削除

		 for (auto& lightningData : effect.lightnings) {
			if (!lightningData.lightning) continue;

			float delayedProgress = std::max(0.0f, (progress - lightningData.delay) / (1.0f - lightningData.delay));
			delayedProgress = std::clamp(delayedProgress, 0.0f, 1.0f);
			float delayedEasedProgress = EasingUtil::Apply(delayedProgress, EasingUtil::Type::EaseInCubic);

			// 回転後の座標を取得
			Vector3 currentStart = RotateVector(lightningData.originalStartPoint, lightningData.orbitAxis, orbitAngle);
			Vector3 currentEnd = RotateVector(lightningData.originalEndPoint, lightningData.orbitAxis, orbitAngle);

			auto& config = lightningData.lightning->GetConfig();

			float remainingLength = 1.0f - delayedEasedProgress;
			config.startPoint = {
			   currentStart.x + (currentEnd.x - currentStart.x) * delayedEasedProgress,
			   currentStart.y + (currentEnd.y - currentStart.y) * delayedEasedProgress,
			   currentStart.z + (currentEnd.z - currentStart.z) * delayedEasedProgress
			};
			config.endPoint = currentEnd;

			config.color = {
			   effect.config.color.x,
			   effect.config.color.y,
			   effect.config.color.z,
			   effect.config.color.w * remainingLength
			};

			lightningData.lightning->ApplyConfigChanges();
		 }

		 if (effect.animationTimer.IsFinished()) {
			// ... (変更なし) ...
			effect.state = AnimationState::Hidden;
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

void LightningEffectManager::SetEffectIntensity(int effectId, float intensity)
{
   if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
	  return;
   }

   auto& effect = effects_[effectId];
   intensity = std::clamp(intensity, 0.0f, 1.0f);

   // 球面配置（複数の雷）の場合のみ本数制御を行う
   if (effect.config.useSphereDistribution) {
	  size_t totalCount = effect.lightnings.size();

	  // 少なくとも1本は表示するか、あるいは0なら完全に消すかの方針に合わせて調整
	  // ここでは intensity * totalCount の数だけ表示する
	  size_t activeCount = static_cast<size_t>(std::ceil(totalCount * intensity));

	  // intensityが0より大きいが計算上0になる場合、最低1本出す演出にするなら以下を有効化
	  // if (intensity > 0.01f && activeCount == 0) activeCount = 1;

	  for (size_t i = 0; i < totalCount; ++i) {
		 if (!effect.lightnings[i].lightning) continue;

		 auto& lightning = effect.lightnings[i].lightning;
		 auto& config = lightning->GetConfig();

		 if (i < activeCount) {
			// 表示: 設定された色を適用
			// ノイズ速度もエネルギーが高いほど速くすると迫力が出ます
			float speedMultiplier = 1.0f + intensity * 2.0f;
			config.noiseSpeed = effect.config.noiseSpeed * speedMultiplier;

			// 色のアルファ値も調整したい場合はここで設定
			config.color = effect.config.color;
		 } else {
			// 非表示: 色を透明にする（SetVisible(false)だと再アクティブ化の管理が複雑になるため透明推奨）
			config.color = effect.config.hiddenColor;
		 }

		 lightning->ApplyConfigChanges();
	  }
   } else {
	  // 単一の雷の場合、Intensityをアルファ値や太さ(VoxelScale)に反映させると良いでしょう
	  if (!effect.lightnings.empty() && effect.lightnings[0].lightning) {
		 auto& lightning = effect.lightnings[0].lightning;
		 auto& config = lightning->GetConfig();

		 // 強度に応じてアルファ値を変更
		 Vector4 color = effect.config.color;
		 color.w *= intensity;
		 config.color = intensity > 0.01f ? color : effect.config.hiddenColor;

		 lightning->ApplyConfigChanges();
	  }
   }
}

void LightningEffectManager::SetEffectOffsets(int effectId, const Vector3& startOffset, const Vector3& endOffset)
{
   if (effectId < 0 || effectId >= static_cast<int>(effects_.size())) {
      return;
   }

   auto& effect = effects_[effectId];
   
   // 全てのライトニングのオリジナル座標を更新
   for (auto& lightningData : effect.lightnings) {
      lightningData.originalStartPoint = effect.position + startOffset;
      lightningData.originalEndPoint = effect.position + endOffset;
   }
   
   // コンフィグも更新
   effect.config.startOffset = startOffset;
   effect.config.endOffset = endOffset;
}
